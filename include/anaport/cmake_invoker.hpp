#pragma once
#include <filesystem>
#include <string>

namespace anaport {

struct InvokeResult {
    int exit_code{-1};
    std::string stdout_text;
    std::string stderr_text;
    bool success() const { return exit_code == 0; }
};

/// Write the CMake File API query file so CMake generates codemodel-v2 reply.
void write_file_api_query(const std::filesystem::path& build_dir);

/// Run cmake -S <src> -B <build> -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
/// Returns the result including stdout/stderr.
InvokeResult invoke_cmake_configure(
    const std::filesystem::path& source_dir,
    const std::filesystem::path& build_dir,
    bool verbose
);

} // namespace anaport
