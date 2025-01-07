#include "../include/message.hpp"
#include "../include/trefc.hpp"

int main(int argc, char* argv[])
{
	try {
		switch (argc) {
		case 1:
			print(std::cout, HELP_MESSAGE);
			return PRINTED_HELP;
		case 4:
			break;
		default:
			print(std::cerr, INVALID_ARGUMENT_COUNT_MESSAGE, argc - 1);
			return INVALID_ARGUMENT_COUNT;
		}

		const Expected<FontInfo, ErrorCode> fontInfo{loadFontInfo(argv[1])};
		if (holds_alternative<ErrorCode>(fontInfo)) {
			return get<ErrorCode>(fontInfo);
		}
		const Expected<Bitmap, ErrorCode> inputImage{loadBitmap(argv[2])};
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