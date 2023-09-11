#include "log.h"

#include <iostream>
#include <string>

namespace log {

void info(std::string message) {
    std::cout << "[info] " + message << std::endl;
}
void warn(std::string message) {
    std::cout << "[warn] " + message << std::endl;
}
void error(std::string message) {
    std::cout << "[error] " + message << std::endl;
}
void debug(std::string message) {
#ifndef NDEBUG
    std::cout << "[debug] " + message << std::endl;
#endif
}

}  // namespace log