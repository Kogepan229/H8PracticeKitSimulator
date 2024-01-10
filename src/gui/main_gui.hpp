#include <chrono>
#include <string>
#include <vector>

#include "emulator/emulator.h"
#include "file_browser.hpp"

namespace gui {

constexpr nfdfilteritem_t ELF_FILE_FILTER[1] = {{"ELF", "elf"}};

class MainGui {
   public:
    MainGui();
    bool update();

   private:
    emulator::EmulatorProcess emulator_process;
    static constexpr int ELF_PATH_BUF_SIZE     = 256;
    static constexpr int SEND_STRING_BUF_SIZE  = 256;
    char elf_path_buf[ELF_PATH_BUF_SIZE]       = {0};
    char send_string_buf[SEND_STRING_BUF_SIZE] = {0};
    size_t max_received_data_size;
    std::vector<std::string> received_messages;
    std::chrono::time_point<std::chrono::system_clock> one_sec_timer;
    long long one_sec_duration;
    filebrowser::FileBrowser elf_browser;
};

}  // namespace gui