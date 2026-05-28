#pragma once
#include "model.hpp"
#include <filesystem>

namespace anaport {

/// Parse the CMake File API reply directory and build a RawProject.
/// build_dir must contain .cmake/api/v1/reply/ after cmake configure.
/// Throws std::runtime_error if reply is absent or malformed.
RawProject parse_file_api(const std::filesystem::path& build_dir, bool verbose);

} // namespace anaport
