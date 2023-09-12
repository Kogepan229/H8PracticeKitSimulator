// Dear ImGui: standalone example application for Glfw + Vulkan
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// Important note to the reader who wish to integrate imgui_impl_vulkan.cpp/.h in their own engine/app.
// - Common ImGui_ImplVulkan_XXX functions and structures are used to interface with imgui_impl_vulkan.cpp/.h.
//   You will use those if you want to use this rendering backend in your engine/app.
// - Helper ImGui_ImplVulkanH_XXX functions and structures are only used by this example (main.cpp) and by
//   the backend itself (imgui_impl_vulkan.cpp), but should PROBABLY NOT be used by your own engine/app code.
// Read comments in imgui_impl_vulkan.h.

#include <stdio.h>   // printf, fprintf
#include <stdlib.h>  // abort

#include <memory>
#include <string>
#include <vector>

#include "download_file.h"
#include "font.h"
#include "graphics.h"
#include "gui/download_file_gui.h"
#include "imgui.h"
#include "lang.h"
#include "project_version.h"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and
// compatibility with old VS compilers. To link with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project should not be affected, as you are
// likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// #define IMGUI_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif

// Main code
int main(int, char**) {
    graphics::init::init_window("H8 Practice Kit Simulator " + std::string(PROJECT_VERSION));
    graphics::init::init_imgui();

    // Load translation
    lang::load_translation("ja_jp");

    // Load Fonts
    init::load_fonts();

    // Upload Fonts
    graphics::init::upload_fonts();

    ImGuiIO& io = ImGui::GetIO();

    // Our state
    bool show_demo_window    = true;
    bool show_another_window = false;
    ImVec4 clear_color       = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::vector<std::shared_ptr<gui::AsyncGui>> async_gui_list = std::vector<std::shared_ptr<gui::AsyncGui>>();
    async_gui_list.push_back(std::make_shared<gui::DownloadFileGui>(
        "Download Emulator",
        "https://github.com/Kogepan229/Koge29_H8-3069F_Emulator/releases/latest/download/"
        "h8-3069f_emulator-x86_64-pc-windows-msvc-0.1.1.zip",
        "./download/"
    ));

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

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f     = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world ウィンドウだよ!");  // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");  // Display some text (you can use a format strings too)
            ImGui::TextUnformatted(lang::translate(lang::LangKeys::LANGUAGE).c_str());
            ImGui::TextUnformatted(lang::translate(lang::LangKeys::TEST_TEXT).c_str());
            ImGui::Checkbox("Demo Window", &show_demo_window);  // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);             // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color);  // Edit 3 floats representing a color

            if (ImGui::Button("Button"
                ))  // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average(平均) %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window) {
            ImGui::Begin(
                "Another Window",
                &show_another_window
            );  // Pass a pointer to our bool variable (the window will have a closing
                // button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        graphics::render(clear_color);
    }

    graphics::cleanup();

    return 0;
}
