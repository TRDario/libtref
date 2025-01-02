#include "../include/file_edit.hpp"
#include "../include/help.hpp"
#include "../include/preview.hpp"
#include "../include/selection.hpp"
#include "../include/view.hpp"
#include "../resources/Roboto-Medium.ttf.hpp"
#include <imgui.h>
#include <tinyfiledialogs.h>
#include <tr/imgui.hpp>

tr::Window initialize()
{
	tr::Window window{"gtref", {800, 600}, tr::CENTERED_POS, tr::WindowFlag::RESIZABLE | tr::WindowFlag::MAXIMIZED};
	window.events().sendTextInputEvents(true);

	ImGui::CreateContext();
	tr::ImGui::initialize();
	ImGui::GetIO().IniFilename = nullptr;
	ImFontConfig imguiFontConfig;
	imguiFontConfig.FontDataOwnedByAtlas = false;
	static constexpr ImWchar RANGES[]{0x20,   0xFF,   0x100,  0x180,  0x192,  0x193,  0x1A0,  0x1A2,  0x1AF,
									  0x1B1,  0x1F0,  0x1F1,  0x1FA,  0x200,  0x218,  0x220,  0x237,  0x238,
									  0x384,  0x3D7,  0x400,  0x514,  0x1E00, 0x1E02, 0x1E3E, 0x1E40, 0x1E80,
									  0x1E86, 0x1EA0, 0x1EFA, 0x2000, 0x200C, 0x2013, 0x2016, 0x20AC, 0x20AD,
									  0x221E, 0x221F, 0x2248, 0x2249, 0x2260, 0x2261, 0x2264, 0x2266, 0x0};
	ImGui::GetIO().Fonts->AddFontFromMemoryTTF(ROBOTO_MEDIUM_TTF, ROBOTO_MEDIUM_TTF_len, 24, &imguiFontConfig, RANGES);

	return window;
}

Signal doRedraw(FileEdit& fileEdit, Selection& selection, View& view, Preview& preview, Help& help)
{
	constexpr ImGuiWindowFlags WINDOW_FLAGS{ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
											ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollWithMouse |
											ImGuiWindowFlags_NoBringToFrontOnFocus};

	tr::ImGui::newFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::SetNextWindowPos({0, 0});
	if (ImGui::Begin("Main", nullptr, WINDOW_FLAGS)) {
		if (ImGui::BeginMenuBar()) {
			if (fileEdit.addFileMenu(selection, view) == Signal::EXIT) {
				return Signal::EXIT;
			}
			fileEdit.addEditMenu(selection);
			selection.addMenu(fileEdit);
			view.addMenu();
			preview.addMenu(fileEdit);
			help.addMenu();
			ImGui::EndMenuBar();
		}
		fileEdit.addWorkspace(selection, view);
		fileEdit.addGlyphPropertiesEditor(selection);
		if (!fileEdit.empty()) {
			view.addZoomControl();
		}
		fileEdit.addLineSkipEditor();
		preview.addPreviewWindow(fileEdit);
		help.addAboutWindow();
	}
	ImGui::End();
	ImGui::Render();

	tr::window().graphics().clear(tr::Clear::COLOR);
	tr::ImGui::draw();
	tr::window().graphics().swap();
	return Signal::CONTINUE;
}

int main()
{
	try {
		tr::Window window{initialize()};

		FileEdit  fileEdit;
		Selection selection;
		View      view;
		Preview   preview;
		Help      help;

		tr::Timer drawTimer{tr::createDrawTimer(tr::display().displayMode(tr::DESKTOP_MODE).refreshRate)};
		bool      redraw{true};
		bool      active{true};

		window.show();
		while (active) {
			window.events().handle([&](const tr::Event& event) {
				tr::ImGui::processEvent(event);
				switch (event.type()) {
				case tr::event_type::DRAW:
					redraw = true;
					break;
				case tr::event_type::QUIT:
					active = !fileEdit.close(selection);
					break;
				}
			});
			if (active && redraw) {
				active = doRedraw(fileEdit, selection, view, preview, help) == Signal::CONTINUE;
				redraw = false;
			}
		}
		window.hide();

		tr::ImGui::shutdown();
		return EXIT_SUCCESS;
	}
	catch (std::exception& err) {
		tinyfd_messageBox("Fatal Error - gtref", err.what(), "ok", "error", 0);
		return EXIT_FAILURE;
	}
}