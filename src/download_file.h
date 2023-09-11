#include <string>

namespace network {
struct DownloadFileResult {
    // Downloaded file path
    std::string file_path;
    // Empty if there are no errors
    std::string error;
};

DownloadFileResult download_file(std::string url, std::string file_dir_path);

}  // namespace network
