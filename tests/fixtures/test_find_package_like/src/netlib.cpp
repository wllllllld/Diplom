#include <thread>
#include <mutex>
#include <functional>

namespace netlib {

class Worker {
    std::thread thread_;
    std::mutex mutex_;
public:
    void start(std::function<void()> fn) {
        thread_ = std::thread(std::move(fn));
    }
    void join() {
        if (thread_.joinable()) thread_.join();
    }
};

} // namespace netlib
