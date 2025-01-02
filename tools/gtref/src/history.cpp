#include "../include/common.hpp"
#include "../include/font.hpp"
#include "../include/history.hpp"
#include "../include/selection.hpp"

/// PUBLIC FUNCTIONS ///

bool History::canUndo() const noexcept
{
	return _curEdit != _edits.begin();
}

bool History::canRedo() const noexcept
{
	return _curEdit != _edits.end();
}

bool History::atSavePoint() const noexcept
{
	return _curEdit == _savedEdit;
}

void History::setFilename(const std::string& filename)
{
	_savedWindowTitle  = std::format("{} - gtref", filename);
	_editedWindowTitle = std::format("{} * - gtref", filename);
}

void History::clear() noexcept
{
	_edits.clear();
	_curEdit = _edits.end();
	_savedEdit.reset();
}

void History::setSavePoint() noexcept
{
	_savedEdit = _curEdit;
	setWindowTitle();
}

void History::undo(Selection& selection, Font& font)
{
	// clang-format off
	visit(tr::Overloaded{
		[&](const GlyphAddition& edit) {
			if (selection == edit.cp) {
				selection.reset();
			}
			font.glyphs.erase(edit.cp);
		},
		[&](const GlyphDeletion& edit) {
			font.glyphs.emplace(edit.cp, edit.glyph);
		},
		[&](const GlyphCodepointEdit& edit) {
			font.glyphs.emplace(edit.prev, font.glyphs.at(edit.next));
			font.glyphs.erase(edit.next);
			if (selection == edit.next) {
				selection.setStale(edit.prev);
			}
		},
		[&](const GlyphTextureOffsetEdit& edit) {
			tref::Glyph& glyph{font.glyphs.at(edit.cp)};
			glyph.x = edit.prev.x;
			glyph.y = edit.prev.y;
		},
		[&](const GlyphSizeEdit& edit) {
			tref::Glyph& glyph{font.glyphs.at(edit.cp)};
			glyph.width  = edit.prev.x;
			glyph.height = edit.prev.y;
		},
		[&](const GlyphPlacementEdit& edit) {
			tref::Glyph& glyph{font.glyphs.at(edit.cp)};
			glyph.xOffset = edit.prev.x;
			glyph.yOffset = edit.prev.y;
		},
		[&](const GlyphAdvanceEdit& edit) {
			font.glyphs.at(edit.cp).advance = edit.prev;
		},
		[&](const LineSkipEdit& edit) {
			font.lineSkip = edit.prev;
		}
	}, *--_curEdit);
	setWindowTitle();
	// clang-format on
}

void History::redo(Selection& selection, Font& font)
{
	// clang-format off
	visit(tr::Overloaded{
		[&](const GlyphAddition& edit) {
			font.glyphs.emplace(edit.cp, tref::Glyph{});
		},
		[&](const GlyphDeletion& edit) {
			if (selection == edit.cp) {
				selection.reset();
			}
			font.glyphs.erase(edit.cp);
		},
		[&](const GlyphCodepointEdit& edit) {
			font.glyphs.emplace(edit.next, font.glyphs.at(edit.prev));
			font.glyphs.erase(edit.prev);
			if (selection == edit.prev) {
				selection.setStale(edit.next);
			}
		},
		[&](const GlyphTextureOffsetEdit& edit) {
			tref::Glyph& glyph{font.glyphs.at(edit.cp)};
			glyph.x = edit.next.x;
			glyph.y = edit.next.y;
		},
		[&](const GlyphSizeEdit& edit) {
			tref::Glyph& glyph{font.glyphs.at(edit.cp)};
			glyph.width  = edit.next.x;
			glyph.height = edit.next.y;
		},
		[&](const GlyphPlacementEdit& edit) {
			tref::Glyph& glyph{font.glyphs.at(edit.cp)};
			glyph.xOffset = edit.next.x;
			glyph.yOffset = edit.next.y;
		},
		[&](const GlyphAdvanceEdit& edit) {
			font.glyphs.at(edit.cp).advance = edit.next;
		},
		[&](const LineSkipEdit& edit) {
			font.lineSkip = edit.next;
		}
	}, *_curEdit++);
	setWindowTitle();
	// clang-format on
}

