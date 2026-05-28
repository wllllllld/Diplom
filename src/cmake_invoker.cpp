#include "anaport/cmake_invoker.hpp"
#include "anaport/logger.hpp"
#include <stdexcept>
#include <fstream>
#include <array>
#include <memory>
#include <sstream>
#include <cstdio>

namespace anaport {

void write_file_api_query(const std::filesystem::path& build_dir) {
    namespace fs = std::filesystem;

    auto query_dir = build_dir / ".cmake" / "api" / "v1" / "query";
    std::error_code ec;
    fs::create_directories(query_dir, ec);
    if (ec) {
        throw std::runtime_error("Cannot create File API query dir: " + query_dir.string() + " — " + ec.message());
    }

    // Create the query marker file (empty file named "codemodel-v2")
    auto query_file = query_dir / "codemodel-v2";
    std::ofstream ofs(query_file);
    if (!ofs) {
        throw std::runtime_error("Cannot write File API query file: " + query_file.string());
    }
    ofs.close();

    log_verbose("Wrote File API query file: " + query_file.string());
}

static std::string run_command(const std::string& cmd, int& exit_code) {
    std::string output;
    std::array<char, 4096> buf{};

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        exit_code = -1;
        return "";
    }

    while (fgets(buf.data(), static_cast<int>(buf.size()), pipe)) {
        output += buf.data();
    }

    exit_code = pclose(pipe);
    // pclose returns the exit status in the same format as wait()
    if (WIFEXITED(exit_code)) {
        exit_code = WEXITSTATUS(exit_code);
    } else {
        exit_code = -1;
    }

    return output;
}

InvokeResult invoke_cmake_configure(
    const std::filesystem::path& source_dir,
    const std::filesystem::path& build_dir,
    bool verbose)
{
    InvokeResult result;

    // Build command
    std::string cmd = "cmake"
        " -S " + source_dir.string() +
        " -B " + build_dir.string() +
        " -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
        " 2>&1";

    log_info("Running: cmake -S " + source_dir.string() + " -B " + build_dir.string());

    int exit_code = 0;
    std::string combined = run_command(cmd, exit_code);
    result.exit_code = exit_code;
    result.stdout_text = combined;  // stderr is merged into stdout via 2>&1

    if (verbose) {
        std::cout << combined;
    }

    if (!result.success()) {
        result.stderr_text = combined;
        log_warn("cmake configure exited with code " + std::to_string(exit_code));
        if (!verbose) {
            log_warn("cmake output:\n" + combined);
        }
    } else {
        log_info("cmake configure succeeded");
    }

    return result;
}

} // namespace anaport
