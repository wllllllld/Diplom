#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <optional>

namespace anaport {

// ─── Low-level data from CMake File API ──────────────────────────────────────

struct CompileFragment {
    std::string fragment;   ///< e.g. "-DFOO=1" or "-I/some/path"
    std::string role;       ///< "flags", "defines", "includes", ...
};

struct IncludePath {
    std::filesystem::path path;
    bool is_system{false};
};

struct CompileGroup {
    std::string language;                       ///< "C", "CXX", "ASM", ...
    std::vector<CompileFragment> fragments;
    std::vector<IncludePath> includes;
    std::vector<std::string> defines;
};

struct LinkFragment {
    std::string fragment;
    std::string role;   ///< "flags", "libraries", "libraryPath", ...
};

struct Artifact {
    std::filesystem::path path;
};

struct TargetDependency {
    std::string id;         ///< CMake target id
    std::string backtrace;
};

/// A single CMake target as parsed from File API.
struct RawTarget {
    std::string id;
    std::string name;
    std::string type;           ///< EXECUTABLE, STATIC_LIBRARY, SHARED_LIBRARY, MODULE_LIBRARY, OBJECT_LIBRARY, INTERFACE_LIBRARY, UTILITY
    std::filesystem::path source_dir;
    std::filesystem::path build_dir;
    std::vector<Artifact> artifacts;
    std::vector<CompileGroup> compile_groups;
    std::vector<LinkFragment> link_fragments;
    std::vector<TargetDependency> dependencies;
    std::vector<std::string> source_files;
};

/// Aggregated project data from File API index + codemodel.
struct RawProject {
    std::string name;
    std::string cmake_version;
    std::filesystem::path source_dir;  ///< Project root (absolute), from codemodel paths.source
    std::vector<RawTarget> targets;
};

// ─── compile_commands.json model ─────────────────────────────────────────────

/// A single entry from compile_commands.json
struct CompileCommandEntry {
    std::string directory;
    std::string file;
    std::string command;        ///< Full command string (may be empty if arguments is set)
    std::vector<std::string> arguments; ///< Tokenised command (may be empty if command is set)
};

/// Extracted flags from compile_commands entries (project-level observations)
struct CompileCommandsObservation {
    std::string file;           ///< Source file that produced this observation
    std::string flag;           ///< The extracted flag/token
    std::string kind;           ///< "include", "define", "sysroot", "arch_flag",
                                ///<  "compiler_flag", "absolute_path"
};

/// Result of parsing compile_commands.json
struct CompileCommandsData {
    bool found{false};
    bool parsed{false};
    int entry_count{0};
    std::string parse_error;    ///< Non-empty if parse partially failed
    std::vector<CompileCommandsObservation> observations;
};

// ─── CMakeCache.txt model ─────────────────────────────────────────────────────

/// A single variable from CMakeCache.txt
struct CacheVariable {
    std::string name;
    std::string type;   ///< BOOL, PATH, FILEPATH, STRING, INTERNAL
    std::string value;
};

/// Result of parsing CMakeCache.txt
struct CMakeCacheData {
    bool found{false};
    bool parsed{false};
    int variable_count{0};
    std::string parse_error;
    std::vector<CacheVariable> variables;
};

// ─── Portability analysis model ──────────────────────────────────────────────

enum class FindingSeverity {
    Info,
    Warning,
    Error
};

inline std::string severity_str(FindingSeverity s) {
    switch (s) {
        case FindingSeverity::Info:    return "info";
        case FindingSeverity::Warning: return "warning";
        case FindingSeverity::Error:   return "error";
    }
    return "unknown";
}

enum class FindingCategory {
    AbsolutePath,
    HardcodedSystemPath,
    PlatformSpecificLibrary,
    PlatformSpecificMacro,
    GeneratorExpression,
    NonPortableCompilerFlag,
    SuspiciousPath,
    CompilerFlag,
    RuntimePath,
    ToolchainHint,
    CacheVariable,
    CompileCommand,
    Info,
    ConfigureFailure
};

inline std::string category_str(FindingCategory c) {
    switch (c) {
        case FindingCategory::AbsolutePath:           return "absolute_path";
        case FindingCategory::HardcodedSystemPath:    return "hardcoded_system_path";
        case FindingCategory::PlatformSpecificLibrary:return "platform_specific_library";
        case FindingCategory::PlatformSpecificMacro:  return "platform_specific_macro";
        case FindingCategory::GeneratorExpression:    return "generator_expression";
        case FindingCategory::NonPortableCompilerFlag:return "non_portable_compiler_flag";
        case FindingCategory::SuspiciousPath:         return "suspicious_path";
        case FindingCategory::CompilerFlag:           return "compiler_flag";
        case FindingCategory::RuntimePath:            return "runtime_path";
        case FindingCategory::ToolchainHint:          return "toolchain_hint";
        case FindingCategory::CacheVariable:          return "cache_variable";
        case FindingCategory::CompileCommand:         return "compile_command";
        case FindingCategory::Info:                   return "info";
        case FindingCategory::ConfigureFailure:       return "configure_failure";
    }
    return "unknown";
}

/// Source/origin of a finding
enum class FindingSource {
    FileApi,
    CompileCommands,
    CMakeCache,
    Analyzer
};

inline std::string source_str(FindingSource s) {
    switch (s) {
        case FindingSource::FileApi:         return "file_api";
        case FindingSource::CompileCommands: return "compile_commands";
        case FindingSource::CMakeCache:      return "cmake_cache";
        case FindingSource::Analyzer:        return "analyzer";
    }
    return "unknown";
}

/// A single portability finding attached to a target or project.
struct Finding {
    FindingSeverity severity;
    FindingCategory category;
    FindingSource   source{FindingSource::FileApi};  ///< Which data source produced this
    std::string target_name;    ///< Empty if project-level
    std::string field;          ///< e.g. "include_path", "link_fragment", "define"
    std::string value;          ///< The offending value
    std::string message;        ///< Human-readable explanation
    std::string recommendation; ///< What to do instead
};

/// Dependency edge between two targets.
struct DependencyEdge {
    std::string from_target;
    std::string to_target;
    std::string kind; ///< "internal", "imported", "system", "unknown"
};

/// Full analysis result for a project.
struct AnalysisResult {
    std::string project_name;
    std::filesystem::path source_dir;
    std::string cmake_version;
    std::vector<RawTarget> targets;          ///< Targets included in analysis (service targets filtered out by default)
    std::vector<RawTarget> service_targets;  ///< CTest dashboard / service UTILITY targets that were excluded
    std::vector<Finding> findings;
    std::vector<DependencyEdge> edges;
    int warning_count{0};
    int error_count{0};
    int info_count{0};

    // Additional data sources metadata
    CompileCommandsData compile_commands;
    CMakeCacheData cmake_cache;
};

} // namespace anaport
