#include "../include/message.hpp"
#include "../include/trefc.hpp"
#include <filesystem>
#include <fstream>

namespace rs = std::ranges;

// Gets the line number for the iterator for error printing purposes.
inline std::size_t errorLine(const std::string& ctx, std::string::const_iterator it) noexcept
{
	return std::count(ctx.begin(), it, '\n') + 1;
}

// Strips the buffer of multiline comments.
std::optional<int> stripMultilineComments(std::string& buffer) noexcept
{
	constexpr std::string_view COMMENT_BEGIN{"/*"};
	constexpr std::string_view COMMENT_END{"*/"};

	for (std::string::iterator it = rs::search(buffer, COMMENT_BEGIN).begin(); it != buffer.end();
		 it                       = rs::search(buffer, COMMENT_BEGIN).begin()) {
		std::string::iterator end{rs::search(buffer, COMMENT_END).end()};
		if (end == buffer.end()) {
			return errorLine(buffer, it);
		}
		std::ptrdiff_t linesRemoved{std::count(it, end, '\n')};
		it = buffer.erase(it, end);
		// To maintain line numbers.
		buffer.insert(it, linesRemoved, '\n');
	}
	return std::nullopt;
}

// Strips the buffer of line comments.
void stripLineComments(std::string& buffer) noexcept
{
	constexpr std::string_view COMMENT_START{"//"};

	for (std::string::iterator it = rs::search(buffer, COMMENT_START).begin(); it != buffer.end();
		 it                       = rs::search(buffer, COMMENT_START).begin()) {
		buffer.erase(it, rs::find(buffer, '\n'));
	};
}

bool isWhitespace(char chr) noexcept
{
	return chr == ' ' || chr == '\t';
}

bool isQuoted(std::string::const_iterator it, const std::string& ctx) noexcept
{
	return it != ctx.begin() && it != ctx.end() - 1 && it[-1] == '\'' && it[1] == '\'';
}

// Strips the buffer of whitespace
void stripWhitespace(std::string& buffer) noexcept
{
	for (std::string::iterator it = buffer.begin(); it != buffer.end();) {
		if (isWhitespace(*it) && !isQuoted(it, buffer)) {
			it = buffer.erase(it);
		}
		else {
			++it;
		}
	}
}

// Strips the buffer of comments and whitespace.
std::optional<int> stripCommentsWhitespace(std::string& buffer) noexcept
{
	buffer.push_back('\n');
	auto result{stripMultilineComments(buffer)};
	if (result.has_value()) {
		return *result;
	}
	stripLineComments(buffer);
	stripWhitespace(buffer);

	return std::nullopt;
}

// Parses a named integer (eg: 'x: 5').
template <std::integral T>
std::optional<std::string::const_iterator> parseNamedInt(T& out, const std::string& ctx, std::string_view name,
														 std::string_view file, std::string::const_iterator start,
														 std::string::const_iterator end)
{
	const std::string_view span{start, end};
	if (!span.starts_with(name)) {
		print(std::cerr, EXPECTED_SYMBOL_MESSAGE, file, errorLine(ctx, start), name);
		return std::nullopt;
	}
	else if (span.size() == name.size() || span[name.size()] != ':') {
		print(std::cerr, EXPECTED_SYMBOL_MESSAGE, file, errorLine(ctx, start), ':');
		return std::nullopt;
	}
	else {
		start += name.size() + 1;
		const std::string_view integer{start, end};

		const std::from_chars_result result{std::from_chars(std::to_address(start), std::to_address(end), out)};
		if (result.ec == std::errc::invalid_argument) {
			print(std::cerr, INVALID_VALUE_MESSAGE, file, errorLine(ctx, start), integer);
			return std::nullopt;
		}
		else if (result.ec == std::errc::result_out_of_range) {
			print(std::cerr, INTEGER_OUT_OF_RANGE_MESSAGE, file, errorLine(ctx, start), integer, name);
			return std::nullopt;
		}
		else {
			return std::next(end);
		}
	}
}

