#include "../include/file_edit.hpp"
#include "../include/selection.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <tinyfiledialogs.h>
#include <tr/imgui.hpp>

constexpr std::uint32_t GLYPH_FILL_COLOR{0x40FF8080};

constexpr std::uint32_t GLYPH_OUTLINE_COLOR{0x80FFFFFF};

/// PUBLIC FUNCTIONS ///

FileEdit::FileEdit(const char* cpath)
{
	if (cpath != nullptr) {
		const std::filesystem::path path{cpath};

		std::array<char, 4> buffer;
		try {
			std::ifstream file{tr::openFileR(path, std::ios::binary)};
			tr::readBinary(file, buffer);
		}
		catch (std::exception& err) {
			const std::string message{std::format("Failed to open {}.", path.string())};
			tinyfd_messageBox("Error - gtref", message.c_str(), "ok", "error", 0);
			return;
		}

		if (std::string_view{buffer.data(), 4} == "TREF") {
			std::optional<LoadResult> loadResult{loadFont(path)};
			if (loadResult.has_value()) {
				_file.emplace(*std::move(loadResult));
				_saved    = _file->font;
				_path     = path;
				_filename = _path.filename().string();
				_file->history.setFilename(_filename);
				_file->history.setSavePoint();
			}
		}
		else {
			std::optional<LoadResult> loadResult{loadImage(path)};
			if (loadResult.has_value()) {
				_file.emplace(*std::move(loadResult));
				_filename = "Untitled";
				_file->history.setFilename(_filename);
				tr::window().setTitle("Untitled * - gtref");
			}
		}
	}
}

bool FileEdit::empty() const noexcept
{
	return !_file.has_value();
}

std::int32_t FileEdit::lineSkip() const noexcept
{
	GTREF_ASSERT(!empty());

	return _file->font.lineSkip;
}

const tref::GlyphMap& FileEdit::glyphs() const noexcept
{
	GTREF_ASSERT(!empty());

	return _file->font.glyphs;
}

const tr::Texture2D& FileEdit::texture() const noexcept
{
	GTREF_ASSERT(!empty());

	return _file->texture;
}

Signal FileEdit::addFileMenu(Selection& selection, View& view)
{
	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("New Font...", "Ctrl+N")) {
			newFont(selection, view);
		}
		if (ImGui::MenuItem("Open Font File...", "Ctrl+O")) {
			openFontFile(selection, view);
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Save", "Ctrl+S", false, !empty())) {
			save();
		}
		if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S", false, !empty())) {
			saveAs();
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Revert File", "Ctrl+R", false, canRevert())) {
			revert(selection);
		}
		if (ImGui::MenuItem("Close", "Ctrl+W", false, !empty())) {
			close(selection);
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Quit", "Ctrl+Q")) {
			if (close(selection)) {
				return Signal::EXIT;
			}
		}
		ImGui::EndMenu();
	}

	if (!ImGui::GetIO().WantCaptureKeyboard) {
		if (ImGui::IsKeyChordPressed(ImGuiKey_ModCtrl | ImGuiKey_N)) {
			newFont(selection, view);
		}
		else if (ImGui::IsKeyChordPressed(ImGuiKey_ModCtrl | ImGuiKey_O)) {
			openFontFile(selection, view);
		}
		else if (!empty() && ImGui::IsKeyChordPressed(ImGuiKey_ModCtrl | ImGuiKey_S)) {
			save();
		}
		else if (!empty() && ImGui::IsKeyChordPressed(ImGuiKey_ModCtrl | ImGuiKey_ModShift | ImGuiKey_S)) {
			saveAs();
		}
		else if (canRevert() && ImGui::IsKeyChordPressed(ImGuiKey_ModCtrl | ImGuiKey_R)) {
			revert(selection);
		}
		else if (!empty() && ImGui::IsKeyChordPressed(ImGuiKey_ModCtrl | ImGuiKey_W)) {
			close(selection);
		}
		else if (ImGui::IsKeyChordPressed(ImGuiKey_ModCtrl | ImGuiKey_Q)) {
			if (close(selection)) {
				return Signal::EXIT;
			}
		}
	}

	return Signal::CONTINUE;
}

