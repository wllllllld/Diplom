#pragma once
#include <string>
#include <iostream>

namespace anaport {

extern bool g_verbose;

inline void log_info(const std::string& msg) {
    std::cout << "[INFO] " << msg << "\n";
}

inline void log_verbose(const std::string& msg) {
    if (g_verbose) {
        std::cout << "[VERB] " << msg << "\n";
    }
}

inline void log_warn(const std::string& msg) {
    std::cerr << "[WARN] " << msg << "\n";
}

inline void log_error(const std::string& msg) {
    std::cerr << "[ERR]  " << msg << "\n";
}

} // namespace anaport
