#pragma once

// Object representing the "Help" menu and its related state.
class Help {
  public:
	// Adds the "About" menu.
	void addMenu();

	// Adds the "About" window.
	void addAboutWindow();

  private:
	// Flag keeping track of the "About" window's state.
	bool _aboutWindowActive{false};
};