void FileEdit::addEditMenu(Selection& selection)
{
	if (ImGui::BeginMenu("Edit")) {
		if (ImGui::MenuItem("Undo", "Ctrl+Z", false, !empty() && _file->history.canUndo())) {
			_file->history.undo(selection, _file->font);
		}
		if (ImGui::MenuItem("Redo", "Ctrl+Y", false, !empty() && _file->history.canRedo())) {
			_file->history.redo(selection, _file->font);
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Insert Glyph", "Ins", false, !empty())) {
			insertGlyph(selection);
		}
		if (ImGui::MenuItem("Delete Selected", "Del", false, !selection.empty() && selection.value() != '\0')) {
			deleteGlyph(selection);
		}
		ImGui::EndMenu();
	}

	if (!ImGui::GetIO().WantCaptureKeyboard) {
		if (!empty() && _file->history.canUndo() && ImGui::IsKeyChordPressed(ImGuiKey_ModCtrl | ImGuiKey_Z)) {
			_file->history.undo(selection, _file->font);
		}
		else if (!empty() && _file->history.canRedo() && ImGui::IsKeyChordPressed(ImGuiKey_ModCtrl | ImGuiKey_Y)) {
			_file->history.redo(selection, _file->font);
		}
		else if (!empty() && ImGui::IsKeyChordPressed(ImGuiKey_Insert)) {
			insertGlyph(selection);
		}
		else if (!selection.empty() && selection.value() != '\0' && ImGui::IsKeyChordPressed(ImGuiKey_Delete)) {
			deleteGlyph(selection);
		}
		else {
			handleArrowKeys(selection);
		}
	}
}

void FileEdit::addWorkspace(Selection& selection, View& view)
{
	constexpr ImGuiWindowFlags WINDOW_FLAGS{ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
											ImGuiWindowFlags_NoScrollWithMouse};
	const ImVec2 workspaceSize{!selection.empty() ? -300.0f : 0.0f, -ImGui::GetTextLineHeightWithSpacing()};

	ImGui::PushStyleColor(ImGuiCol_ChildBg, 0xFF1C1C1C);
	if (ImGui::BeginChild("Workspace", workspaceSize, ImGuiChildFlags_Borders, WINDOW_FLAGS)) {
		const bool dragging{view.handleInput()};
		if (!empty()) {
			const glm::vec2 imageSize{glm::vec2{_file->bitmap.size()} * view.zoom()};
			const glm::vec2 windowSize{std::bit_cast<glm::vec2>(ImGui::GetContentRegionAvail())};
			const glm::vec2 initialCursorPos{std::bit_cast<glm::vec2>(ImGui::GetCursorPos())};
			const glm::vec2 cursorPos{initialCursorPos + (windowSize - imageSize) / 2.0f - view.scroll()};

			ImGui::SetCursorPos(std::bit_cast<ImVec2>(cursorPos));
			ImGui::Image(tr::ImGui::getTextureID(_file->texture), std::bit_cast<ImVec2>(imageSize), {0, 0}, {1, 1},
						 {1, 1, 1, 1}, {0.25, 0.25, 0.25, 1});

			for (auto& [cp, glyph] : glyphs()) {
				const glm::vec2 tl{cursorPos + glm::vec2(glyph.x, glyph.y) * view.zoom()};
				ImGui::PushID(cp);
				if (glyph.width == 0 || glyph.height == 0) {
					handleWhitespaceGlyph(selection, cp, tl, dragging);
				}
				else {
					const glm::vec2 size{glm::vec2(glyph.width, glyph.height) * view.zoom()};
					handleRegularGlyph(selection, cp, tl, size, dragging);
				}
				ImGui::PopID();
			}

			if (selection.fresh()) {
				selection.setStale();
			}
			else if (!dragging && ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
				selection.reset();
			}
		}
	}
	ImGui::EndChild();
	ImGui::PopStyleColor();
}

