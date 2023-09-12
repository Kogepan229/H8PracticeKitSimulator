#pragma once

#include <string>

#include "async_gui.h"

namespace gui {

class DownloadFileGui : public gui::AsyncGui {
    void update() override;

   public:
    DownloadFileGui(std::string url, std::string desc_dir_path);
    virtual ~DownloadFileGui();
    int content_length;
    int received_length;
};

}  // namespace gui