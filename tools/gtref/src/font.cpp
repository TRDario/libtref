#include "../include/font.hpp"
#include <GL/glew.h>
#include <tinyfiledialogs.h>
#include <tr/imgui.hpp>

std::optional<LoadResult> loadImage(const std::filesystem::path& path) noexcept
{
	try {
		return {{.font = {.lineSkip = 0, .glyphs = {}}, .image = tr::loadBitmapFile(path)}};
	}
	catch (std::exception& err) {
		tinyfd_messageBox("Error - gtref", err.what(), "ok", "error", 0);
		return std::nullopt;
	}
}

std::optional<LoadResult> loadFont(const std::filesystem::path& path) noexcept
{
	try {
		std::ifstream file{tr::openFileR(path, std::ios::binary)};
		const auto [lineSkip, glyphs, bitmap]{tref::decode(file)};
		const tr::BitmapView image{bitmap.data(), {bitmap.width(), bitmap.height()}, tr::BitmapFormat::RGBA_8888};
		return {{.font  = {.lineSkip = lineSkip, .glyphs = std::move(glyphs)},
				 .image = tr::Bitmap{image, tr::BitmapFormat::RGBA_8888}}};
	}
	catch (std::exception& err) {
		tinyfd_messageBox("Error - gtref", err.what(), "ok", "error", 0);
		return std::nullopt;
	}
}

void saveFont(const std::filesystem::path& path, const Font& font, const tr::Bitmap& bitmap) noexcept
{
	try {
		std::ofstream file{tr::openFileW(path, std::ios::binary)};
		glm::uvec2    size{bitmap.size()};
		tref::encode(file, font.lineSkip, font.glyphs, tref::InputBitmap{bitmap.data(), size.x, size.y});
	}
	catch (std::exception& err) {
		tinyfd_messageBox("Error - gtref", err.what(), "ok", "error", 0);
	}
}