void FileEdit::addGlyphPropertiesEditor(Selection& selection)
{
	if (selection.empty()) {
		return;
	}

	GTREF_ASSERT(glyphs().contains(selection.value()));

	constexpr std::uint16_t U16_1{1};
	constexpr std::uint16_t U16_10{10};
	constexpr std::int16_t  I16_1{1};
	constexpr std::int16_t  I16_10{10};

	tref::Codepoint cpEdit{selection.value()};
	tref::Glyph     glyphEdit{glyphs().at(selection.value())};
	ImGui::SameLine();
	if (ImGui::BeginChild("Glyph Properties", ImVec2(300, -ImGui::GetTextLineHeightWithSpacing()),
						  ImGuiChildFlags_Borders, ImGuiWindowFlags_NoMove)) {
		ImGui::SeparatorText("Glyph Properties");
		ImGui::PushItemWidth(185);
		addCodepointEditor(selection, cpEdit);
		ImGui::SeparatorText("Texture Parameters");
		if (ImGui::InputScalar("X-offset", ImGuiDataType_U16, &glyphEdit.x, &U16_1, &U16_10, "%d")) {
			editGlyphTextureX(cpEdit, glyphEdit.x);
		}
		if (ImGui::InputScalar("Y-offset", ImGuiDataType_U16, &glyphEdit.y, &U16_1, &U16_10, "%d")) {
			editGlyphTextureY(cpEdit, glyphEdit.y);
		}
		if (ImGui::InputScalar("Width", ImGuiDataType_U16, &glyphEdit.width, &U16_1, &U16_10, "%d")) {
			editGlyphWidth(cpEdit, glyphEdit.width);
		}
		if (ImGui::InputScalar("Height", ImGuiDataType_U16, &glyphEdit.height, &U16_1, &U16_10, "%d")) {
			editGlyphHeight(cpEdit, glyphEdit.height);
		}
		ImGui::SeparatorText("Placement Parameters");
		if (ImGui::InputScalar("X-offset##1", ImGuiDataType_S16, &glyphEdit.xOffset, &I16_1, &I16_10)) {
			editGlyphPlacementX(cpEdit, glyphEdit.xOffset);
		}
		if (ImGui::InputScalar("Y-offset##1", ImGuiDataType_S16, &glyphEdit.yOffset, &I16_1, &I16_10)) {
			editGlyphPlacementY(cpEdit, glyphEdit.yOffset);
		}
		if (ImGui::InputScalar("Advance", ImGuiDataType_S16, &glyphEdit.advance, &I16_1, &I16_10)) {
			editGlyphAdvance(cpEdit, glyphEdit.advance);
		}
		ImGui::PopItemWidth();
	}
	ImGui::EndChild();
}

void FileEdit::addLineSkipEditor()
{
	if (!empty()) {
		int lineSkipInput = _file->font.lineSkip;
		ImGui::SameLine(ImGui::GetWindowWidth() - 210);
		ImGui::SetNextItemWidth(110);
		if (ImGui::InputInt("Line Skip", &lineSkipInput, 1, 10)) {
			editLineSkip(lineSkipInput);
		}
	}
}

bool FileEdit::close(Selection& selection) noexcept
{
	if (!_file.has_value() || _file->history.atSavePoint() || confirmClose()) {
		_file.reset();
		_saved.reset();
		_path.clear();
		_filename.clear();
		tr::window().setTitle("gtref");
		selection.reset();
		return true;
	}
	return false;
}

/// INTERNAL FUNCTIONS ///

FileEdit::File::File(LoadResult&& loadResult)
	: font{std::move(loadResult.font)}
	, bitmap{std::move(loadResult.image)}
	, texture{bitmap, tr::ALL_MIPMAPS, tr::TextureFormat::RGBA8}
{
	tr::ImGui::setTextureFilter(texture, tr::MinFilter::LMIPS_LINEAR, tr::MagFilter::NEAREST);
}

