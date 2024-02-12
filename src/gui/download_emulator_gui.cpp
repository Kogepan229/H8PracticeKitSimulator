#include "download_emulator_gui.h"

#include <network/download_file.h>

#include <filesystem>
#include <format>
#include <future>

#include "elzip.hpp"
#include "emulator/emulator.h"
#include "imgui.h"
#include "log.h"
#include "utils/future.hpp"

#if !defined(_WIN32) && !defined(_WIN64)
#include <sys/stat.h>
#endif

namespace gui {

std::string unzip(const elz::path &archive, const elz::path &target, const std::string &password) {
    try {
        elz::extractZip(archive, target, password);
    } catch (const elz::zip_exception &e) {
        return e.what();
    }
    return "";
}

DownloadEmulatorGui::DownloadEmulatorGui() {
    if (emulator::exist_emulator() && emulator::check_version()) {
        this->is_update = true;
    } else {
        this->is_update = false;
    }

    if (emulator::exist_emulator()) {
        klog::info("Emulator Version: " + emulator::get_version());
    } else {
        klog::info("Emulator is not found.");
    }

    this->window_name            = is_update ? "Update Emulator" : "Download Emulator";
    this->content_length         = 0;
    this->received_length        = 0;
    this->status                 = DownloadEmulatorStatus::PREPARE;
    this->result_ft_check_latest = std::async(emulator::get_latest_info);
}

DownloadEmulatorGui::~DownloadEmulatorGui() {
}

void DownloadEmulatorGui::update() {
    if (status != DownloadEmulatorStatus::PREPARE) {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(550, 150), ImGuiCond_Appearing);

        if (!ImGui::BeginPopupModal(window_name.c_str(), NULL, ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::OpenPopup(window_name.c_str());
            return;
        }
    }

    switch (status) {
        case gui::DownloadEmulatorStatus::PREPARE:
            if (utils::future_is_ready(result_ft_check_latest)) {
                auto result = result_ft_check_latest.get();
                if (!result.error.empty()) {
                    status        = DownloadEmulatorStatus::ERROR;
                    error_message = result.error;
                    break;
                }

                if (result.version != emulator::get_version()) {
                    if (is_update) {
                        klog::info(
                            std::format("Update Emulator to {} (Current {})", result.version, emulator::get_version())
                        );
                    }

                    this->result_ft_download = std::async(
                        network::download_file, result.url, "./tmp/download/", &content_length, &received_length, true
                    );
                    status = DownloadEmulatorStatus::DOWNLOAD;
                    klog::debug("Start download Emulator.");
                } else {
                    klog::info(std::format("The Emulator is currently the latest version ({}).", result.version));
                    deleted = true;
                }
            }
            return;
        case DownloadEmulatorStatus::DOWNLOAD:
            ImGui::TextUnformatted("Downloading ...");
            if (utils::future_is_ready(result_ft_download)) {
                auto result = result_ft_download.get();
                if (!result.error.empty()) {
                    status        = DownloadEmulatorStatus::ERROR;
                    error_message = result.error;
                }

                status        = DownloadEmulatorStatus::UNZIP;
                zip_file_path = result.file_path;

                // Create directory
                try {
                    std::filesystem::create_directories("./emulator");
                } catch (const std::filesystem::filesystem_error &e) {
                    klog::error(e.what());
                    error_message = e.what();
                    status        = DownloadEmulatorStatus::ERROR;
                }

                // Start unzip
                result_ft_unzip = std::async(unzip, result.file_path, "./emulator", "");
            }
            break;

        case DownloadEmulatorStatus::UNZIP:
            ImGui::TextUnformatted("Unzipping files ...");

            if (utils::future_is_ready(result_ft_unzip)) {
                std::string result = result_ft_unzip.get();
                if (result.empty()) {
#if !defined(_WIN32) && !defined(_WIN64)
                    if (chmod(std::string(emulator::EMULATOR_PATH).c_str(), 0755)) {
                        status        = DownloadEmulatorStatus::ERROR;
                        error_message = "Failed to change Emulator permission.";
                        break;
                    }
#endif
                    status = DownloadEmulatorStatus::FINISHED;
                } else {
                    status        = DownloadEmulatorStatus::ERROR;
                    error_message = result;
                }
                if (std::filesystem::exists(zip_file_path)) {
                    std::filesystem::remove(zip_file_path);
                }
            }
            break;

        case DownloadEmulatorStatus::FINISHED:
            ImGui::TextUnformatted(is_update ? "Update finished!" : "Download finished!");
            break;

        case DownloadEmulatorStatus::ERROR:
            ImGui::TextUnformatted(("Error: " + error_message).c_str());
            break;

        default:
            break;
    }

    // ImGui::Text("%d/%d", received_length, content_length);
    if (content_length > 0) {
        ImGui::ProgressBar(
            (float)received_length / (float)content_length, ImVec2(-1, 0.0f),
            std::format("{}/{}", received_length, content_length).c_str()
        );
    }

    if (status == DownloadEmulatorStatus::FINISHED || status == DownloadEmulatorStatus::ERROR) {
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
            deleted = true;
        }
    }

    ImGui::EndPopup();
}

}  // namespace gui