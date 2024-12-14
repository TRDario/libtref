#include "../include/trefc.hpp"
#include <filesystem>
#include <fstream>

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

// Gets the line number for the iterator for error printing purposes.
inline std::size_t errorLineNumber(const std::string& context, std::string::const_iterator it) noexcept
{
	return std::count(context.begin(), it, '\n') + 1;
}

// Strips the buffer of comments and whitespace.
std::optional<int> stripCommentsWhitespace(std::string& buffer) noexcept
{
	constexpr std::string_view C_COMMENT_BEGIN{"/*"};
	constexpr std::string_view C_COMMENT_END{"*/"};
	constexpr std::string_view CPP_COMMENT_START{"//"};

	buffer.push_back('\n');
	for (auto it = std::ranges::search(buffer, C_COMMENT_BEGIN).begin(); it != buffer.end();
		 it      = std::ranges::search(buffer, C_COMMENT_BEGIN).begin()) {
		auto end{std::ranges::search(buffer, C_COMMENT_END).end()};
		if (end == buffer.end()) {
			return errorLineNumber(buffer, it);
		}
		auto linesRemoved{std::count(it, end, '\n')};
		it = buffer.erase(it, end);
		// To maintain line numbers.
		buffer.insert(it, linesRemoved, '\n');
	}
	for (auto it = std::ranges::search(buffer, CPP_COMMENT_START).begin(); it != buffer.end();
		 it      = std::ranges::search(buffer, CPP_COMMENT_START).begin()) {
		buffer.erase(it, std::ranges::find(buffer, '\n'));
	}
	for (auto it = buffer.begin(); it != buffer.end();) {
		auto whitespace{[it] { return *it == ' ' || *it == '\t'; }};
		auto quoted{[&] { return it != buffer.begin() && it != buffer.end() - 1 && it[-1] == '\'' && it[1] == '\''; }};

		if (whitespace() && !quoted()) {
			it = buffer.erase(it);
		}
		else {
			++it;
		}
	}
	return std::nullopt;
}

// Parses a named integer (eg: 'x: 5').
template <std::integral T>
std::optional<std::string::const_iterator> parseNamedInteger(T& value, const std::string& context,
															 std::string_view name, std::string_view filename,
															 std::string::const_iterator start,
															 std::string::const_iterator end)
{
	const std::string_view span{start, end};
	if (!span.starts_with(name)) {
		print(std::cerr, EXPECTED_SYMBOL_MESSAGE, filename, errorLineNumber(context, start), name);
		return std::nullopt;
	}
	else if (span.size() == name.size() || span[name.size()] != ':') {
		print(std::cerr, EXPECTED_SYMBOL_MESSAGE, filename, errorLineNumber(context, start), ':');
		return std::nullopt;
	}
	else {
		start += name.size() + 1;
		const std::string_view integer{start, end};

		const auto parsingResult{std::from_chars(std::to_address(start), std::to_address(end), value)};
		if (parsingResult.ec == std::errc::invalid_argument) {
			print(std::cerr, INVALID_VALUE_MESSAGE, filename, errorLineNumber(context, start), integer);
			return std::nullopt;
		}
		else if (parsingResult.ec == std::errc::result_out_of_range) {
			print(std::cerr, INTEGER_OUT_OF_RANGE_MESSAGE, filename, errorLineNumber(context, start), integer, name);
			return std::nullopt;
		}
		else {
			return std::next(end);
		}
	}
}

// Converts a UTF8 sequence to a Unicode codepoint.
std::optional<std::uint32_t> utf8ToCodepoint(std::string_view span) noexcept
{
	switch (span.size()) {
	case 1:
		if (std::uint8_t(span[0]) >= 0x80) {
			return std::nullopt;
		}
		return span[0];
	case 2:
		if ((std::uint8_t(span[0]) & 0xE0) != 0xC0 || (std::uint8_t(span[1]) & 0xC0) != 0x80) {
			return std::nullopt;
		}
		return ((std::uint8_t(span[0]) & 0x1F) << 6) + (std::uint8_t(span[1]) & 0x3F);
	case 3:
		if ((std::uint8_t(span[0]) & 0xF0) != 0xE0 || (std::uint8_t(span[1]) & 0xC0) != 0x80 ||
			(std::uint8_t(span[2]) & 0xC0) != 0x80) {
			return std::nullopt;
		}
		return ((std::uint8_t(span[0]) & 0xF) << 10) + ((std::uint8_t(span[1]) & 0x3F) << 6) +
			   (std::uint8_t(span[2]) & 0x3F);
	case 4:
		if ((std::uint8_t(span[0]) & 0xF8) != 0xF0 || (std::uint8_t(span[1]) & 0xC0) != 0x80 ||
			(std::uint8_t(span[2]) & 0xC0) != 0x80 || (std::uint8_t(span[3]) & 0xC0) != 0x80) {
			return std::nullopt;
		}
		return ((std::uint8_t(span[0]) & 0x7) << 18) + ((std::uint8_t(span[1]) & 0x3F) << 12) +
			   ((std::uint8_t(span[2]) & 0x3F) << 6) + (std::uint8_t(span[3]) & 0x3F);
	default:
		return std::nullopt;
	}
}

