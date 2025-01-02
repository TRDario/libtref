#pragma once
#include <tr/tr.hpp>
#include <tref/tref.hpp>

class FileEdit;

// Object representing the "Preview" menu and its related state.
class Preview {
  public:
	// Adds the "Preview" menu.
	void addMenu(const FileEdit& fileEdit);

	// Adds the "Preview" window.
	void addPreviewWindow(const FileEdit& fileEdit);

  private:
	struct TextBounds {
		glm::vec2 min;
		glm::vec2 max;
	};

	// Flag keeping track of the "Preview" window's state.
	bool _previewWindowActive{false};
	// Buffer holding the preview text.
	std::array<char, 127> _previewTextBuffer{};

	// Finds the bounds for the text in the buffer.
	TextBounds findTextBounds(const FileEdit& fileEdit) const noexcept;

	// Draws a glyph in the preview window.
	void drawGlyph(const FileEdit& fileEdit, tref::Codepoint cp, const TextBounds& bounds, int& line, int& x) const;
};