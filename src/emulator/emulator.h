#pragma once

#include <future>
#include <string>

namespace emulator {

#if defined(_WIN32) || defined(_WIN64)
constexpr std::string_view EMULATOR_PATH = ".\\emulator\\koge29_h8-3069f_emulator.exe";
#else
constexpr std::string_view EMULATOR_PATH = "./emulator/koge29_h8-3069f_emulator";
#endif

bool exist_emulator();

std::string get_version();

bool check_version();

struct LatestEmulatorInfo {
    std::string version = "";
    std::string url     = "";
    std::string error   = "";
};

LatestEmulatorInfo get_latest_info();

class EmulatorProcess {
   public:
    bool start(std::string elf_path);

   private:
    std::future<bool> result_ft_process;
    bool exec(std::string elf_path);
};

};  // namespace emulator