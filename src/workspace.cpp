#include "anaport/workspace.hpp"
#include "anaport/logger.hpp"
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <cerrno>

namespace anaport {

std::filesystem::path prepare_workspace(const Options& opts) {
    namespace fs = std::filesystem;

    // Validate source directory
    if (!fs::exists(opts.source_dir)) {
        throw std::runtime_error("Source directory does not exist: " + opts.source_dir.string());
    }
    if (!fs::is_directory(opts.source_dir)) {
        throw std::runtime_error("Source path is not a directory: " + opts.source_dir.string());
    }

    auto cmake_lists = opts.source_dir / "CMakeLists.txt";
    if (!fs::exists(cmake_lists)) {
        throw std::runtime_error("No CMakeLists.txt found in: " + opts.source_dir.string());
    }

    // Create output directory
    std::error_code ec;
    fs::create_directories(opts.output_dir, ec);
    if (ec) {
        throw std::runtime_error("Cannot create output directory: " + opts.output_dir.string() + " — " + ec.message());
    }

    // Determine build directory
    fs::path build_dir;
    if (!opts.build_dir.empty()) {
        build_dir = opts.build_dir;
        fs::create_directories(build_dir, ec);
        if (ec) {
            throw std::runtime_error("Cannot create build directory: " + build_dir.string() + " — " + ec.message());
        }
    } else {
        // Create a temp directory
        std::string tmpl = "/tmp/anaport_XXXXXX";
        char buf[256];
        std::strncpy(buf, tmpl.c_str(), sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        char* result = mkdtemp(buf);
        if (!result) {
            throw std::runtime_error(std::string("mkdtemp failed: ") + std::strerror(errno));
        }
        build_dir = result;
    }

    log_verbose("Source dir : " + opts.source_dir.string());
    log_verbose("Output dir : " + opts.output_dir.string());
    log_verbose("Build  dir : " + build_dir.string());

    return build_dir;
}

void cleanup_build_dir(const std::filesystem::path& build_dir, bool keep) {
    if (keep) {
        log_info("Keeping build directory: " + build_dir.string());
        return;
    }
    std::error_code ec;
    std::filesystem::remove_all(build_dir, ec);
    if (ec) {
        log_warn("Could not remove build directory " + build_dir.string() + ": " + ec.message());
    } else {
        log_verbose("Removed build directory: " + build_dir.string());
    }
}

} // namespace anaport
