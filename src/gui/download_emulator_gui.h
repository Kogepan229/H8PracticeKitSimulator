#pragma once

#include <future>
#include <string>

#include "async_gui.h"
#include "download_file.h"

namespace gui {

enum class DownloadEmulatorStatus {
    DOWNLOAD,
    UNZIP,
    FINISHED,
    ERROR,
};

class DownloadEmulatorGui : public gui::AsyncGui {
    std::string window_name;
    DownloadEmulatorStatus status;
    std::string error_message;
    std::string zip_file_path;
    std::future<network::DownloadFileResult> result_ft_download;
    std::future<std::string> result_ft_unzip;
    void update() override;

   public:
    DownloadEmulatorGui(std::string window_name, std::string url, std::string desc_dir_path);
    virtual ~DownloadEmulatorGui();
    int content_length;
    int received_length;
};

}  // namespace gui