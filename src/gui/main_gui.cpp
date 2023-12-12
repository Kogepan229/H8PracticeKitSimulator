#include "main_gui.hpp"

#include "imgui.h"
#include "lang.h"

namespace gui {

bool MainGui::update() {
    auto view_port = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(view_port->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(view_port->Size, ImGuiCond_Always);
    ImGui::SetNextWindowViewport(view_port->ID);
    ImGui::Begin(
        "Main Window", nullptr,
        ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings
    );

    if (ImGui::BeginMenuBar()) {
        ImGui::MenuItem(lang::translate(lang::LangKeys::MAIN_GUI_SELECT_ELF).c_str());
        ImGui::EndMenuBar();
    }

    ImGui::InputText("Elf Path", elf_path_buf, ELF_PATH_BUF_SIZE);
    if (ImGui::Button("Start")) {
    }

    ImGui::End();
    return false;
}

}  // namespace gui
