#include "../include/trefc.hpp"

constexpr auto TREFC_HELP_MESSAGE{"tre Font Compiler (trefc) by TRDario.\n"
								  "Usage: trefc [input file] [image file (BMP, PNG, JPEG)] [output file]\n"};

constexpr auto INVALID_ARGUMENT_COUNT_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" expected 3 arguments, recieved {}\n"};

constexpr auto UNHANDLED_EXCEPTION_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"unhandled exception:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" {}\n"};

int main(int argc, char* argv[])
{
	try {
		switch (argc) {
		case 1:
			print(std::cout, TREFC_HELP_MESSAGE);
			return PRINTED_HELP;
		case 4:
			break;
		default:
			print(std::cerr, INVALID_ARGUMENT_COUNT_MESSAGE, argc - 1);
			return INVALID_ARGUMENT_COUNT;
		}

		const auto fontInfo{loadFontInfo(argv[1])};
		if (holds_alternative<ErrorCode>(fontInfo)) {
			return get<ErrorCode>(fontInfo);
		}
		const auto inputImage{loadBitmap(argv[2])};
		if (holds_alternative<ErrorCode>(inputImage)) {
			return get<ErrorCode>(inputImage);
		}
		return writeToOutput(argv[3], get<FontInfo>(fontInfo), get<Bitmap>(inputImage));
	}
	catch (std::exception& err) {
		print(std::cerr, UNHANDLED_EXCEPTION_MESSAGE, err.what());
		return UNHANDLED_EXCEPTION;
	}
}