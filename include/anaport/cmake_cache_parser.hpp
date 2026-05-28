#pragma once
#include "model.hpp"
#include <filesystem>

namespace anaport {

/// Parse CMakeCache.txt from the build directory.
/// Returns CMakeCacheData with all variables and their types/values.
/// Gracefully handles missing or malformed files.
CMakeCacheData parse_cmake_cache(const std::filesystem::path& build_dir,
                                 bool verbose = false);

} // namespace anaport
