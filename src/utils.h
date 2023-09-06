#pragma once

#include <string>
#include <utility>

namespace utils {

std::pair<std::string, std::string> split_str_on_first(const std::string &src, char delim);

}