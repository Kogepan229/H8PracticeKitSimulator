#pragma once

#include <string>

namespace network {
struct DownloadFileResult {
    // Downloaded file path
    std::string file_path;
    // Empty if there are no errors
    std::string error;
};

DownloadFileResult download_file(
    const std::string url, const std::string desc_dir_path, int *const content_length, int *const received_length
);

}  // namespace network
