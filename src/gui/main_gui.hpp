#include <chrono>
#include <string>
#include <vector>

#include "emulator/emulator.h"

namespace gui {

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
};

}  // namespace gui