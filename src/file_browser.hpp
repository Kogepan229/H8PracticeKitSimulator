#include <future>
#include <string>

#include "nfd.hpp"

namespace filebrowser {

class FileBrowser {
   public:
    FileBrowser();
    ~FileBrowser();
    void open(const nfdu8filteritem_t *filter_items, size_t filter_item_count, std::string default_path = "");
    bool check_finished();
    std::string get_file_path();

   private:
    NFD::UniquePath out_path;
    std::string result_path;
    std::future<nfdresult_t> filebrowser_ft;
    nfdresult_t open_dialog(const nfdu8filteritem_t *filter_items, size_t filter_item_count, std::string default_path);
};

}  // namespace filebrowser