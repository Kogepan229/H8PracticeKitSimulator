#include "string.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "mongoose.h"


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

std::vector<std::string> split_str(std::string str, std::string separator) {
    std::vector<std::string> out;

    int len = separator.length();
    if (len == 0) {
        out.push_back(str);
    } else {
        int offset = 0;
        while (true) {
            int pos = str.find(separator, offset);
            if (pos < 0) {
                out.push_back(str.substr(offset));
                break;
            }
            out.push_back(str.substr(offset, pos - offset));
            offset = pos + len;
        }
    }
    return out;
}

std::string conv_mg_str(mg_str str) {
    auto s = std::make_unique<char[]>(str.len + 1);
    memcpy(s.get(), str.ptr, str.len);
    s[str.len] = '\0';
    return std::string(s.get());
}

}  // namespace utils