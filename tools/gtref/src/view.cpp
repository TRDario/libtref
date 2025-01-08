#include "../include/view.hpp"
#include <imgui.h>

constexpr std::array<float, 24> ZOOM_LEVELS{-INFINITY, 1.0 / 16.0, 1.0 / 11.0, 1.0 / 8.0, 1.0 / 5.5, 1.0 / 4.0,
											1.0 / 3.0, 1.0 / 2.0,  2.0 / 3.0,  1,         1.5,       2,
											2.5,       3,          4,          5.5,       8,         11,
											16,        23,         32,         45,        64,        INFINITY};

/// PUBLIC FUNCTIONS ///

float View::zoom() const noexcept
{
	return _zoom;
}

const glm::vec2& View::scroll() const noexcept
{
	return _scroll;
}

void View::reset() noexcept
{
	_zoom   = 1;
	_scroll = {};
}

bool View::handleInput() noexcept
{
	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
		if (ImGui::IsKeyDown(ImGuiKey_ModCtrl)) {
			if (ImGui::GetIO().MouseWheel > 0) {
				zoomIn();
			}
			else if (ImGui::GetIO().MouseWheel < 0) {
				zoomOut();
			}
		}
		else if (ImGui::IsKeyDown(ImGuiKey_ModShift)) {
			if (ImGui::GetIO().MouseWheel > 0) {
				_scroll.x -= 7.5f * _zoom;
			}
			else if (ImGui::GetIO().MouseWheel < 0) {
				_scroll.x += 7.5f * _zoom;
			}
		}
		else {
			if (ImGui::GetIO().MouseWheel > 0) {
				_scroll.y -= 7.5f * _zoom;
			}
			else if (ImGui::GetIO().MouseWheel < 0) {
				_scroll.y += 7.5f * _zoom;
			}
		}
	}

	// Set mode to dragging if the mouse is dragging in the window, keep dragging mode one frame after letting go
	// to prevent triggering glyph selection/deselection, and then reset to false.
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
		if (!_dragging && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
			_dragging = true;
		}

		if (_dragging) {
			const ImVec2 delta{ImGui::GetIO().MouseDelta};
			_scroll.x -= delta.x;
			_scroll.y -= delta.y;
			return true;
		}
		else {
			return false;
		}
	}
	else {
		if (_dragging) {
			_dragging = false;
			return true;
		}
		else {
			return false;
		}
	}
}

void View::addMenu()
{
	if (ImGui::BeginMenu("View")) {
		if (ImGui::MenuItem("Zoom In", "Ctrl+=")) {
			zoomIn();
		}
		if (ImGui::MenuItem("Zoom Out", "Ctrl+-")) {
			zoomOut();
		}
		if (ImGui::MenuItem("Reset View", "Ctrl+0")) {
			reset();
		}
		ImGui::EndMenu();
	}

	if (!ImGui::GetIO().WantCaptureKeyboard) {
		if (ImGui::IsKeyChordPressed(ImGuiKey_ModCtrl | ImGuiKey_Equal)) {
			zoomIn();
		}
		else if (ImGui::IsKeyChordPressed(ImGuiKey_ModCtrl | ImGuiKey_Minus)) {
			zoomOut();
		}
		else if (ImGui::IsKeyChordPressed(ImGuiKey_ModCtrl | ImGuiKey_0)) {
			reset();
		}
	}
}

void View::addZoomControl() noexcept
{
	float zoomInput = _zoom * 100;
	ImGui::SetNextItemWidth(96);
	if (ImGui::InputFloat("##Zoom", &zoomInput, 0.0f, 0.0f, "%.2f%%")) {
		_zoom = std::clamp(zoomInput / 100, 1.0f / 16, 64.0f);
	}
}

/// INTERNAL FUNCTIONS ///

void View::zoomIn() noexcept
{
	// Cannot just do next like in zoomOut because it would skip in the case of *it != _zoom.
	auto it{std::ranges::lower_bound(ZOOM_LEVELS, _zoom)};
	if (*it == _zoom) {
		++it;
	}

	if (*it != INFINITY) {
		setZoom(*it);
	}
}

void View::zoomOut() noexcept
{
	auto next{*std::prev(std::ranges::lower_bound(ZOOM_LEVELS, _zoom))};
	if (next != -INFINITY) {
		setZoom(next);
	}
}

void View::setZoom(float zoom) noexcept
{
	_scroll *= (zoom / _zoom);
	_zoom = zoom;
}