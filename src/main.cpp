#include <memory>
#include <string>
#include <vector>

#include "entity.hpp"
#include "font.h"
#include "graphics.h"
#include "gui/download_emulator_gui.h"
#include "gui/main_gui.hpp"
#include "imgui.h"
#include "lang.h"
#include "nfd.hpp"
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

    NFD::Init();

    // Load translation
    lang::load_translation("ja_jp");

    // Load Fonts
    init::load_fonts();

    // Our state
    bool show_demo_window = true;
    ImVec4 clear_color    = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::vector<std::unique_ptr<h8pks::EntityBase>> entities = std::vector<std::unique_ptr<h8pks::EntityBase>>();

    entities.push_back(std::make_unique<gui::DownloadEmulatorGui>());

    gui::MainGui main_gui;

    // Main loop
    while (!graphics::window_should_close()) {
        graphics::new_frame();

        main_gui.update();

        // Process entities
        for (auto it = entities.begin(); it != entities.end();) {
            it->get()->update();

            if (it->get()->deleted) {
                it = entities.erase(it);
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

    NFD::Quit();

    graphics::cleanup();

    return 0;
}
