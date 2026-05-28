#pragma once
#include "model.hpp"
#include <filesystem>

namespace anaport {

/// Parse compile_commands.json from the build directory.
/// Returns CompileCommandsData with found/parsed flags and observations.
/// Gracefully handles missing or malformed files.
CompileCommandsData parse_compile_commands(const std::filesystem::path& build_dir,
                                           bool verbose = false);

} // namespace anaport
