#include "mathlib.hpp"
#include "strutils.hpp"
#include <iostream>

int main() {
    std::cout << "add(3,4) = " << mathlib::add(3, 4) << "\n";
    std::cout << "multiply(3,4) = " << mathlib::multiply(3, 4) << "\n";
    std::cout << "to_upper(hello) = " << strutils::to_upper("hello") << "\n";
    return 0;
}
