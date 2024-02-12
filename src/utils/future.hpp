#pragma once

#include <future>

namespace utils {

template <typename T>
bool future_is_ready(std::future<T> &ft) {
    if (ft.valid()) {
        using namespace std::chrono_literals;
        auto status = ft.wait_for(0ms);
        return status == std::future_status::ready;
    }
    return false;
}

}  // namespace utils