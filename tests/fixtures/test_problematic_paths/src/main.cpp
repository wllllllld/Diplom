// Intentionally simplistic: the portability issues are in CMakeLists.txt
#include <iostream>
#include <cstdlib>

int main() {
    std::cout << "ProblematicPaths app — portability analysis test fixture\n";

#ifdef __linux__
    std::cout << "Running on Linux\n";
#elif defined(__APPLE__)
    std::cout << "Running on macOS\n";
#else
    std::cout << "Running on unknown platform\n";
#endif

    return EXIT_SUCCESS;
}
