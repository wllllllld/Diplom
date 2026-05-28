#pragma once
#include <filesystem>
#include <string>

namespace anaport {

/// Minimum severity level for report output filtering.
enum class SeverityFilter {
    Info    = 0,    ///< Show all findings (info, warning, error)
    Warning = 1,    ///< Show only warning and error
    Error   = 2     ///< Show only error
};

/// Command-line options parsed from argv.
struct Options {
    std::filesystem::path source_dir;   ///< Path to the CMake project source root
    std::filesystem::path output_dir;   ///< Where to write reports
    std::filesystem::path build_dir;    ///< Where to configure the project (tmp unless --keep-build-dir)
    bool keep_build_dir{false};         ///< Do not delete build dir after analysis
    bool verbose{false};                ///< Print extra diagnostic information
    bool include_utility_targets{false};///< Include CTest dashboard/service UTILITY targets in analysis

    // Output format selection (all false = generate all reports = default behavior)
    bool out_json{false};       ///< Generate report.json        (-j / --json)
    bool out_markdown{false};   ///< Generate report.md           (-m / --markdown)
    bool out_graphviz{false};   ///< Generate dependencies.dot    (-g / --graph)

    // Severity filtering
    SeverityFilter min_severity{SeverityFilter::Info}; ///< Minimum severity to emit in reports
    // --no-info is equivalent to --min-severity warning
};

/// Parse command-line arguments.
/// Throws std::runtime_error on invalid input.
Options parse_options(int argc, char* argv[]);

/// Print usage to stderr.
void print_usage(const char* program_name);

} // namespace anaport