void FileEdit::handleArrowKeys(Selection& selection)
{
	if (selection.empty()) {
		return;
	}

	const tref::Glyph& glyph{glyphs().at(selection.value())};
	if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
		if (ImGui::IsKeyDown(ImGuiKey_ModShift)) {
			if (glyph.height != 0) {
				if (ImGui::IsKeyDown(ImGuiKey_ModCtrl)) {
					editGlyphHeight(selection.value(), std::max(glyph.height - 10, 0));
				}
				else {
					editGlyphHeight(selection.value(), glyph.height - 1);
				}
			}
		}
		else {
			if (glyph.y != 0) {
				if (ImGui::IsKeyDown(ImGuiKey_ModCtrl)) {
					editGlyphTextureY(selection.value(), std::max(glyph.y - 10, 0));
				}
				else {
					editGlyphTextureY(selection.value(), glyph.y - 1);
				}
			}
		}
	}
	else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
		if (ImGui::IsKeyDown(ImGuiKey_ModShift)) {
			if (glyph.y + glyph.height != _file->bitmap.size().y) {
				if (ImGui::IsKeyDown(ImGuiKey_ModCtrl)) {
					editGlyphHeight(selection.value(), std::min(glyph.height + 10, _file->bitmap.size().y - glyph.y));
				}
				else {
					editGlyphHeight(selection.value(), glyph.height + 1);
				}
			}
		}
		else {
			if (glyph.y + glyph.height != _file->bitmap.size().y) {
				if (ImGui::IsKeyDown(ImGuiKey_ModCtrl)) {
					editGlyphTextureY(selection.value(), std::min(glyph.y + 10, _file->bitmap.size().y - glyph.height));
				}
				else {
					editGlyphTextureY(selection.value(), glyph.y + 1);
				}
			}
		}
	}
	if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
		if (ImGui::IsKeyDown(ImGuiKey_ModShift)) {
			if (glyph.width != 0) {
				if (ImGui::IsKeyDown(ImGuiKey_ModCtrl)) {
					editGlyphWidth(selection.value(), std::max(glyph.width - 10, 0));
				}
				else {
					editGlyphWidth(selection.value(), glyph.width - 1);
				}
			}
		}
		else {
			if (glyph.x != 0) {
				if (ImGui::IsKeyDown(ImGuiKey_ModCtrl)) {
					editGlyphTextureX(selection.value(), std::max(glyph.x - 10, 0));
				}
				else {
					editGlyphTextureX(selection.value(), glyph.x - 1);
				}
			}
		}
	}
	else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
		if (ImGui::IsKeyDown(ImGuiKey_ModShift)) {
			if (glyph.x + glyph.width != _file->bitmap.size().x) {
				if (ImGui::IsKeyDown(ImGuiKey_ModCtrl)) {
					editGlyphWidth(selection.value(), std::min(glyph.width + 10, _file->bitmap.size().x - glyph.x));
				}
				else {
					editGlyphWidth(selection.value(), glyph.width + 1);
				}
			}
		}
		else {
			if (glyph.x + glyph.width != _file->bitmap.size().x) {
				if (ImGui::IsKeyDown(ImGuiKey_ModCtrl)) {
					editGlyphTextureX(selection.value(), std::min(glyph.x + 10, _file->bitmap.size().x - glyph.width));
				}
				else {
					editGlyphTextureX(selection.value(), glyph.x + 1);
				}
			}
		}
	}
}

bool FileEdit::newFont(Selection& selection, View& view)
{
	constexpr const char* PATTERNS[]{"*.bmp", "*.png", "*.jpg", "*.jpeg", "*.qoi"};
	const char*           path{tinyfd_openFileDialog("Choose Image File", nullptr, 5, PATTERNS, "Image Files", 0)};
	if (path != nullptr) {
		if (!close(selection)) {
			return false;
		}
		std::optional<LoadResult> loadResult{loadImage(path)};
		if (!loadResult.has_value()) {
			return false;
		}
		_file.emplace(*std::move(loadResult));
		_filename = "Untitled";
		_file->history.setFilename(_filename);
		selection.reset();
		view.reset();
		tr::window().setTitle("Untitled * - gtref");
		return true;
	}
	else {
		return false;
	}
}

