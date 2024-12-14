#include "../include/trefc.hpp"
#include <fstream>

constexpr auto IMAGE_ENCODING_FAILURE_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" failed to encode image\n"};

constexpr auto WRITING_FAILURE_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" a writing operation failed on '{}'\n"};

ErrorCode writeToOutput(std::string_view path, const FontInfo& fontInfo, const Bitmap& bitmap)
{
	std::ofstream file{path.data(), std::ios::binary};
	if (!file.is_open()) {
		print(std::cerr, FILE_OPENING_FAILURE_MESSAGE, path);
		return FILE_OPENING_FAILURE;
	}

	try {
		tref::encode(file, fontInfo.lineSkip, fontInfo.glyphs, bitmap);
	}
	catch (tref::EncodingError& err) {
		print(std::cerr, IMAGE_ENCODING_FAILURE_MESSAGE);
		return IMAGE_FAILURE;
	}
	if (!file) {
		print(std::cerr, WRITING_FAILURE_MESSAGE, path);
		return WRITING_FAILURE;
	}
	return SUCCESS;
}