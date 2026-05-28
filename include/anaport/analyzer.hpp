#pragma once
#include "model.hpp"
#include <filesystem>

namespace anaport {

/// Returns true if the given UTILITY target is a CTest dashboard service target
/// (Continuous*, Experimental*, Nightly*, NightlyMemoryCheck) that is not useful
/// for portability analysis.
bool is_ctest_service_target(const RawTarget& target);

/// Run portability analysis rules over a RawProject and produce an AnalysisResult.
/// Optionally incorporates findings from compile_commands.json and CMakeCache.txt.
/// build_dir is used to classify build-tree paths (RPATH, cache vars) as non-portable
/// but not as hard errors.
/// If include_utility_targets is false (default), CTest dashboard service targets
/// are excluded from analysis and recorded in result.service_targets.
AnalysisResult analyze(const RawProject& project,
                       const CompileCommandsData& cc_data,
                       const CMakeCacheData& cache_data,
                       const std::filesystem::path& build_dir,
                       bool verbose,
                       bool include_utility_targets = false);

} // namespace anaport
