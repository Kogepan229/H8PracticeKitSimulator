#include "emulator.h"

#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <format>
#include <nlohmann/json.hpp>
#include <string>

#include "log.h"
#include "network/http_get.hpp"
#include "utils.h"

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
        log::error("Could not check emulator version. Could not found emulator.");
        return false;
    }

    FILE *fp;
    std::string cmdline = std::string(emulator::EMULATOR_PATH) + " --version";
    if ((fp = popen(cmdline.c_str(), "r")) == NULL) {
        log::error("Could not check emulator version. Failed exec.");
        return false;
    }
    char buf[BUFSIZE];

    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
    }

    (void)pclose(fp);

    auto strs = utils::split_str(std::string(buf), " ");
    if (strs.size() != 2) {
        log::error("Could not check emulator version. Invalid version.");
        return false;
    }

    emulator_version = strs[1];
    return true;
}

LatestEmulatorInfo get_latest_info() {
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

};  // namespace emulator