// Converts a UTF8 sequence to a Unicode codepoint.
std::optional<tref::Codepoint> utf8ToCodepoint(std::string_view span) noexcept
{
	std::span<const std::uint8_t> c{reinterpret_cast<const std::uint8_t*>(span.data()), span.size()};

	switch (span.size()) {
	case 1:
		if (c[0] >= 0x80) {
			return std::nullopt;
		}
		return span[0];
	case 2:
		if ((c[0] & 0xE0) != 0xC0 || (c[1] & 0xC0) != 0x80) {
			return std::nullopt;
		}
		return ((c[0] & 0x1F) << 6) + (c[1] & 0x3F);
	case 3:
		if ((c[0] & 0xF0) != 0xE0 || (c[1] & 0xC0) != 0x80 || (c[2] & 0xC0) != 0x80) {
			return std::nullopt;
		}
		return ((c[0] & 0xF) << 12) + ((c[1] & 0x3F) << 6) + (c[2] & 0x3F);
	case 4:
		if ((c[0] & 0xF8) != 0xF0 || (c[1] & 0xC0) != 0x80 || (c[2] & 0xC0) != 0x80 || (c[3] & 0xC0) != 0x80) {
			return std::nullopt;
		}
		return ((c[0] & 0x7) << 18) + ((c[1] & 0x3F) << 12) + ((c[2] & 0x3F) << 6) + (c[3] & 0x3F);
	default:
		return std::nullopt;
	}
}

// Parses a unicode character glyph codepoint.
std::optional<std::string::const_iterator> parseUnicode(tref::Codepoint& out, const std::string& ctx,
														std::string_view file, std::string::const_iterator start,
														std::string::const_iterator end)
{
	++start;
	--end;

	if (*end != '\'') {
		print(std::cerr, INVALID_CODEPOINT_MESSAGE, file, errorLine(ctx, start),
			  std::string_view{std::prev(start), std::next(end)});
		return std::nullopt;
	}

	const std::optional<tref::Codepoint> cp{utf8ToCodepoint({start, end})};
	if (!cp.has_value()) {
		print(std::cerr, INVALID_CODEPOINT_MESSAGE, file, errorLine(ctx, start), std::string_view{start, end});
		return std::nullopt;
	}
	out = *cp;
	return end + 2;
}

std::optional<std::string::const_iterator> parseHex(tref::Codepoint& out, const std::string& ctx, std::string_view file,
													std::string::const_iterator start, std::string::const_iterator end)
{
	if (std::from_chars(std::to_address(start + 2), std::to_address(end), out, 16).ec != std::errc{}) {
		print(std::cerr, INVALID_CODEPOINT_MESSAGE, file, errorLine(ctx, start), std::string_view{start, end});
		return std::nullopt;
	}
	return std::next(end);
}

// Parses a glyph codepoint (Formats: 'a', 0x20, NUL).
std::optional<std::string::const_iterator> parseGlyphCodepoint(tref::Codepoint& out, const std::string& ctx,
															   std::string_view file, std::string::const_iterator start)
{
	constexpr std::string_view DELIMITERS{":\n"};

	if (std::string_view{start, ctx.end()}.starts_with("''':")) {
		out = '\'';
		return start + 4;
	}

	std::string::const_iterator end{std::find_first_of(start, ctx.end(), DELIMITERS.begin(), DELIMITERS.end())};
	if (*end != ':') {
		print(std::cerr, EXPECTED_SYMBOL_MESSAGE, file, errorLine(ctx, start), ':');
		return std::nullopt;
	}

	if (*start == '\'') {
		return parseUnicode(out, ctx, file, start, end);
	}
	else if (std::string_view{start, end}.starts_with("0x")) {
		return parseHex(out, ctx, file, start, end);
	}
	else if (std::string_view{start, end} == "NUL") {
		out = '\0';
		return std::next(end);
	}
	else {
		print(std::cerr, INVALID_CODEPOINT_MESSAGE, file, errorLine(ctx, start), std::string_view{start, end});
		return std::nullopt;
	}
}

