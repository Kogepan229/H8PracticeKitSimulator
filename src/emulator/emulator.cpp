#include "emulator.h"

#include <cstdio>
#include <filesystem>
#include <string>

#include "log.h"
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

};  // namespace emulator