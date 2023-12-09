#pragma once

#include <future>
#include <string>

#include "emulator/emulator.h"
#include "entity.hpp"
#include "network/download_file.h"
#include "network/http_get.hpp"

namespace gui {

enum class DownloadEmulatorStatus {
    PREPARE,
    DOWNLOAD,
    UNZIP,
    FINISHED,
    ERROR,
};

class DownloadEmulatorGui : public h8kps::EntityBase {
   public:
    DownloadEmulatorGui();
    virtual ~DownloadEmulatorGui();
    void update() override;

   private:
    bool is_update;
    std::string window_name;
    DownloadEmulatorStatus status;
    int content_length;
    int received_length;
    std::string error_message;
    std::string zip_file_path;
    std::future<emulator::LatestEmulatorInfo> result_ft_check_latest;
    std::future<network::DownloadFileResult> result_ft_download;
    std::future<std::string> result_ft_unzip;
};

}  // namespace gui