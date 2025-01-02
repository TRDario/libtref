#pragma once
#include <tr/tr.hpp>
#include <tref/tref.hpp>

// Font data.
struct Font {
	std::int32_t   lineSkip;
	tref::GlyphMap glyphs;
};

// Data loaded from file.
struct LoadResult {
	Font       font;
	tr::Bitmap image;
};

// Loads an image from file.
std::optional<LoadResult> loadImage(const std::filesystem::path& path) noexcept;

// Loads a font from file.
std::optional<LoadResult> loadFont(const std::filesystem::path& path) noexcept;

// Saves a font to file.
void saveFont(const std::filesystem::path& path, const Font& font, const tr::Bitmap& bitmap) noexcept;