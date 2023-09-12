#pragma once

#include <future>
#include <string>

#include "async_gui.h"
#include "download_file.h"

namespace gui {

class DownloadFileGui : public gui::AsyncGui {
    bool finished;
    std::future<network::DownloadFileResult> result_ft;
    network::DownloadFileResult result;
    void update() override;

   public:
    DownloadFileGui(std::string url, std::string desc_dir_path);
    virtual ~DownloadFileGui();
    int content_length;
    int received_length;
};

}  // namespace gui