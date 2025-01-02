#include "../include/file_edit.hpp"
#include "../include/preview.hpp"
#include <imgui.h>
#include <tr/imgui.hpp>

/// PUBLIC FUNCTIONS ///

void Preview::addMenu(const FileEdit& fileEdit)
{
	if (ImGui::BeginMenu("Preview")) {
		if (ImGui::MenuItem("Open Preview Window...", "F1", false, !fileEdit.empty())) {
			_previewWindowActive = true;
		}
		ImGui::EndMenu();
	}

	if (!fileEdit.empty() && !ImGui::GetIO().WantCaptureKeyboard && ImGui::IsKeyChordPressed(ImGuiKey_F1)) {
		_previewWindowActive = !_previewWindowActive;
	}
}

void Preview::addPreviewWindow(const FileEdit& fileEdit)
{
	if (fileEdit.empty()) {
		_previewWindowActive = false;
	}
	if (!_previewWindowActive) {
		return;
	}

	const std::string_view text{_previewTextBuffer.data()};
	const TextBounds       bounds{findTextBounds(fileEdit)};
	const float            windowWidth{bounds.max.x - bounds.min.x};

	if (ImGui::Begin("Preview", &_previewWindowActive,
					 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
		const float inputHeight{ImGui::GetTextLineHeightWithSpacing() *
								std::max(2.0f, std::ranges::count(text, '\n') + 1.0f)};
		ImGui::InputTextMultiline("##Preview Text", _previewTextBuffer.data(), 126, {windowWidth, inputHeight});
		ImGui::Separator();

		const float previewHeight{bounds.max.y - bounds.min.y};
		if (ImGui::BeginChild("##Preview View", {windowWidth, previewHeight}, ImGuiChildFlags_Borders,
							  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar)) {
			int line{0};
			int x{0};
			for (tref::Codepoint cp : tr::utf8Range(text)) {
				drawGlyph(fileEdit, cp, bounds, line, x);
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

/// INTERNAL FUNCTIONS ///

Preview::TextBounds Preview::findTextBounds(const FileEdit& fileEdit) const noexcept
{
	const std::string_view text{_previewTextBuffer.data()};
	const int              lineSkip{fileEdit.lineSkip()};

	TextBounds bounds{{0, 0}, {256, fileEdit.lineSkip() * (std::ranges::count(text, '\n') + 1)}};

	int line{0};
	int x{0};
	for (tref::Codepoint cp : tr::utf8Range(text)) {
		if (cp == '\n') {
			++line;
			x = 0;
			continue;
		}
		else if (!fileEdit.glyphs().contains(cp)) {
			cp = '\0';
		}

		const tref::Glyph& glyph{fileEdit.glyphs().at(cp)};

		x += glyph.advance;
		if (glyph.width != 0 && glyph.height != 0) {
			bounds.min.x = std::min(bounds.min.x, float(x + glyph.xOffset));
			bounds.min.y = std::min(bounds.min.y, float(line * lineSkip + glyph.yOffset));
			bounds.max.x = std::max(bounds.max.x, float(x + glyph.xOffset + glyph.width));
			bounds.max.y = std::max(bounds.max.y, float(line * lineSkip + glyph.yOffset + glyph.height));
		}
	}

	if (bounds.max.x < bounds.min.x) {
		std::swap(bounds.min.x, bounds.max.x);
	}
	if (bounds.max.y < bounds.min.y) {
		std::swap(bounds.min.y, bounds.max.y);
	}

	return bounds;
}

void Preview::drawGlyph(const FileEdit& fileEdit, tref::Codepoint cp, const TextBounds& bounds, int& line, int& x) const
{
	if (cp == '\n') {
		++line;
		x = 0;
		return;
	}
	else if (!fileEdit.glyphs().contains(cp)) {
		cp = '\0';
	}

	const tref::Glyph& glyph{fileEdit.glyphs().at(cp)};
	if (glyph.width == 0 || glyph.height == 0) {
		x += glyph.advance;
		return;
	}

	const glm::vec2 offset{glyph.x, glyph.y};
	const glm::vec2 size{glyph.width, glyph.height};
	const glm::vec2 textureSize{fileEdit.texture().size()};
	const ImVec2    uv1{std::bit_cast<ImVec2>(offset / textureSize)};
	const ImVec2    uv2{std::bit_cast<ImVec2>((offset + size) / textureSize)};
	ImGui::SetCursorPosX(x + glyph.xOffset - bounds.min.x);
	ImGui::SetCursorPosY(line * fileEdit.lineSkip() - bounds.min.y + glyph.yOffset);
	ImGui::Image(tr::ImGui::getTextureID(fileEdit.texture()), std::bit_cast<ImVec2>(size), uv1, uv2);

	x += glyph.advance;
}