#include "utils.h"

#include <string>
#include <utility>

namespace utils {

std::pair<std::string, std::string> split_str_on_first(const std::string &src, char delim) {
    std::string s1 = "";
    std::string s2 = "";
    bool found     = false;
    for (char ch : src) {
        if (ch == delim) {
            found = true;
            continue;
        }
        if (!found) {
            s1 += ch;
        } else {
            s2 += ch;
        }
    }
    return {s1, s2};
}

}  // namespace utils