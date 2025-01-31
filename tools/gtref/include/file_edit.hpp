#pragma once
#include "common.hpp"
#include "font.hpp"
#include "history.hpp"
#include "view.hpp"

class Selection;

// Object representing the "File" and "Edit" menus and their related state.
class FileEdit {
  public:
	// Attempts to load a file from the command-line arguments, or if cpath is nullptr load empty.
	FileEdit(const char* cpath);

	// Returns true if no font is loaded in.
	bool empty() const noexcept;

	// Gets the line skip of the font.
	std::int32_t lineSkip() const noexcept;

	// Gets read-only access to the glyphs of the font.
	const tref::GlyphMap& glyphs() const noexcept;

	// Gets the font texture.
	const tr::ColorTexture2D& texture() const noexcept;

	// Tries to close the current font file, prompting to save if needed.
	bool close(Selection& selection) noexcept;

	// Adds the "File" menu.
	Signal addFileMenu(Selection& selection, View& view);

	// Adds the "Edit" menu.
	void addEditMenu(Selection& selection);

	// Adds the workspace view.
	void addWorkspace(Selection& selection, View& view);

	// Adds the "Glyph Properties" editor.
	void addGlyphPropertiesEditor(Selection& selection);

	// Adds the line skip editing UI.
	void addLineSkipEditor();

  private:
	struct File {
		Font               font;
		tr::Bitmap         bitmap;
		tr::ColorTexture2D texture;
		History            history;

		File(LoadResult&& loadResult);
	};

	// The live data being used and manipulated.
	std::optional<File> _file{};
	// A reference saved font for reversions.
	std::optional<Font> _saved{};
	// The path to the current font (if any).
	std::filesystem::path _path{};
	// The filename of the current font.
	std::string _filename{};

	/// PROCESS KEY DOWN INTERNAL FUNCTIONS ///

	// Handles arrow key shortcuts.
	void handleArrowKeys(Selection& selection);

	/// FILE MENU INTERNAL FUNCTIONS ///

	// Tries to open an image file.
	bool newFont(Selection& selection, View& view);

	// Tries to open a font file.
	bool openFontFile(Selection& selection, View& view);

	// Tries to save to the same file the font was loaded from.
	// If the current font was not loaded from file, behaves as saveAs().
	bool save();

	// Tries to save the current font to a file.
	bool saveAs();

	// Saves the current font to file.
	void saveAs(const std::filesystem::path& path);

	// Shows a dialog so the user can confirm the close action. Returns true if confirmed.
	bool confirmClose() noexcept;

	// Gets whether a file reversion can be done.
	bool canRevert() const noexcept;

	// Reverts all changes to the file.
	void revert(Selection& selection);

	/// EDIT MENU INTERNAL FUNCTIONS ///

	// Adds a new glyph.
	void insertGlyph(Selection& selection);

	// Deletes a glyph.
	void deleteGlyph(Selection& selection);

	/// WORKSPACE INTERNAL FUNCTIONS ///

	// Handles a glyph with 0 width/height.
	void handleWhitespaceGlyph(Selection& selection, tref::Codepoint cp, const glm::vec2& tl, bool dragging);

	// Handles a regular glyph.
	void handleRegularGlyph(Selection& selection, tref::Codepoint cp, const glm::vec2& tl, const glm::vec2& size,
							bool dragging);

	/// GLYPH EDITOR INTERNAL FUNCTIONS ///

	// Edits a glyph's codepoint.
	void editGlyphCodepoint(tref::Codepoint old, tref::Codepoint now);

	// Adds the codepoint editing part of the glyph properties editor.
	void addCodepointEditor(Selection& selection, tref::Codepoint& cpEdit);

	// Edits a glyph's X texture offset.
	void editGlyphTextureX(tref::Codepoint cp, std::uint16_t offset);

	// Edits a glyph's Y texture offset.
	void editGlyphTextureY(tref::Codepoint cp, std::uint16_t offset);

	// Edits a glyph's width.
	void editGlyphWidth(tref::Codepoint cp, std::uint16_t width);

	// Edits a glyph's height.
	void editGlyphHeight(tref::Codepoint cp, std::uint16_t height);

	// Edits a glyph's X offset.
	void editGlyphPlacementX(tref::Codepoint cp, std::int16_t offset);

	// Edits a glyph's Y offset.
	void editGlyphPlacementY(tref::Codepoint cp, std::int16_t offset);

	// Edits a glyph's advance.
	void editGlyphAdvance(tref::Codepoint cp, std::int16_t advance);

	// Edits the line skip of the font.
	void editLineSkip(std::int32_t lineSkip);
};