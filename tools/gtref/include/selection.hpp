#pragma once
#include <tr/tr.hpp>
#include <tref/tref.hpp>

class FileEdit;

// Object representing the "Selection" manu and its related state.
//
// NOTE: The selection can be "fresh" or "stale". This matters when processing the GUI due to how button hitboxes are
// implemented. Fresh selections won't let a glyph hitbox fallthrough, stale ones will.
class Selection {
  public:
	// Checks whether a codepoint is selected.
	bool operator==(const tref::Codepoint& cp) const noexcept;

	// Gets whether the selection is empty.
	bool empty() const noexcept;

	// Gets whether the selection is fresh.
	bool fresh() const noexcept;

	// Gets the selected glyph.
	tref::Codepoint value() const noexcept;

	// Sets a new codepoint as the fresh selected codepoint.
	void setFresh(tref::Codepoint cp) noexcept;

	// Sets a new codepoint as the stale selected codepoint.
	void setStale(tref::Codepoint cp) noexcept;

	// Makes the selection stale (Fresh selections won't let a glyph hitbox fallthrough, stale will).
	void setStale() noexcept;

	// Clears the selection state.
	void reset() noexcept;

	// Adds the "Selection" menu.
	void addMenu(const FileEdit& fileEdit);

  private:
	enum class State : std::uint8_t {
		EMPTY,
		STALE,
		FRESH
	};

	// The selected codepoint.
	tref::Codepoint _cp;
	// The type of selection (or lack thereof).
	State _state{State::EMPTY};

	// Selects the next glyph.
	void selectNext(const FileEdit& fileEdit);

	// Selects the previous glyph.
	void selectPrevious(const FileEdit& fileEdit);
};