bool FileEdit::openFontFile(Selection& selection, View& view)
{
	constexpr const char* PATTERNS[]{"*.tref"};
	const char*           path{tinyfd_openFileDialog("Open Font File", nullptr, 1, PATTERNS, "Font Files", 0)};
	if (path != nullptr) {
		if (!close(selection)) {
			return false;
		}
		std::optional<LoadResult> loadResult{loadFont(path)};
		if (!loadResult.has_value()) {
			return false;
		}
		_file.emplace(*std::move(loadResult));
		_saved    = _file->font;
		_path     = path;
		_filename = _path.filename().string();
		_file->history.setFilename(_filename);
		_file->history.setSavePoint();
		selection.reset();
		view.reset();
		return true;
	}
	else {
		return false;
	}
}

void FileEdit::saveAs(const std::filesystem::path& path)
{
	GTREF_ASSERT(!empty());

	saveFont(path, _file->font, _file->bitmap);
	_saved = _file->font;
	if (_path != path) {
		_path     = path;
		_filename = path.filename().string();
		_file->history.setFilename(_filename);
	}
	_file->history.setSavePoint();
}

bool FileEdit::save()
{
	GTREF_ASSERT(!empty());

	if (_path.empty()) {
		return saveAs();
	}
	else {
		saveAs(_path);
		return true;
	}
}

bool FileEdit::saveAs()
{
	GTREF_ASSERT(!empty());

	constexpr const char* PATTERNS[]{"*.tref"};
	const char*           path{tinyfd_saveFileDialog("Save As", _filename.c_str(), 1, PATTERNS, "Font Files")};
	if (path != nullptr) {
		saveAs(path);
		return true;
	}
	else {
		return false;
	}
}

bool FileEdit::confirmClose() noexcept
{
	GTREF_ASSERT(!empty());

	const std::string message{std::format("Do you want to save the changes you made to {}?", _filename)};
	switch (tinyfd_messageBox("gtref", message.c_str(), "yesnocancel", "question", 1)) {
	case 0: // Cancel
		return false;
	case 1: // Yes
		return save();
	case 2: // No
		return true;
	default:
		return false;
	}
}

bool FileEdit::canRevert() const noexcept
{
	return _saved.has_value();
}

void FileEdit::revert(Selection& selection)
{
	GTREF_ASSERT(!empty());
	GTREF_ASSERT(canRevert());

	_file->font = *_saved;
	_file->history.clear();
	_file->history.setSavePoint();
	selection.reset();
}

void FileEdit::insertGlyph(Selection& selection)
{
	GTREF_ASSERT(!empty());

	tref::Codepoint cp{*rs::find_if(std::views::iota(0U), [&](auto cp) { return !glyphs().contains(cp); })};

	_file->history.addEdit(History::GlyphAddition{cp});
	_file->font.glyphs.emplace(cp, tref::Glyph{});
	selection.setStale(cp);
}

void FileEdit::deleteGlyph(Selection& selection)
{
	GTREF_ASSERT(!empty());
	GTREF_ASSERT(!selection.empty());
	GTREF_ASSERT(glyphs().contains(selection.value()));
	GTREF_ASSERT(selection.value() != '\0');

	_file->history.addEdit(History::GlyphDeletion{selection.value(), glyphs().at(selection.value())});
	_file->font.glyphs.erase(selection.value());
	selection.reset();
}

