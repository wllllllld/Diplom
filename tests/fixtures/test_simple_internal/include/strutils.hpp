#pragma once
#include <string>

namespace strutils {
    std::string to_upper(std::string s);
    bool starts_with(const std::string& s, const std::string& prefix);
}
