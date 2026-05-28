#include "lib_b.hpp"
#include "lib_a.hpp"
#include <algorithm>

namespace lib_b {
    std::vector<std::string> process(const std::vector<std::string>& inputs) {
        std::vector<std::string> results;
        results.reserve(inputs.size());
        for (auto& s : inputs) {
            results.push_back(lib_a::greet(s));
        }
        return results;
    }
}