void FileEdit::handleWhitespaceGlyph(Selection& selection, tref::Codepoint cp, const glm::vec2& tl, bool dragging)
{
	const glm::vec2 offset{std::bit_cast<glm::vec2>(ImGui::GetWindowPos())};
	const glm::vec2 scroll{ImGui::GetScrollX(), ImGui::GetScrollY()};
	const ImVec2    drawPos{std::bit_cast<ImVec2>(offset - scroll + tl)};

	ImGui::SetCursorPos(std::bit_cast<ImVec2>(tl - 5.0f));
	if (!dragging && ImGui::InvisibleButton("", {10, 10})) {
		selection.setFresh(cp);
	}

	ImGui::GetWindowDrawList()->AddCircleFilled(drawPos, 5.0f, GLYPH_FILL_COLOR);
	if (selection == cp) {
		ImGui::GetWindowDrawList()->AddCircle(drawPos, 5.0f, GLYPH_OUTLINE_COLOR);
	}
}

void FileEdit::handleRegularGlyph(Selection& selection, tref::Codepoint cp, const glm::vec2& tl, const glm::vec2& size,
								  bool dragging)
{
	const glm::vec2 offset{std::bit_cast<glm::vec2>(ImGui::GetWindowPos())};
	const glm::vec2 scroll{ImGui::GetScrollX(), ImGui::GetScrollY()};
	const ImVec2    drawStart{std::bit_cast<ImVec2>(offset - scroll + tl)};
	const ImVec2    drawEnd{std::bit_cast<ImVec2>(offset - scroll + tl + size)};

	ImGui::SetCursorPos(std::bit_cast<ImVec2>(tl));
	if (!dragging && ImGui::InvisibleButton("", std::bit_cast<ImVec2>(size))) {
		selection.setFresh(cp);
	}

	ImGui::GetWindowDrawList()->AddRectFilled(drawStart, drawEnd, GLYPH_FILL_COLOR);
	if (selection == cp) {
		ImGui::GetWindowDrawList()->AddRect(drawStart, drawEnd, GLYPH_OUTLINE_COLOR);
	}
}

void FileEdit::editGlyphCodepoint(tref::Codepoint old, tref::Codepoint now)
{
	GTREF_ASSERT(!empty());
	GTREF_ASSERT(glyphs().contains(old));
	GTREF_ASSERT(!glyphs().contains(now));
	GTREF_ASSERT(old != '\0');

	_file->history.addEdit(History::GlyphCodepointEdit{old, now});
	_file->font.glyphs.emplace(now, glyphs().at(old));
	_file->font.glyphs.erase(old);
}

void FileEdit::editGlyphTextureX(tref::Codepoint cp, std::uint16_t offset)
{
	GTREF_ASSERT(!empty());
	GTREF_ASSERT(glyphs().contains(cp));

	tref::Glyph& glyph{_file->font.glyphs.at(cp)};

	_file->history.addEdit(History::GlyphTextureOffsetEdit{cp, {glyph.x, glyph.y}, {offset, glyph.y}});
	glyph.x = offset;
}

void FileEdit::editGlyphTextureY(tref::Codepoint cp, std::uint16_t offset)
{
	GTREF_ASSERT(!empty());
	GTREF_ASSERT(glyphs().contains(cp));

	tref::Glyph& glyph{_file->font.glyphs.at(cp)};

	_file->history.addEdit(History::GlyphTextureOffsetEdit{cp, {glyph.x, glyph.y}, {glyph.x, offset}});
	glyph.y = offset;
}

void FileEdit::editGlyphWidth(tref::Codepoint cp, std::uint16_t width)
{
	GTREF_ASSERT(!empty());
	GTREF_ASSERT(glyphs().contains(cp));

	tref::Glyph& glyph{_file->font.glyphs.at(cp)};

	_file->history.addEdit(History::GlyphSizeEdit{cp, {glyph.width, glyph.height}, {width, glyph.height}});
	glyph.width = width;
}

void FileEdit::editGlyphHeight(tref::Codepoint cp, std::uint16_t height)
{
	GTREF_ASSERT(!empty());
	GTREF_ASSERT(glyphs().contains(cp));

	tref::Glyph& glyph{_file->font.glyphs.at(cp)};

	_file->history.addEdit(History::GlyphSizeEdit{cp, {glyph.width, glyph.height}, {glyph.width, height}});
	glyph.height = height;
}