// Parses a named glyph attribute (x, y, etc...).
std::optional<std::string::const_iterator> parseGlyphAttribute(auto& out, std::string_view name, char delim,
															   const std::string& ctx, std::string_view file,
															   std::string::const_iterator start)
{
	constexpr std::string_view DELIMITERS{",\n"};

	const std::string::const_iterator lineEnd{std::find(start, ctx.end(), '\n')};
	const std::string::const_iterator end{std::find_first_of(start, lineEnd, DELIMITERS.begin(), DELIMITERS.end())};
	if ((delim == '\n') ^ (end == lineEnd)) {
		print(std::cerr, EXPECTED_SYMBOL_MESSAGE, file, errorLine(ctx, start),
			  delim == '\n' ? "\\n" : std::string_view{&delim, 1});
		return std::nullopt;
	}
	if (!(parseNamedInt(out, ctx, name, file, start, end))) {
		return std::nullopt;
	}
	return std::next(end);
}

// Parses the rest of the glyph after the codepoint.
std::optional<std::string::const_iterator> parseGlyphAttributes(tref::Glyph& out, const std::string& ctx,
																std::string_view            file,
																std::string::const_iterator start)
{
	std::optional<std::string::const_iterator> it = start;
	if (!(it = parseGlyphAttribute(out.x, "x", ',', ctx, file, *it)) ||
		!(it = parseGlyphAttribute(out.y, "y", ',', ctx, file, *it)) ||
		!(it = parseGlyphAttribute(out.width, "width", ',', ctx, file, *it)) ||
		!(it = parseGlyphAttribute(out.height, "height", ',', ctx, file, *it)) ||
		!(it = parseGlyphAttribute(out.xOffset, "xoffset", ',', ctx, file, *it)) ||
		!(it = parseGlyphAttribute(out.yOffset, "yoffset", ',', ctx, file, *it)) ||
		!(it = parseGlyphAttribute(out.advance, "advance", '\n', ctx, file, *it))) {
		return std::nullopt;
	}
	return it;
}

Expected<FontInfo, ErrorCode> loadFontInfo(std::string_view path)
{
	if (!std::filesystem::exists(path)) {
		print(std::cerr, FILE_NOT_FOUND_MESSAGE, path);
		return FILE_NOT_FOUND;
	}
	std::ifstream file{path.data(), std::ios::binary};
	if (!file.is_open()) {
		print(std::cerr, FILE_OPENING_FAILURE_MESSAGE, path);
		return FILE_OPENING_FAILURE;
	}

	std::string              buffer{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
	const std::optional<int> stripResult{stripCommentsWhitespace(buffer)};
	if (stripResult.has_value()) {
		print(std::cerr, UNTERMINATED_COMMENT_MESSAGE, path, *stripResult);
		return PARSING_FAILURE;
	}

	FontInfo                                   font;
	std::optional<std::string::const_iterator> it;
	if (!(it = parseNamedInt(font.lineSkip, buffer, "line_skip", path, buffer.begin(), rs::find(buffer, '\n')))) {
		return PARSING_FAILURE;
	}

	while (*it < buffer.end() - 1) {
		if (**it == '\n') {
			++*it;
			continue;
		}

		tref::Codepoint codepoint;
		if (!(it = parseGlyphCodepoint(codepoint, buffer, path, *it))) {
			return PARSING_FAILURE;
		}
		else if (font.glyphs.contains(codepoint)) {
			print(std::cerr, DUPLICATE_CODEPOINT_MESSAGE, path, errorLine(buffer, *it), codepoint);
			return PARSING_FAILURE;
		}

		tref::Glyph glyph;
		if (!(it = parseGlyphAttributes(glyph, buffer, path, *it))) {
			return PARSING_FAILURE;
		}
		font.glyphs.emplace(codepoint, glyph);
	}
	if (!font.glyphs.contains('\0')) {
		font.glyphs.emplace('\0', tref::Glyph{0, 0, 0, 0, 0, 0, 0});
	}
	return std::move(font);
}