#include "main_gui.hpp"

#include <format>
#include <vector>

#include "emulator/emulator.h"
#include "imgui.h"
#include "lang.h"
#include "log.h"
#include "utils/time.hpp"

namespace gui {

MainGui::MainGui()
    : emulator_process(emulator::EmulatorProcess())
    , max_received_data_size(200)
    , received_messages(std::vector<std::string>())
    , one_sec_timer(std::chrono::system_clock::now())
    , one_sec_duration(0) {
}

bool MainGui::update() {
    auto view_port = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(view_port->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
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
        emulator_process.init();
        emulator_process.start(std::string(elf_path_buf));
    }

    ImGui::Separator();

    ImGui::InputText("Input Send", send_string_buf, SEND_STRING_BUF_SIZE);
    ImGui::BeginDisabled(!emulator_process.is_running());
    if (ImGui::Button("Send")) {
        emulator_process.send(std::string(send_string_buf));
    }
    ImGui::EndDisabled();

    {
        auto received = emulator_process.get_received_data();

        for (auto it = received.begin(); it != received.end(); ++it) {
            if (received_messages.size() >= max_received_data_size) {
                received_messages.erase(received_messages.begin());
            }
            received_messages.push_back(std::format("[{}] {}", utils::get_current(), *it));

            if (*it == "1sec") {
                auto now_t       = std::chrono::system_clock::now();
                auto dur         = now_t - one_sec_timer;
                one_sec_duration = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
                one_sec_timer    = now_t;
            }
        }
    }

    ImGui::Text("1sec duration: %llu", one_sec_duration);
    ImGui::Text("%llu", received_messages.size());
    ImGui::BeginListBox("Received", ImVec2(0, 400));
    for (auto it = received_messages.rbegin(); it != received_messages.rend(); ++it) {
        ImGui::TextUnformatted(it->c_str());
    }

    ImGui::EndListBox();

    ImGui::End();
    return false;
}

}  // namespace gui
