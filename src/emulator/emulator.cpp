#include "emulator.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <future>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "log.h"
#include "mongoose.h"
#include "network/http_get.hpp"
#include "utils/string.hpp"

#define BUFSIZE 256

namespace emulator {

std::string emulator_version = "";

bool exist_emulator() {
    return std::filesystem::is_regular_file(emulator::EMULATOR_PATH);
}

std::string get_version() {
    return emulator_version;
}

bool check_version() {
    if (!exist_emulator()) {
        klog::error("Could not check emulator version. Could not found emulator.");
        return false;
    }

    FILE *fp;
    std::string cmdline = std::string(emulator::EMULATOR_PATH) + " --version";
    if ((fp = popen(cmdline.c_str(), "r")) == NULL) {
        klog::error("Could not check emulator version. Failed exec.");
        return false;
    }
    char buf[BUFSIZE];

    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
    }

    (void)pclose(fp);

    auto strs = utils::split_str(std::string(buf), " ");
    if (strs.size() != 2) {
        klog::error("Could not check emulator version. Invalid version.");
        return false;
    }

    emulator_version = strs[1];
    emulator_version.replace(emulator_version.size() - 1, 1, "");
    return true;
}

LatestEmulatorInfo get_latest_info() {
    klog::debug("Get latest emulator info.");
    auto info = LatestEmulatorInfo();

    auto result = network::http_get("https://api.github.com/repos/Kogepan229/Koge29_H8-3069F_Emulator/releases/latest");
    if (!result.error.empty()) {
        info.error = result.error;
        return info;
    }

    nlohmann::json data = nlohmann::json::parse(result.body);

    if (!data["tag_name"].empty()) {
        std::string tag = data["tag_name"].get<std::string>();
        info.version    = tag.replace(0, 1, "");  // replace "v" -> ""
    } else {
        info.error = "Could not found tag_name.";
        return info;
    }

#if defined(_WIN32) || defined(_WIN64)
    constexpr std::string_view TARGET = "windows";
#else
    constexpr std::string_view TARGET = "linux";
#endif

    for (size_t i = 0; i < data["assets"].size(); i++) {
        if (!data["assets"][i]["name"].empty()) {
            std::string name = data["assets"][i]["name"].get<std::string>();
            if (name.find(TARGET) != std::string::npos) {
                if (!data["assets"][i]["browser_download_url"].empty()) {
                    info.url = data["assets"][i]["browser_download_url"].get<std::string>();
                    break;
                } else {
                    info.error = "Could not download_url. But file name is found.";
                    return info;
                }
            }
        }
    }

    if (info.url.empty()) {
        info.error = "Could not download_url.";
        return info;
    }

    return info;
}

EmulatorProcess::EmulatorProcess()
    : max_received_data_size(2000)
    , received_data(std::make_shared<utils::MutexGuard<std::vector<std::string>>>())
    , send_data_queue(std::queue<std::string>()) {
}

EmulatorProcess::~EmulatorProcess() {
}

void EmulatorProcess::init() {
    if (this->result_ft_exec.valid() && this->result_ft_communicate.valid()) {
        if (!this->is_running()) {
            this->result_ft_exec        = std::future<bool>();
            this->result_ft_communicate = std::future<std::string>();
        }
    }
}

// Return true when the emulator is already running
bool EmulatorProcess::start(std::string elf_path) {
    if (!this->result_ft_exec.valid() && !result_ft_communicate.valid()) {
        klog::debug("Start Emulator.");
        this->result_ft_exec        = std::async([&](std::string elf_path) { return this->exec(elf_path); }, elf_path);
        this->result_ft_communicate = std::async([&] { return this->communicate(); });
        return false;
    } else {
        klog::debug("Emulator is running.");
        return true;
    }
}

bool EmulatorProcess::is_running() {
    if (this->result_ft_exec.valid() && this->result_ft_communicate.valid()) {
        using namespace std::chrono_literals;
        auto check_exec_status        = this->result_ft_exec.wait_for(0ms);
        auto check_communicate_status = this->result_ft_communicate.wait_for(0ms);
        if (check_exec_status == std::future_status::ready && check_communicate_status == std::future_status::ready) {
            return false;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

bool EmulatorProcess::exec(std::string elf_path) {
    klog::info("Execute Emulator.");
    int status = system(std::format("{} --elf={}", EMULATOR_PATH, elf_path).c_str());
    if (status != 0) {
        klog::error(std::format("Failed execute emulator. status: {}", status));
        return true;
    } else {
        klog::info("Stopped Emulator.");
    }

    return false;
}

struct CommunicateCallbackData {
    bool closed                         = false;
    const size_t max_received_data_size = 0;
    std::shared_ptr<utils::MutexGuard<std::vector<std::string>>> received_data;
    utils::MutexGuard<std::queue<std::string>> *send_data_queue;
    std::string error = "";
};

void communicate_callback(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    CommunicateCallbackData *cb_data = static_cast<CommunicateCallbackData *>(fn_data);
    if (ev == MG_EV_READ) {
        auto received_str = utils::conv_mg_str(mg_str_n((char *)c->recv.buf, c->recv.len));

        // Separate with \n
        auto strs = utils::split_str(received_str, "\n");
        for (auto it = strs.begin(); it != strs.end(); ++it) {
            if (it->empty()) {
                continue;
            }

            {
                auto d = cb_data->received_data->auto_lock();
                if (d->size() >= cb_data->max_received_data_size) {
                    d->erase(d->begin());
                }
                d->push_back(*it);
            }
        }
    } else if (ev == MG_EV_ERROR) {
        cb_data->error  = std::string((char *)ev_data);
        cb_data->closed = true;
        return;
    } else if (ev == MG_EV_CLOSE) {
        cb_data->closed = true;
        return;
    }

    {
        auto q = cb_data->send_data_queue->auto_lock();
        while (!q->empty()) {
            auto str = q->front().c_str();
            auto s   = std::make_unique<char[]>(q->front().size() + 1);
            memcpy(s.get(), str, q->front().size());
            s[q->front().size()] = '\n';

            mg_send(c, s.get(), q->front().size() + 1);
            q->pop();
        }
    }
}

std::string EmulatorProcess::communicate() {
    CommunicateCallbackData callback_data =
        CommunicateCallbackData(false, max_received_data_size, received_data, &send_data_queue);
    klog::info("Start communication with Emulator");

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_connect(&mgr, "tcp://127.0.0.1:12345", communicate_callback, &callback_data);
    while (!callback_data.closed) {
        mg_mgr_poll(&mgr, 10);
    }
    mg_mgr_free(&mgr);

    klog::info("Disconnected from Emulator.");
    return callback_data.error;
}

void EmulatorProcess::send(std::string data) {
    auto q = send_data_queue.auto_lock();
    q->push(data);
}

std::shared_ptr<utils::MutexGuard<std::vector<std::string>>> EmulatorProcess::get_received_data() {
    return received_data;
}

};  // namespace emulator