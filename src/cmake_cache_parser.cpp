#include "anaport/cmake_cache_parser.hpp"
#include "anaport/logger.hpp"
#include <fstream>
#include <string>
#include <algorithm>

namespace anaport {

// ─── Parsing helpers ──────────────────────────────────────────────────────────

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

/// Parse one cache line: NAME:TYPE=VALUE
/// Returns false if the line is not a valid cache entry.
static bool parse_cache_line(const std::string& line, CacheVariable& var) {
    // Skip comments and empty lines
    if (line.empty()) return false;
    if (line[0] == '#' || line[0] == '/') return false;

    // Find the colon separating name from type
    auto colon = line.find(':');
    if (colon == std::string::npos) return false;

    // Find the equals separating type from value
    auto eq = line.find('=', colon);
    if (eq == std::string::npos) return false;

    var.name  = trim(line.substr(0, colon));
    var.type  = trim(line.substr(colon + 1, eq - colon - 1));
    var.value = trim(line.substr(eq + 1));

    if (var.name.empty() || var.type.empty()) return false;

    return true;
}

// ─── Variables of interest for portability ────────────────────────────────────

/// List of important variable name prefixes/patterns
static bool is_interesting_variable(const std::string& name) {
    // Exact names of high interest
    static const std::vector<std::string> EXACT = {
        "CMAKE_SYSTEM_NAME",
        "CMAKE_C_COMPILER",     "CMAKE_CXX_COMPILER",
        "CMAKE_C_FLAGS",        "CMAKE_CXX_FLAGS",
        "CMAKE_EXE_LINKER_FLAGS", "CMAKE_SHARED_LINKER_FLAGS",
        "CMAKE_PREFIX_PATH",    "CMAKE_INSTALL_PREFIX",
        "CMAKE_TOOLCHAIN_FILE", "CMAKE_FIND_ROOT_PATH",
        "CMAKE_OSX_SYSROOT",    "CMAKE_SYSROOT",
        "CMAKE_BUILD_TYPE",     "CMAKE_GENERATOR",
        "CMAKE_MODULE_PATH"
    };
    for (auto& e : EXACT) {
        if (name == e) return true;
    }

    // Suffix patterns of interest
    static const std::vector<std::string> SUFFIXES = {
        "_DIR", "_INCLUDE_DIR", "_INCLUDE_DIRS", "_LIBRARY",
        "_LIBRARIES", "_ROOT", "_PATH", "_EXECUTABLE"
    };
    for (auto& sfx : SUFFIXES) {
        if (name.size() > sfx.size() &&
            name.compare(name.size() - sfx.size(), sfx.size(), sfx) == 0) {
            return true;
        }
    }

    // Prefix patterns
    static const std::vector<std::string> PREFIXES = {
        "CMAKE_C_", "CMAKE_CXX_", "CMAKE_EXE_", "CMAKE_SHARED_",
        "CMAKE_STATIC_", "CMAKE_FIND_", "CMAKE_OSX_"
    };
    for (auto& pfx : PREFIXES) {
        if (name.size() > pfx.size() && name.compare(0, pfx.size(), pfx) == 0) {
            return true;
        }
    }

    return false;
}

// ─── Public API ───────────────────────────────────────────────────────────────

CMakeCacheData parse_cmake_cache(const std::filesystem::path& build_dir, bool verbose) {
    CMakeCacheData data;
    auto cache_path = build_dir / "CMakeCache.txt";

    if (!std::filesystem::exists(cache_path)) {
        log_verbose("CMakeCache.txt not found at: " + cache_path.string());
        data.found = false;
        return data;
    }

    data.found = true;
    log_info("Found CMakeCache.txt: " + cache_path.string());

    std::ifstream ifs(cache_path);
    if (!ifs) {
        data.parse_error = "Cannot open file: " + cache_path.string();
        log_warn("CMakeCache.txt: " + data.parse_error);
        return data;
    }

    data.parsed = true;
    std::string line;
    int line_no = 0;

    while (std::getline(ifs, line)) {
        ++line_no;
        line = trim(line);

        CacheVariable var;
        if (!parse_cache_line(line, var)) continue;

        // Only store interesting variables (reduces noise)
        if (is_interesting_variable(var.name)) {
            data.variables.push_back(var);
        }
        ++data.variable_count;
    }

    log_info("CMakeCache.txt: parsed " + std::to_string(data.variable_count)
             + " total variables, " + std::to_string(data.variables.size())
             + " interesting");

    if (verbose) {
        for (auto& v : data.variables) {
            log_verbose("  cache [" + v.type + "] " + v.name + " = " + v.value);
        }
    }

    return data;
}

} // namespace anaport
