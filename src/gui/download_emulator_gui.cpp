#include "download_emulator_gui.h"

#include <download_file.h>

#include <chrono>
#include <filesystem>
#include <future>
#include <thread>

#include "async_gui.h"
#include "elzip.hpp"
#include "imgui.h"
#include "log.h"

namespace gui {

std::string unzip(const elz::path &archive, const elz::path &target, const std::string &password) {
    try {
        elz::extractZip(archive, target, password);
    } catch (elz::zip_exception e) {
        return e.what();
    }
    return "";
}

DownloadEmulatorGui::DownloadEmulatorGui(std::string window_name, std::string url, std::string desc_dir_path) {
    this->window_name     = window_name;
    this->content_length  = 0;
    this->received_length = 0;
    this->status          = DownloadEmulatorStatus::DOWNLOAD;
    this->result_ft_download =
        std::async(network::download_file, url, desc_dir_path, &content_length, &received_length);
}

DownloadEmulatorGui::~DownloadEmulatorGui() {
}

void DownloadEmulatorGui::update() {
    ImGui::Begin(window_name.c_str(), (bool *)0, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse);

    switch (status) {
        case DownloadEmulatorStatus::DOWNLOAD:
            ImGui::TextUnformatted("Downloading ...");
            if (result_ft_download.valid()) {
                using namespace std::chrono_literals;
                auto download_status = result_ft_download.wait_for(0ms);
                if (download_status == std::future_status::ready) {
                    auto result = result_ft_download.get();
                    if (result.error.empty()) {
                        status        = DownloadEmulatorStatus::UNZIP;
                        zip_file_path = result.file_path;

                        // Create directory
                        try {
                            std::filesystem::create_directories(std::string("./emulator"));
                        } catch (std::filesystem::filesystem_error e) {
                            log::error(e.what());
                            error_message = e.what();
                            status        = DownloadEmulatorStatus::ERROR;
                        }

                        // Start unzip
                        result_ft_unzip = std::async(unzip, result.file_path, std::string("./emulator"), "");
                    } else {
                        status        = DownloadEmulatorStatus::ERROR;
                        error_message = result.error;
                    }
                }
            }
            break;

        case DownloadEmulatorStatus::UNZIP:
            ImGui::TextUnformatted("Unzipping files ...");

            if (result_ft_unzip.valid()) {
                using namespace std::chrono_literals;
                auto unzip_status = result_ft_unzip.wait_for(0ms);
                if (unzip_status == std::future_status::ready) {
                    std::string result = result_ft_unzip.get();
                    if (result.empty()) {
                        status = DownloadEmulatorStatus::FINISHED;
                        std::filesystem::remove(zip_file_path);
                    } else {
                        status        = DownloadEmulatorStatus::ERROR;
                        error_message = result;
                    }
                }
            }
            break;

        case DownloadEmulatorStatus::FINISHED:
            ImGui::TextUnformatted("Download finished!");
            break;

        case DownloadEmulatorStatus::ERROR:
            ImGui::TextUnformatted(("Error: " + error_message).c_str());
            break;

        default:
            break;
    }

    ImGui::Text("%d/%d", received_length, content_length);

    if (status == DownloadEmulatorStatus::FINISHED || status == DownloadEmulatorStatus::ERROR) {
        if (ImGui::Button("Close")) {
            deleted = true;
        }
    }

    ImGui::End();
}

}  // namespace gui