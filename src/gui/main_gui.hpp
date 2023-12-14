#include "emulator/emulator.h"

namespace gui {

class MainGui {
   public:
    MainGui();
    bool update();

   private:
    emulator::EmulatorProcess emulator_process;
    static constexpr int ELF_PATH_BUF_SIZE = 256;
    char elf_path_buf[ELF_PATH_BUF_SIZE]   = {0};
};

}  // namespace gui