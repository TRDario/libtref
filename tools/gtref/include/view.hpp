#pragma once
#include <tr/tr.hpp>

// Object representing the "View" menu and its related state.
class View {
  public:
	// Gets the current zoom level.
	float zoom() const noexcept;

	// Gets the current scrolling offset.
	const glm::vec2& scroll() const noexcept;

	// Resets the view.
	void reset() noexcept;

	// Handles zooming. Returns true if the mouse is dragging.
	bool handleInput() noexcept;

	// Adds the "View" menu.
	void addMenu();

	// Adds the zoom scale editing UI.
	void addZoomControl() noexcept;

  private:
	float     _zoom{1.0f};
	glm::vec2 _scroll{0, 0};
	bool      _dragging{false};

	// Sets the zoom level to the next higher one.
	void zoomIn() noexcept;

	// Sets the zoom level to the next lower one.
	void zoomOut() noexcept;

	// Sets the zoom level.
	void setZoom(float zoom) noexcept;
};