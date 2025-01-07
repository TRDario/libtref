#include "../include/message.hpp"
#include "../include/trefc.hpp"
#include <fstream>

ErrorCode writeToOutput(std::string_view path, const FontInfo& fontInfo, const Bitmap& bitmap)
{
	std::ofstream file{path.data(), std::ios::binary};
	if (!file.is_open()) {
		print(std::cerr, FILE_OPENING_FAILURE_MESSAGE, path);
		return FILE_OPENING_FAILURE;
	}

	try {
		tref::encode(file, fontInfo.lineSkip, fontInfo.glyphs, bitmap);
		if (!file) {
			print(std::cerr, WRITING_FAILURE_MESSAGE, path);
			return WRITING_FAILURE;
		}
	}
	catch (tref::EncodingError& err) {
		print(std::cerr, IMAGE_ENCODING_FAILURE_MESSAGE);
		return IMAGE_FAILURE;
	}

	return SUCCESS;
}