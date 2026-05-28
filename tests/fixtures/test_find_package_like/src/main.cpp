#include <iostream>
#include <thread>

int main() {
    std::thread t([]{ std::cout << "Hello from thread\n"; });
    t.join();
    return 0;
}
