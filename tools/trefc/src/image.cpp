#include "../include/message.hpp"
#include "../include/trefc.hpp"
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#include "../include/stb_image.h"

Bitmap::Bitmap(const void* data, unsigned int width, unsigned int height) noexcept
	: BitmapRef{data, width, height}
{
}

Bitmap::~Bitmap() noexcept
{
	stbi_image_free((void*)(data));
}

Expected<Bitmap, ErrorCode> loadBitmap(std::string_view path)
{
	if (!std::filesystem::exists(path)) {
		print(std::cerr, FILE_NOT_FOUND_MESSAGE, path);
		return FILE_NOT_FOUND;
	}

	int         width, height, channels;
	const void* bitmapData{stbi_load(path.data(), &width, &height, &channels, 4)};
	if (bitmapData == nullptr) {
		print(std::cerr, IMAGE_LOADING_FAILURE_MESSAGE, path, stbi_failure_reason());
		return IMAGE_FAILURE;
	}
	else {
		return Expected<Bitmap, ErrorCode>{std::in_place_type<Bitmap>, bitmapData, width, height};
	}
}