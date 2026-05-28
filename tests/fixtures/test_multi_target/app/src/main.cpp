#include "lib_a.hpp"
#include "lib_b.hpp"
#include <iostream>
#include <vector>

int main() {
    std::cout << lib_a::greet("World") << "\n";

    std::vector<std::string> names = {"Alice", "Bob", "Charlie"};
    auto results = lib_b::process(names);
    for (auto& r : results) {
        std::cout << r << "\n";
    }

    std::cout << "lib_a version: " << lib_a::version() << "\n";
    return 0;
}
