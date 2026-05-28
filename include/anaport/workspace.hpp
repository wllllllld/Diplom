#pragma once
#include "options.hpp"
#include <filesystem>

namespace anaport {

/// Validates the project source directory and ensures build/output dirs exist.
/// Returns the resolved (absolute) build directory path.
/// Throws std::runtime_error on validation failure.
std::filesystem::path prepare_workspace(const Options& opts);

/// Delete the build directory (used when --keep-build-dir is not set).
void cleanup_build_dir(const std::filesystem::path& build_dir, bool keep);

} // namespace anaport