void History::addEdit(const Edit& edit)
{
	if (canMergeIntoExistingEdit(edit)) {
		mergeIntoExistingEdit(edit);
	}
	else {
		pushNewEdit(edit);
	}
	setWindowTitle();
}

/// INTERNAL FUNCTIONS ///

void History::setWindowTitle() const noexcept
{
	if (atSavePoint() && tr::window().title() != _savedWindowTitle) {
		tr::window().setTitle(_savedWindowTitle);
	}
	else if (!atSavePoint() && tr::window().title() != _editedWindowTitle) {
		tr::window().setTitle(_editedWindowTitle);
	}
}

bool History::canMergeIntoExistingEdit(const Edit& edit) const noexcept
{
	// clang-format off
	return _curEdit != _edits.begin() && prev(_curEdit)->index() == edit.index() &&
		visit(tr::Overloaded{
			[](const GlyphAddition&) {
				return false;
			},
			[](const GlyphDeletion&) {
				return false;
			},
			[&](const GlyphCodepointEdit& edit) {
				return get_if<GlyphCodepointEdit>(&_edits.back())->next == edit.prev;
			},
			[&](const GlyphTextureOffsetEdit& edit) {
				return get_if<GlyphTextureOffsetEdit>(&_edits.back())->cp == edit.cp;
			},
			[&](const GlyphSizeEdit& edit) {
				return get_if<GlyphSizeEdit>(&_edits.back())->cp == edit.cp;
			},
			[&](const GlyphPlacementEdit& edit) {
				return get_if<GlyphPlacementEdit>(&_edits.back())->cp == edit.cp;
			},
			[&](const GlyphAdvanceEdit& edit) {
				return get_if<GlyphAdvanceEdit>(&_edits.back())->cp == edit.cp;
			},
			[](const LineSkipEdit&) {
				return true;
			}
		}, edit);
	// clang-format on
}

void History::mergeIntoExistingEdit(const Edit& edit) noexcept
{
	GTREF_ASSERT(_curEdit != _edits.begin());
	GTREF_ASSERT(edit.index() == prev(_curEdit)->index());

	// clang-format off
	visit(tr::Overloaded{
		[&] (const GlyphCodepointEdit& edit)     { get_if<GlyphCodepointEdit>(&*prev(_curEdit))->next = edit.next; },
		[&] (const GlyphTextureOffsetEdit& edit) { get_if<GlyphTextureOffsetEdit>(&*prev(_curEdit))->next = edit.next; },
		[&] (const GlyphSizeEdit& edit)          { get_if<GlyphSizeEdit>(&*prev(_curEdit))->next = edit.next; },
		[&] (const GlyphPlacementEdit& edit)     { get_if<GlyphPlacementEdit>(&*prev(_curEdit))->next = edit.next; },
		[&] (const GlyphAdvanceEdit& edit)       { get_if<GlyphAdvanceEdit>(&*prev(_curEdit))->next = edit.next; },
		[&] (const LineSkipEdit& edit)           { get_if<LineSkipEdit>(&*prev(_curEdit))->next = edit.next; },
		[ ] (const auto&) {}
	}, edit);
	// clang-format on
}

void History::pushNewEdit(const Edit& edit)
{
	if (_curEdit != _edits.end()) {
		if (_savedEdit.has_value()) {
			if (_savedEdit == _curEdit) {
				_savedEdit = _edits.end();
			}
			else if (distance(_edits.begin(), *_savedEdit) > distance(_edits.begin(), _curEdit)) {
				_savedEdit.reset();
			}
		}
		_edits.erase(_curEdit, _edits.end());
		_curEdit = _edits.end();
	}
	_edits.push_back(edit);
	if (_savedEdit == _edits.end()) {
		_savedEdit = std::prev(_edits.end());
	}
}