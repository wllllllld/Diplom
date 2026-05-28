#include "strutils.hpp"
#include <algorithm>

namespace strutils {
    std::string to_upper(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c){ return std::toupper(c); });
        return s;
    }
    bool starts_with(const std::string& s, const std::string& prefix) {
        return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
    }
}
