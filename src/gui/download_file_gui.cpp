#include "download_file_gui.h"

#include <download_file.h>

#include "gui/async_gui.h"
#include "gui/download_file_gui.h"
#include "imgui.h"

namespace gui {

DownloadFileGui::DownloadFileGui(std::string url, std::string desc_dir_path) {
    content_length  = 0;
    received_length = 0;
    network::download_file(url, desc_dir_path, &content_length, &received_length);
}

DownloadFileGui::~DownloadFileGui() {
}

void DownloadFileGui::update() {
    ImGui::Begin("Download file");
    ImGui::Text("%d/%d", received_length, content_length);
    ImGui::End();
}

}  // namespace gui