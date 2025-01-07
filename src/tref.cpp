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

template <class T> T readBinary(char*& ptr) noexcept
{
	T value;
	std::memcpy(&value, ptr, sizeof(T));
	ptr += sizeof(T);
	return value;
}

template <class T> void writeBinary(std::ostream& os, const T& value) noexcept
{
	os.write((const char*)(&value), sizeof(value));
}

tref::DecodingResult tref::decode(std::istream& is)
{
	char magic[4];
	is.read(magic, sizeof(magic));
	if (std::string_view{magic, sizeof(magic)} != "TREF") {
		throw DecodingError{"Invalid .tref file header."};
	}

	std::uint32_t rawSize;
	is.read((char*)(&rawSize), sizeof(rawSize));
	std::vector<char> raw(rawSize);
	std::vector<char> lz4{std::istreambuf_iterator<char>{is}, std::istreambuf_iterator<char>{}};
	const int         reportedSize{LZ4_decompress_safe(lz4.data(), raw.data(), lz4.size(), raw.size())};
	if (std::uint32_t(reportedSize) != rawSize) {
		throw DecodingError{"LZ4 decompression of tref file failed."};
	}
	char* it{raw.data()};

	const std::int32_t  lineSkip{readBinary<std::int32_t>(it)};
	const std::uint32_t count{readBinary<std::uint32_t>(it)};
	GlyphMap            glyphs;
	for (std::uint32_t i = 0; i < count; ++i) {
		glyphs.emplace(readBinary<Codepoint>(it), readBinary<Glyph>(it));
	}

	qoi_desc   desc;
	std::byte* bitmap{(std::byte*)(qoi_decode(it, std::distance(it, raw.data() + raw.size()), &desc, 4))};
	if (bitmap == nullptr) {
		throw DecodingError{"Failed to decode QOI data."};
	}

	return DecodingResult{lineSkip, std::move(glyphs), DecodedBitmap{bitmap, desc.width, desc.height}};
}

void tref::encode(std::ostream& os, std::int32_t lineSkip, const GlyphMap& glyphs, const BitmapRef& bitmap)
{
	const qoi_desc desc{bitmap.width, bitmap.height, 4, QOI_SRGB};
	int            bitmapSize;
	const char*    qoi{(const char*)(qoi_encode(bitmap.data, &desc, &bitmapSize))};
	if (qoi == nullptr) {
		throw EncodingError{"Failed to encode QOI data."};
	}

	std::stringstream buffer{std::ios::binary};
	writeBinary(buffer, lineSkip);
	writeBinary(buffer, std::uint32_t(glyphs.size()));
	for (auto& [cp, glyph] : glyphs) {
		writeBinary(buffer, cp);
		writeBinary(buffer, glyph);
	}
	buffer.write(qoi, bitmapSize);

	const std::string raw{std::move(buffer).str()};
	if (raw.size() > LZ4_MAX_INPUT_SIZE) {
		throw EncodingError{"File is too large to encode."};
	}
	std::vector<char> lz4(LZ4_compressBound(raw.size()));
	lz4.resize(LZ4_compress_default(raw.c_str(), lz4.data(), raw.size(), lz4.size()));

	os.write("TREF", 4);
	writeBinary(os, std::uint32_t(raw.size()));
	os.write(lz4.data(), lz4.size());
}