#pragma once
#include <glm/glm.hpp>
#include <list>
#include <optional>
#include <tref/tref.hpp>
#include <variant>

struct Font;
class Selection;

// Object holding the edit history.
class History {
  public:
	// A of a new glyph.
	struct GlyphAddition {
		tref::Codepoint cp;
	};
	// Deletion of an existing glyph.
	struct GlyphDeletion {
		tref::Codepoint cp;
		tref::Glyph     glyph;
	};
	// Editing of a codepoint of a glyph.
	struct GlyphCodepointEdit {
		tref::Codepoint prev;
		tref::Codepoint next;
	};
	// Editing of the texture offset of a glyph.
	struct GlyphTextureOffsetEdit {
		tref::Codepoint cp;
		glm::u16vec2    prev;
		glm::u16vec2    next;
	};
	// Editing of the size of a glyph.
	struct GlyphSizeEdit {
		tref::Codepoint cp;
		glm::u16vec2    prev;
		glm::u16vec2    next;
	};
	// Editing of the placement offset of a glyph.
	struct GlyphPlacementEdit {
		tref::Codepoint cp;
		glm::i16vec2    prev;
		glm::i16vec2    next;
	};
	// Editing of the advancement of a glyph.
	struct GlyphAdvanceEdit {
		tref::Codepoint cp;
		std::int16_t    prev;
		std::int16_t    next;
	};
	// Editing of the line skip of the font.
	struct LineSkipEdit {
		std::int32_t prev;
		std::int32_t next;
	};
	// Generic font edit that can be undone and redone.
	using Edit = std::variant<GlyphAddition, GlyphDeletion, GlyphCodepointEdit, GlyphTextureOffsetEdit, GlyphSizeEdit,
							  GlyphPlacementEdit, GlyphAdvanceEdit, LineSkipEdit>;

	// Returns true if undo can be used.
	bool canUndo() const noexcept;

	// Returns true if redo can be used.
	bool canRedo() const noexcept;

	// Gets whether the history is currently at the saved point.
	bool atSavePoint() const noexcept;

	// Sets the filename of the edited file.
	void setFilename(const std::string& filename);

	// Clears the history.
	void clear() noexcept;

	// Sets the point where the file is considered unedited.
	void setSavePoint() noexcept;

	// Undoes an edit.
	void undo(Selection& selection, Font& font);

	// Redoes an edit.
	void redo(Selection& selection, Font& font);

	// Adds an edit to the history.
	void addEdit(const Edit& edit);

  private:
	// The list of saved edits. It can be traversed up and down.
	std::list<Edit> _edits{};
	// Iterator to one past the last edit.
	std::list<Edit>::iterator _curEdit{_edits.end()};
	// Optional iterator to the edit where the file is unmodified.
	std::optional<std::list<Edit>::iterator> _savedEdit{};
	// If _filename = "File.tref" -> _uneditedWindowTitle = "File.tref - gtref".
	std::string _savedWindowTitle{};
	// If _filename = "File.tref" -> _uneditedWindowTitle = "File.tref * - gtref".
	std::string _editedWindowTitle{};

	// Sets the window title.
	void setWindowTitle() const noexcept;

	// Checks if merging into an existing edit instead of creating a new one is possible.
	bool canMergeIntoExistingEdit(const Edit& edit) const noexcept;

	// Merges an edit into an existing edit.
	void mergeIntoExistingEdit(const Edit& edit) noexcept;

	// Adds a new edit.
	void pushNewEdit(const Edit& edit);
};