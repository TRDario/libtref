#include "../include/common.hpp"
#include "../include/help.hpp"
#include <imgui.h>

void Help::addMenu()
{
	if (ImGui::BeginMenu("Help")) {
		if (ImGui::MenuItem("About")) {
			_aboutWindowActive = true;
		}
		ImGui::EndMenu();
	}
}

void Help::addAboutWindow()
{
	if (!_aboutWindowActive) {
		return;
	}

	if (ImGui::Begin("About", &_aboutWindowActive, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
		ImGui::Text("gtref - Graphical .tref Editor");
		ImGui::Text("v0.0.1" GTREF_VERSION_SUFFIX GTREF_PLATFORM);
		ImGui::Text("Made with Dear ImGui");
		ImGui::Separator();
		ImGui::Text("(C) 2025 TRDario (Dario Cvitanović)");
		ImGui::Text("https://github.com/TRDario/gtref");
		ImGui::Text("Licensed under the MIT License");
	}
	ImGui::End();
}