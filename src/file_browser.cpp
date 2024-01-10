#include "file_browser.hpp"

#include <cstddef>
#include <format>
#include <future>
#include <string>

#include "log.h"
#include "nfd.hpp"

namespace filebrowser {

FileBrowser::FileBrowser() {
}

FileBrowser::~FileBrowser() {
}

nfdresult_t FileBrowser::open_dialog(
    const nfdu8filteritem_t *filter_items, size_t filter_item_count, std::string default_path
) {
    return NFD::OpenDialog(
        out_path, filter_items, filter_item_count, !default_path.empty() ? default_path.c_str() : nullptr
    );
}

void FileBrowser::open(const nfdu8filteritem_t *filter_items, size_t filter_item_count, std::string default_path) {
    filebrowser_ft = std::async(&FileBrowser::open_dialog, this, filter_items, filter_item_count, default_path);
}

bool FileBrowser::check_finished() {
    if (filebrowser_ft.valid()) {
        using namespace std::chrono_literals;
        auto download_status = filebrowser_ft.wait_for(0ms);
        if (download_status == std::future_status::ready) {
            auto result = filebrowser_ft.get();
            if (result == NFD_OKAY) {
                result_path = std::string(out_path.get());
            } else if (result == NFD_CANCEL) {
                result_path = "";
            } else {
                klog::error(std::format("FileBrowser Error: {}", NFD_GetError()));
            }
            return true;
        }
    }

    return false;
}

std::string FileBrowser::get_file_path() {
    return result_path;
}

}  // namespace filebrowser