#include "../include/file_edit.hpp"
#include "../include/selection.hpp"
#include <imgui.h>

/// PUBLIC FUNCTIONS ///

bool Selection::operator==(const tref::Codepoint& cp) const noexcept
{
	return _state != State::EMPTY && _cp == cp;
}

bool Selection::empty() const noexcept
{
	return _state == State::EMPTY;
}

bool Selection::fresh() const noexcept
{
	return _state == State::FRESH;
}

tref::Codepoint Selection::value() const noexcept
{
	return _cp;
}

void Selection::setFresh(tref::Codepoint cp) noexcept
{
	_cp    = cp;
	_state = State::FRESH;
}

void Selection::setStale(tref::Codepoint cp) noexcept
{
	_cp    = cp;
	_state = State::STALE;
}

void Selection::setStale() noexcept
{
	_state = State::STALE;
}

void Selection::reset() noexcept
{
	_state = State::EMPTY;
}

void Selection::addMenu(const FileEdit& fileEdit)
{
	if (ImGui::BeginMenu("Selection")) {
		if (ImGui::MenuItem("Select Next", "Tab", false, !fileEdit.empty())) {
			selectNext(fileEdit);
		}
		if (ImGui::MenuItem("Select Previous", "Shift+Tab", false, !fileEdit.empty())) {
			selectPrevious(fileEdit);
		}
		if (ImGui::MenuItem("Clear Selection", "Esc", false, !empty())) {
			reset();
		}
		ImGui::EndMenu();
	}

	if (!ImGui::GetIO().WantCaptureKeyboard) {
		if (!fileEdit.empty() && ImGui::IsKeyChordPressed(ImGuiKey_Tab)) {
			selectNext(fileEdit);
		}
		else if (!fileEdit.empty() && ImGui::IsKeyChordPressed(ImGuiKey_ModShift | ImGuiKey_Tab)) {
			selectPrevious(fileEdit);
		}
		else if (!empty() && ImGui::IsKeyChordPressed(ImGuiKey_Escape)) {
			reset();
		}
	}
}

/// INTERNAL FUNCTIONS ///

void Selection::selectNext(const FileEdit& fileEdit)
{
	const auto minmax{std::ranges::minmax_element(fileEdit.glyphs() | std::views::keys)};
	if (_state == State::EMPTY || _cp == *minmax.max) {
		setStale(*minmax.min);
	}
	else {
		const auto range{std::views::iota(_cp + 1, *minmax.max)};
		setStale(*std::ranges::find_if(range, [&](tref::Codepoint cp) { return fileEdit.glyphs().contains(cp); }));
	}
}

void Selection::selectPrevious(const FileEdit& fileEdit)
{
	const auto minmax{std::ranges::minmax_element(fileEdit.glyphs() | std::views::keys)};
	if (_state == State::EMPTY || _cp == *minmax.min) {
		setStale(*minmax.max);
	}
	else {
		const auto range{std::views::iota(*minmax.min, _cp) | std::views::reverse};
		setStale(*std::ranges::find_if(range, [&](tref::Codepoint cp) { return fileEdit.glyphs().contains(cp); }));
	}
}