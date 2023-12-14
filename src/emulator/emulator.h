#pragma once

#include <cstddef>
#include <future>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "utils/mutex_guard.hpp"

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
    EmulatorProcess();
    ~EmulatorProcess();
    void init();
    bool start(std::string elf_path);
    void send(std::string data);
    std::shared_ptr<utils::MutexGuard<std::vector<std::string>>> get_received_data();

   private:
    size_t max_received_data_size;
    std::shared_ptr<utils::MutexGuard<std::vector<std::string>>> received_data;
    utils::MutexGuard<std::queue<std::string>> send_data_queue;

    std::future<bool> result_ft_exec;
    std::future<std::string> result_ft_communicate;
    bool exec(std::string elf_path);
    std::string communicate();
};

};  // namespace emulator