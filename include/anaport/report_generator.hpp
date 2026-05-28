#pragma once
#include "model.hpp"
#include "options.hpp"
#include <filesystem>
#include <string>

namespace anaport {

/// Write report.json to output_dir (filtered by min_severity).
void write_json_report(const AnalysisResult& result,
                       const std::filesystem::path& output_dir,
                       SeverityFilter min_severity = SeverityFilter::Info);

/// Write report.md to output_dir (filtered by min_severity).
void write_markdown_report(const AnalysisResult& result,
                           const std::filesystem::path& output_dir,
                           SeverityFilter min_severity = SeverityFilter::Info);

/// Write dependencies.dot to output_dir.
void write_dot_report(const AnalysisResult& result,
                      const std::filesystem::path& output_dir,
                      SeverityFilter min_severity = SeverityFilter::Info);

/// Write reports based on Options (respects format flags and severity filter).
/// If none of out_json/out_markdown/out_graphviz are set, writes all three.
void write_reports(const AnalysisResult& result,
                   const std::filesystem::path& output_dir,
                   const Options& opts);

// ── Failure-report helpers (cmake configure failed / File API unavailable) ──

/// Context for a configure-failure report.
struct FailureContext {
    std::filesystem::path source_dir;
    std::filesystem::path build_dir;
    int  configure_exit_code{-1};      ///< cmake exit code (non-zero)
    std::string error_message;         ///< Exception or cmake error description
    CompileCommandsData compile_commands; ///< may be partially populated
    CMakeCacheData cmake_cache;           ///< may be partially populated
};

/// Write report.json diagnostic failure report to output_dir.
void write_failure_json(const FailureContext& ctx,
                        const std::filesystem::path& output_dir);

/// Write report.md diagnostic failure report to output_dir.
void write_failure_markdown(const FailureContext& ctx,
                            const std::filesystem::path& output_dir);

/// Write dependencies.dot empty graph with failure comment to output_dir.
void write_failure_dot(const FailureContext& ctx,
                       const std::filesystem::path& output_dir);

/// Write all requested failure reports respecting format flags.
/// If none of out_json/out_markdown/out_graphviz are set, writes all three.
void write_failure_reports(const FailureContext& ctx,
                           const std::filesystem::path& output_dir,
                           const Options& opts);

} // namespace anaport