void FileEdit::editGlyphPlacementX(tref::Codepoint cp, std::int16_t offset)
{
	GTREF_ASSERT(!empty());
	GTREF_ASSERT(glyphs().contains(cp));

	tref::Glyph& glyph{_file->font.glyphs.at(cp)};

	_file->history.addEdit(History::GlyphPlacementEdit{cp, {glyph.xOffset, glyph.yOffset}, {offset, glyph.yOffset}});
	glyph.xOffset = offset;
}

void FileEdit::editGlyphPlacementY(tref::Codepoint cp, std::int16_t offset)
{
	GTREF_ASSERT(!empty());
	GTREF_ASSERT(glyphs().contains(cp));

	tref::Glyph& glyph{_file->font.glyphs.at(cp)};

	_file->history.addEdit(History::GlyphPlacementEdit{cp, {glyph.xOffset, glyph.yOffset}, {glyph.xOffset, offset}});
	glyph.yOffset = offset;
}

void FileEdit::editGlyphAdvance(tref::Codepoint cp, std::int16_t advance)
{
	GTREF_ASSERT(!empty());
	GTREF_ASSERT(glyphs().contains(cp));

	tref::Glyph& glyph{_file->font.glyphs.at(cp)};

	_file->history.addEdit(History::GlyphAdvanceEdit{cp, glyph.advance, advance});
	glyph.advance = advance;
}

void FileEdit::addCodepointEditor(Selection& selection, tref::Codepoint& cpEdit)
{
	GTREF_ASSERT(!empty());
	GTREF_ASSERT(!selection.empty());

	if (selection.value() == '\0') {
		ImGui::BeginDisabled();
	}

	if (ImGui::InputScalar("Codepoint", ImGuiDataType_U32, &cpEdit, nullptr, nullptr, "%X",
						   ImGuiInputTextFlags_CharsHexadecimal)) {
		if (!glyphs().contains(cpEdit) && cpEdit < 0x110000) {
			editGlyphCodepoint(selection.value(), cpEdit);
			selection.setStale(cpEdit);
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Text, 0xFF0000FF);
			ImGui::Text("Invalid/occupied codepoint!");
			ImGui::PopStyleColor();
			cpEdit = selection.value();
		}
	}
	if (cpEdit == '\0') {
		ImGui::Text(" = (Fallback Character)");
	}
	else if (cpEdit < ' ' || cpEdit == 0x7F) {
		ImGui::Text(" = (ASCII Control Character)");
	}
	else {
		std::array<char, 5> buf{};
		if (cpEdit < 0x80) {
			buf[0] = cpEdit;
		}
		else if (cpEdit < 0x800) {
			buf[0] = 0xC0 | (cpEdit >> 6);
			buf[1] = 0x80 | (cpEdit & 0x3F);
		}
		else if (cpEdit < 0x10000) {
			buf[0] = 0xE0 | (cpEdit >> 12);
			buf[1] = 0x80 | ((cpEdit >> 6) & 0x3F);
			buf[2] = 0x80 | (cpEdit & 0x3F);
		}
		else if (cpEdit < 0x110000) {
			buf[0] = 0xF0 | (cpEdit >> 18);
			buf[1] = 0x80 | ((cpEdit >> 12) & 0x3F);
			buf[2] = 0x80 | ((cpEdit >> 6) & 0x3F);
			buf[3] = 0x80 | (cpEdit & 0x3F);
		}

		ImGui::Text(" = '%s'", buf.data());
	}

	if (selection.value() == '\0') {
		ImGui::EndDisabled();
	}
}

void FileEdit::editLineSkip(std::int32_t lineSkip)
{
	GTREF_ASSERT(!empty());

	_file->history.addEdit(History::LineSkipEdit{_file->font.lineSkip, lineSkip});
	_file->font.lineSkip = lineSkip;
}