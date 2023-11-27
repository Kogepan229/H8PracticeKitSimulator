#pragma once

#include <string>
#include <utility>
#include <vector>

namespace utils {

std::pair<std::string, std::string> split_str_on_first(const std::string &src, char delim);

std::vector<std::string> split_str(std::string str, std::string separator);

}