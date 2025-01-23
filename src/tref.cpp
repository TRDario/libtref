#include "../include/tref/tref.hpp"
#include <cstring>
#include <lz4.h>
#include <sstream>
#include <vector>

#define QOI_IMPLEMENTATION
#include "../include/tref/qoi.h"

tref::DecodedBitmap::DecodedBitmap(std::byte* data, unsigned int width, unsigned int height) noexcept
	: _data{data}, _width{width}, _height{height}
{
}

tref::DecodedBitmap::~DecodedBitmap() noexcept
{
	std::free(_data);
}

std::span<const std::byte> tref::DecodedBitmap::data() const noexcept
{
	return {_data, _width * _height * 4};
}

unsigned int tref::DecodedBitmap::width() const noexcept
{
	return _width;
}

unsigned int tref::DecodedBitmap::height() const noexcept
{
	return _height;
}

template <class T> T readBinary(const std::byte*& ptr, const std::byte* end)
{
	if ((end - ptr) < static_cast<std::ptrdiff_t>(sizeof(T))) {
		throw tref::DecodingError{"Invalid .tref file."};
	}

	T value;
	std::memcpy(&value, ptr, sizeof(T));
	ptr += sizeof(T);
	return value;
}

template <class T> void writeBinary(std::ostream& os, const T& value) noexcept
{
	os.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

template <class T> void writeBinaryRange(std::ostream& os, const T& range) noexcept
{
	os.write(reinterpret_cast<const char*>(range.data()), range.size());
}

tref::DecodingResult tref::decode(std::span<const std::byte> data)
{
	const std::byte* it{data.data()};
	const std::byte* end{data.data() + data.size()};

	if (std::string_view{readBinary<std::array<char, 4>>(it, end).data(), 4} != "TREF") {
		throw DecodingError{"Invalid .tref file header."};
	}

	std::vector<std::byte> raw(readBinary<std::uint32_t>(it, end));
	std::vector<std::byte> lz4{it, end};
	const int              reportedSize{LZ4_decompress_safe(reinterpret_cast<const char*>(lz4.data()),
															reinterpret_cast<char*>(raw.data()), lz4.size(), raw.size())};
	if (static_cast<std::uint32_t>(reportedSize) != raw.size()) {
		throw DecodingError{"Decompression of .tref file failed."};
	}
	it  = raw.data();
	end = raw.data() + raw.size();

	const std::int32_t  lineSkip{readBinary<std::int32_t>(it, end)};
	const std::uint32_t count{readBinary<std::uint32_t>(it, end)};
	GlyphMap            glyphs;
	for (std::uint32_t i = 0; i < count; ++i) {
		glyphs.emplace(readBinary<Codepoint>(it, end), readBinary<Glyph>(it, end));
	}

	qoi_desc   desc;
	std::byte* bitmap{static_cast<std::byte*>(qoi_decode(it, std::distance(it, end), &desc, 4))};
	if (bitmap == nullptr) {
		throw DecodingError{"Failed to decode .tref file image data."};
	}

	return DecodingResult{lineSkip, std::move(glyphs), DecodedBitmap{bitmap, desc.width, desc.height}};
}

void tref::encode(std::ostream& os, std::int32_t lineSkip, const GlyphMap& glyphs, const BitmapRef& bitmap)
{
	const qoi_desc             desc{bitmap.width, bitmap.height, 4, QOI_SRGB};
	int                        size;
	std::span<const std::byte> qoi{static_cast<const std::byte*>(qoi_encode(bitmap.data, &desc, &size)),
								   static_cast<std::size_t>(size)};
	if (qoi.data() == nullptr) {
		throw EncodingError{"Failed to encode .tref file image data."};
	}

	std::ostringstream buffer{std::ios::binary};
	writeBinary(buffer, lineSkip);
	writeBinary(buffer, static_cast<std::uint32_t>(glyphs.size()));
	for (auto& [cp, glyph] : glyphs) {
		writeBinary(buffer, cp);
		writeBinary(buffer, glyph);
	}
	writeBinaryRange(os, qoi);

	const std::string raw{std::move(buffer).str()};
	if (raw.size() > LZ4_MAX_INPUT_SIZE) {
		throw EncodingError{".tref file is too large to encode."};
	}
	std::vector<std::byte> lz4(LZ4_compressBound(raw.size()));
	lz4.resize(LZ4_compress_default(raw.c_str(), reinterpret_cast<char*>(lz4.data()), raw.size(), lz4.size()));

	os.write("TREF", 4);
	writeBinary(os, static_cast<std::uint32_t>(raw.size()));
	writeBinaryRange(os, lz4);
}