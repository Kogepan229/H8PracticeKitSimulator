#pragma once

#include <string>

namespace klog {

void info(std::string message);
void warn(std::string message);
void error(std::string message);
void debug(std::string message);

}  // namespace klog