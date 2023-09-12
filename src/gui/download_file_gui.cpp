#include "download_file_gui.h"

#include <download_file.h>

#include <chrono>
#include <future>
#include <thread>

#include "gui/async_gui.h"
#include "gui/download_file_gui.h"
#include "imgui.h"

namespace gui {

DownloadFileGui::DownloadFileGui(std::string window_name, std::string url, std::string desc_dir_path) {
    this->window_name     = window_name;
    this->content_length  = 0;
    this->received_length = 0;
    this->result_ft       = std::async(network::download_file, url, desc_dir_path, &content_length, &received_length);
}

DownloadFileGui::~DownloadFileGui() {
}

void DownloadFileGui::update() {
    ImGui::Begin(window_name.c_str());

    if (result_ft.valid()) {
        using namespace std::chrono_literals;
        auto status = result_ft.wait_for(0ms);
        if (status == std::future_status::ready) {
            finished = true;
            result   = result_ft.get();
        }
    }

    if (finished) {
        if (result.error.empty()) {
            ImGui::TextUnformatted("Download finished!");
        } else {
            ImGui::TextUnformatted(result.error.c_str());
        }
    } else {
        ImGui::TextUnformatted("Downloading ...");
    }
    ImGui::Text("%d/%d", received_length, content_length);
    ImGui::End();
}

}  // namespace gui