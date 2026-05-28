#include "lib_a.hpp"

namespace lib_a {
    std::string greet(const std::string& name) {
        return "Hello, " + name + "!";
    }
    int version() { return 1; }
}
