#pragma once

#include <string>

namespace network {

struct HttpGetResult {
    // Results
    std::string head;
    std::string body;
    // Empty if there are no errors
    std::string error;
};

HttpGetResult http_get(const std::string url);

}  // namespace network