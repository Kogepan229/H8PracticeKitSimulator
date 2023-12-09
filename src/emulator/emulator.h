#pragma once

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

};  // namespace emulator