// Parses a glyph codepoint (Formats: 'a', 0x20, NUL).
std::optional<std::string::const_iterator> parseGlyphCodepoint(std::uint32_t& codepoint, const std::string& context,
															   std::string_view            filename,
															   std::string::const_iterator start)
{
	if (std::string_view{start, context.end()}.starts_with("''':")) {
		codepoint = '\'';
		return start + 4;
	}

	constexpr std::string_view DELIMITERS{":\n"};
	auto                       end{std::find_first_of(start, context.end(), DELIMITERS.begin(), DELIMITERS.end())};
	if (*end != ':') {
		print(std::cerr, EXPECTED_SYMBOL_MESSAGE, filename, errorLineNumber(context, start), ':');
		return std::nullopt;
	}

	if (*start == '\'') {
		++start;
		--end;
		if (*end != '\'') {
			print(std::cerr, INVALID_CODEPOINT_MESSAGE, filename, errorLineNumber(context, start),
				  std::string_view{std::prev(start), std::next(end)});
			return std::nullopt;
		}

		auto conversionResult{utf8ToCodepoint({start, end})};
		if (!conversionResult.has_value()) {
			print(std::cerr, INVALID_CODEPOINT_MESSAGE, filename, errorLineNumber(context, start),
				  std::string_view{start, end});
			return std::nullopt;
		}
		codepoint = *conversionResult;
		return end + 2;
	}
	else if (std::string_view{start, end}.starts_with("0x")) {
		const auto parsingResult{std::from_chars(std::to_address(start + 2), std::to_address(end), codepoint, 16)};
		if (parsingResult.ec != std::errc()) {
			print(std::cerr, INVALID_CODEPOINT_MESSAGE, filename, errorLineNumber(context, start),
				  std::string_view{start, end});
			return std::nullopt;
		}
	}
	else if (std::string_view{start, end} == "NUL") {
		codepoint = '\0';
	}
	else {
		print(std::cerr, INVALID_CODEPOINT_MESSAGE, filename, errorLineNumber(context, start),
			  std::string_view{start, end});
		return std::nullopt;
	}
	return std::next(end);
}

// Parses a named glyph attribute (x, y, etc...).
std::optional<std::string::const_iterator> parseGlyphAttribute(auto& value, std::string_view name, char delim,
															   const std::string& context, std::string_view filename,
															   std::string::const_iterator start)
{
	constexpr std::string_view DELIMITERS{",\n"};

	const auto lineEnd{std::find(start, context.end(), '\n')};
	const auto end{std::find_first_of(start, lineEnd, DELIMITERS.begin(), DELIMITERS.end())};
	if ((delim == '\n') ^ (end == lineEnd)) {
		print(std::cerr, EXPECTED_SYMBOL_MESSAGE, filename, errorLineNumber(context, start),
			  delim == '\n' ? "\\n" : std::string_view{&delim, 1});
		return std::nullopt;
	}
	const auto parseResult{parseNamedInteger(value, context, name, filename, start, end)};
	if (!parseResult.has_value()) {
		return std::nullopt;
	}
	return std::next(end);
}

// Parses the rest of the glyph after the codepoint.
std::optional<std::string::const_iterator> parseGlyphAttributes(tref::Glyph& glyph, const std::string& context,
																std::string_view            filename,
																std::string::const_iterator start)
{
	std::optional<std::string::const_iterator> it = start;
	it                                            = parseGlyphAttribute(glyph.x, "x", ',', context, filename, *it);
	if (!it.has_value()) {
		return std::nullopt;
	}
	it = parseGlyphAttribute(glyph.y, "y", ',', context, filename, *it);
	if (!it.has_value()) {
		return std::nullopt;
	}
	it = parseGlyphAttribute(glyph.width, "width", ',', context, filename, *it);
	if (!it.has_value()) {
		return std::nullopt;
	}
	it = parseGlyphAttribute(glyph.height, "height", ',', context, filename, *it);
	if (!it.has_value()) {
		return std::nullopt;
	}
	it = parseGlyphAttribute(glyph.xOffset, "xoffset", ',', context, filename, *it);
	if (!it.has_value()) {
		return std::nullopt;
	}
	it = parseGlyphAttribute(glyph.yOffset, "yoffset", ',', context, filename, *it);
	if (!it.has_value()) {
		return std::nullopt;
	}
	return parseGlyphAttribute(glyph.advance, "advance", '\n', context, filename, *it);
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

	std::string buffer{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
	const auto  stripResult{stripCommentsWhitespace(buffer)};
	if (stripResult.has_value()) {
		print(std::cerr, UNTERMINATED_COMMENT_MESSAGE, path, *stripResult);
		return PARSING_FAILURE;
	}

	FontInfo                                   fontInfo;
	std::optional<std::string::const_iterator> it;
	it = parseNamedInteger(fontInfo.lineSkip, buffer, "line_skip", path, buffer.begin(),
						   std::ranges::find(buffer, '\n'));
	if (!it.has_value()) {
		return PARSING_FAILURE;
	}

	while (*it < buffer.end() - 1) {
		if (**it == '\n') {
			++*it;
			continue;
		}

		std::uint32_t codepoint;
		it = parseGlyphCodepoint(codepoint, buffer, path, *it);
		if (!it.has_value()) {
			return PARSING_FAILURE;
		}
		else if (fontInfo.glyphs.contains(codepoint)) {
			print(std::cerr, DUPLICATE_CODEPOINT_MESSAGE, path, errorLineNumber(buffer, *it), codepoint);
			return PARSING_FAILURE;
		}

		tref::Glyph glyph;
		it = parseGlyphAttributes(glyph, buffer, path, *it);
		if (!it.has_value()) {
			return PARSING_FAILURE;
		}
		fontInfo.glyphs.emplace(codepoint, glyph);
	}
	if (fontInfo.glyphs.empty()) {
		fontInfo.glyphs.emplace('\0', tref::Glyph{0, 0, 0, 0, 0, 0, 0});
	}
	return std::move(fontInfo);
}