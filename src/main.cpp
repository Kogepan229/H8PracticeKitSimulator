#include <memory>
#include <string>
#include <vector>

#include "emulator/emulator.h"
#include "font.h"
#include "graphics.h"
#include "gui/download_emulator_gui.h"
#include "imgui.h"
#include "lang.h"
#include "log.h"
#include "project_version.h"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and
// compatibility with old VS compilers. To link with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project should not be affected, as you are
// likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// Main code
int main(int, char**) {
    graphics::init::init_window("H8 Practice Kit Simulator " + std::string(PROJECT_VERSION));
    graphics::init::init_imgui();

    // Load translation
    lang::load_translation("ja_jp");

    // Load Fonts
    init::load_fonts();

    // Our state
    bool show_demo_window = true;
    ImVec4 clear_color    = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::vector<std::shared_ptr<gui::AsyncGui>> async_gui_list = std::vector<std::shared_ptr<gui::AsyncGui>>();

    if (emulator::exist_emulator() && emulator::check_version()) {
        log::debug("Emulator Version: " + emulator::get_version());
    } else {
        async_gui_list.push_back(std::make_shared<gui::DownloadEmulatorGui>(
            "Download Emulator",
            "https://github.com/Kogepan229/Koge29_H8-3069F_Emulator/releases/latest/download/"
            "h8-3069f_emulator-x86_64-pc-windows-msvc-0.1.2.zip",
            "./tmp/download/"
        ));
    }

    // Main loop
    while (!graphics::window_should_close()) {
        graphics::new_frame();

        // Process async gui
        for (auto it = async_gui_list.begin(); it != async_gui_list.end();) {
            it->get()->update();

            if (it->get()->deleted) {
                it = async_gui_list.erase(it);
            } else {
                it++;
            }
        }

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code
        // to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        graphics::render(clear_color);
    }

    graphics::cleanup();

    return 0;
}
