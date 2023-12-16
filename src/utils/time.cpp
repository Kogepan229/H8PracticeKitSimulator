#include "time.hpp"

#include <chrono>
#include <format>
#include <string>

namespace utils {

std::string get_current() {
    auto tp      = std::chrono::system_clock::now();
    auto tp_msec = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    int msec     = tp_msec.count() % 1000;
    time_t time  = std::chrono::system_clock::to_time_t(tp);
    struct tm* t = localtime(&time);
    return std::format("{}:{}:{} {}", t->tm_hour, t->tm_min, t->tm_sec, msec);
}

}  // namespace utils