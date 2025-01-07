#pragma once

inline constexpr const char* HELP_MESSAGE{"tre Font Compiler (trefc) by TRDario.\n"
										  "Usage: trefc [input file] [image file (BMP, PNG, JPEG)] [output file]\n"};

inline constexpr const char* INVALID_ARGUMENT_COUNT_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" expected 3 arguments, recieved {}\n"};

inline constexpr const char* UNHANDLED_EXCEPTION_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"unhandled exception:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" {}\n"};

inline constexpr const char* FILE_NOT_FOUND_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" file '{}' not found\n"};

inline constexpr const char* FILE_OPENING_FAILURE_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" failed to open file '{}'\n"};

constexpr auto IMAGE_LOADING_FAILURE_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" failed to load image from '{}' ({})\n"};

constexpr auto UNTERMINATED_COMMENT_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" while parsing '{}':\n"
	"unterminated block ('/*') comment (at line {})\n"};

constexpr auto INVALID_VALUE_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" while parsing '{}':\n"
	"line {}: '{}' is not a permitted integer value\n"};

constexpr auto INTEGER_OUT_OF_RANGE_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" while parsing '{}':\n"
	"line {}: '{}' is not within the supported range of values for '{}'\n"};

constexpr auto EXPECTED_SYMBOL_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" while parsing '{}':\n"
	"line {}: expected '{}'\n"};

constexpr auto INVALID_CODEPOINT_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" while parsing '{}':\n"
	"line {}: invalid codepoint '{}'\n"};

constexpr auto DUPLICATE_CODEPOINT_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" while parsing '{}':\n"
	"line {}: duplicate codepoint '{:#06x}'\n"};

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