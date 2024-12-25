#include "../include/tref/tref.hpp"
#include <array>
#include <cstring>
#include <lz4.h>
#include <sstream>
#include <vector>

#define QOI_IMPLEMENTATION
#include "../include/tref/qoi.h"

tref::OutputBitmap::OutputBitmap(std::byte* data, unsigned int width, unsigned int height) noexcept
	: _data{data}, _width{width}, _height{height}
{
}

tref::OutputBitmap::~OutputBitmap() noexcept
{
	std::free(_data);
}

std::span<const std::byte> tref::OutputBitmap::data() const noexcept
{
	return {_data, _width * _height * 4};
}

unsigned int tref::OutputBitmap::width() const noexcept
{
	return _width;
}

unsigned int tref::OutputBitmap::height() const noexcept
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

tref::DecodingResult tref::decode(std::istream& is)
{
	std::array<char, 4> magic;
	is.read(magic.data(), 4);
	if (std::string_view{magic.data(), 4} != "TREF") {
		throw DecodingError{"Invalid .tref file header."};
	}

	std::uint32_t rawSize;
	is.read((char*)(&rawSize), sizeof(rawSize));

	std::vector<char> lz4Data;
	std::vector<char> rawData(rawSize);

	std::copy(std::istreambuf_iterator<char>{is}, std::istreambuf_iterator<char>{}, std::back_inserter(lz4Data));
	const auto reportedSize{LZ4_decompress_safe(lz4Data.data(), rawData.data(), lz4Data.size(), rawData.size())};
	if (std::uint32_t(reportedSize) != rawSize) {
		throw DecodingError{"LZ4 decompression of tref file failed."};
	}

	char*    ptr{rawData.data()};
	auto     lineSkip{readBinary<std::int32_t>(ptr)};
	auto     nglyphs{readBinary<std::uint32_t>(ptr)};
	GlyphMap glyphs;
	for (std::uint32_t i = 0; i < nglyphs; ++i) {
		glyphs.emplace(readBinary<Codepoint>(ptr), readBinary<Glyph>(ptr));
	}

	qoi_desc desc;
	auto     decodedImage{qoi_decode(ptr, rawData.size() - (ptr - rawData.data()), &desc, 4)};
	if (decodedImage == nullptr) {
		throw DecodingError{"Failed to decode QOI data."};
	}
	return {lineSkip, std::move(glyphs), OutputBitmap{(std::byte*)(decodedImage), desc.width, desc.height}};
}

void tref::encode(std::ostream& os, std::int32_t lineSkip, const GlyphMap& glyphs, const InputBitmap& bitmap)
{
	qoi_desc   desc{bitmap.width, bitmap.height, 4, QOI_SRGB};
	int        qoiImageSize;
	const auto qoiImage{qoi_encode(bitmap.data, &desc, &qoiImageSize)};
	if (qoiImage == nullptr) {
		throw EncodingError{"Failed to encode QOI data."};
	}

	std::stringstream bufferStream;
	bufferStream.write((const char*)(&lineSkip), sizeof(lineSkip));
	auto nglyphs{std::uint32_t(glyphs.size())};
	bufferStream.write((const char*)(&nglyphs), sizeof(nglyphs));
	for (auto& [codepoint, glyph] : glyphs) {
		bufferStream.write((const char*)(&codepoint), sizeof(codepoint));
		bufferStream.write((const char*)(&glyph), sizeof(glyph));
	}
	bufferStream.write((const char*)(qoiImage), qoiImageSize);
	auto data{std::move(bufferStream).str()};
	auto uncompressedSize{std::uint32_t(data.size())};

	std::vector<char> buffer(LZ4_compressBound(data.size()));
	buffer.resize(LZ4_compress_default(data.c_str(), buffer.data(), data.size(), buffer.size()));
	os.write("TREF", 4);
	os.write((const char*)(&uncompressedSize), sizeof(uncompressedSize));
	os.write(buffer.data(), buffer.size());
}