#include "anaport/analyzer.hpp"
#include "anaport/logger.hpp"
#include <algorithm>
#include <functional>
#include <set>
#include <regex>
#include <unordered_map>

namespace anaport {

// ─── CTest service target classification ─────────────────────────────────────

static const std::vector<std::string> CTEST_SERVICE_PREFIXES = {
    "Continuous", "Experimental", "Nightly", "NightlyMemoryCheck",
};

static bool name_matches_ctest_prefix(const std::string& name) {
    for (auto& pfx : CTEST_SERVICE_PREFIXES) {
        if (name == pfx || name.starts_with(pfx)) {
            if (name.size() == pfx.size()) return true;
            char next = name[pfx.size()];
            if (next >= 'A' && next <= 'Z') return true;
        }
    }
    return false;
}

bool is_ctest_service_target(const RawTarget& target) {
    if (target.type != "UTILITY") return false;
    if (!name_matches_ctest_prefix(target.name)) return false;
    if (!target.compile_groups.empty()) return false;
    if (!target.link_fragments.empty()) return false;
    for (auto& art : target.artifacts) {
        auto s = art.path.string();
        if (s.find("CMakeFiles") == std::string::npos) return false;
    }
    return true;
}

// ─── Portability rule helpers ─────────────────────────────────────────────────

static const std::vector<std::string> HARDCODED_SYSTEM_PREFIXES = {
    "/usr/local", "/usr/X11", "/opt/homebrew", "/opt/local",
    "/opt/", "/home/", "/root/",
    "/sw/", "/mingw", "/cygdrive",
    "/nix/store", "/Users/", "/tmp/", "/var/folders/"
};

static const std::vector<std::string> VENDOR_CUSTOM_PREFIXES = {
    "/opt/homebrew", "/opt/local", "/nix/store",
    "/opt/conda", "/opt/miniconda", "/opt/anaconda",
    "/opt/intel", "/opt/cuda", "/opt/rocm",
    "/home/linuxbrew", "/usr/local/Cellar"
};

static const std::vector<std::string> PLATFORM_LIBS = {
    // Linux-specific
    "pthread", "dl", "rt", "m", "resolv", "aio", "nsl",
    "X11", "Xext", "xcb", "GL", "GLU", "GLUT",
    // macOS-specific
    "objc", "CoreFoundation", "AppKit", "Foundation", "IOKit",
    // BSD-specific
    "kvm", "procstat",
    // Windows
    "ws2_32", "advapi32", "kernel32",
    // Common system libs that may be absent on some platforms
    "uuid", "crypto", "ssl", "z", "bz2", "lzma"
};

static const std::vector<std::string> PLATFORM_MACROS = {
    "__linux__", "__APPLE__", "_WIN32", "__FreeBSD__", "__NetBSD__",
    "__OpenBSD__", "__sun", "__CYGWIN__", "LINUX", "APPLE", "WIN32",
    "_GNU_SOURCE", "__GLIBC__", "__MUSL__"
};

/// Architecture/CPU-specific compile flags
static const std::vector<std::string> ARCH_FLAGS = {
    "-march=", "-mtune=", "-mcpu=", "-msse", "-mavx", "-mfpu=",
    "-m32", "-m64", "-mfloat-abi=", "-mabi=", "-mthumb", "-marm"
};

/// Compiler-specific flags (not universally portable)
static const std::vector<std::string> COMPILER_SPECIFIC_FLAGS = {
    "-stdlib=", "-static-libstdc++", "-static-libgcc",
    "--target=", "-target ", "--sysroot=", "--sysroot", "-isysroot"
};

/// RPATH / runtime linker path flags
static const std::vector<std::string> RPATH_FLAGS = {
    "-rpath", "-rpath-link", "-Wl,-rpath", "-Wl,--rpath",
    "-Wl,-R", "RPATH", "-install_name", "-Wl,-install_name"
};

static bool has_prefix(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

static bool is_absolute_path(const std::filesystem::path& p) {
    return p.is_absolute();
}

static bool is_hardcoded_system_path(const std::string& path_str) {
    for (auto& pfx : HARDCODED_SYSTEM_PREFIXES) {
        if (has_prefix(path_str, pfx)) return true;
    }
    return false;
}

static bool is_vendor_custom_prefix(const std::string& path_str) {
    for (auto& pfx : VENDOR_CUSTOM_PREFIXES) {
        if (has_prefix(path_str, pfx)) return true;
    }
    return false;
}

static std::string extract_lib_name(const std::string& fragment) {
    if (fragment.size() > 2 && fragment.substr(0, 2) == "-l") {
        return fragment.substr(2);
    }
    std::filesystem::path p(fragment);
    std::string stem = p.stem().string();
    if (stem.size() > 3 && stem.substr(0, 3) == "lib") {
        return stem.substr(3);
    }
    return stem;
}

static bool is_platform_lib(const std::string& libname) {
    for (auto& pl : PLATFORM_LIBS) {
        if (libname == pl) return true;
    }
    return false;
}

static bool contains_generator_expr(const std::string& s) {
    return s.find("$<") != std::string::npos;
}

static bool is_arch_flag(const std::string& frag) {
    for (auto& af : ARCH_FLAGS) {
        if (frag.find(af) != std::string::npos) return true;
    }
    return false;
}

static bool is_compiler_specific_flag(const std::string& frag) {
    for (auto& cf : COMPILER_SPECIFIC_FLAGS) {
        if (frag.find(cf) != std::string::npos) return true;
    }
    return false;
}

static bool is_rpath_flag(const std::string& frag) {
    for (auto& rf : RPATH_FLAGS) {
        if (frag.find(rf) != std::string::npos) return true;
    }
    return false;
}

/// Returns true if path_str is inside the build tree or project source tree.
static bool is_local_tree_path(const std::string& path_str,
                                const std::string& source_root,
                                const std::string& build_root)
{
    if (!source_root.empty() && has_prefix(path_str, source_root)) return true;
    if (!build_root.empty()  && has_prefix(path_str, build_root))  return true;
    return false;
}

/// Extract just the path component from an RPATH flag like
/// "-Wl,-rpath,/some/path" or "-rpath /some/path".
static std::string extract_rpath_value(const std::string& frag) {
    // "-Wl,-rpath,/path"  →  /path
    auto comma = frag.rfind(',');
    if (comma != std::string::npos && frag[comma + 1] == '/') {
        return frag.substr(comma + 1);
    }
    // "-rpath /path" or "-Wl,-R/path"
    auto slash = frag.find('/');
    if (slash != std::string::npos) {
        return frag.substr(slash);
    }
    return frag;
}

// ─── File API analysis checks ─────────────────────────────────────────────────

static void check_include_paths(
    const RawTarget& target,
    const CompileGroup& cg,
    std::vector<Finding>& findings,
    const std::function<bool(const std::string&)>& is_project_local)
{
    for (auto& inc : cg.includes) {
        auto path_str = inc.path.string();
        if (path_str.empty()) continue;

        if (contains_generator_expr(path_str)) {
            findings.push_back({
                FindingSeverity::Info, FindingCategory::GeneratorExpression,
                FindingSource::FileApi,
                target.name, "include_path", path_str,
                "Include path contains a generator expression: " + path_str,
                "Ensure the generator expression resolves correctly on all target platforms."
            });
            continue;
        }

        if (is_absolute_path(inc.path)) {
            if (is_project_local(path_str)) {
                findings.push_back({
                    FindingSeverity::Info, FindingCategory::AbsolutePath,
                    FindingSource::FileApi,
                    target.name, "include_path", path_str,
                    "Project-local absolute include path (resolved BUILD_INTERFACE): " + path_str,
                    "This is typical for BUILD_INTERFACE generator expressions and is generally fine."
                });
                continue;
            }
            if (is_vendor_custom_prefix(path_str)) {
                findings.push_back({
                    FindingSeverity::Warning, FindingCategory::SuspiciousPath,
                    FindingSource::FileApi,
                    target.name, "include_path", path_str,
                    "Vendor/custom prefix in include path: " + path_str,
                    "Use find_package() or CMake variables instead of hard-coding vendor-specific paths "
                    "like /opt/homebrew, /nix/store, /opt/conda."
                });
            } else if (is_hardcoded_system_path(path_str)) {
                findings.push_back({
                    FindingSeverity::Warning, FindingCategory::HardcodedSystemPath,
                    FindingSource::FileApi,
                    target.name, "include_path", path_str,
                    "Hard-coded system include path: " + path_str,
                    "Use find_package() or target_include_directories() with a CMake variable. "
                    "Avoid hard-coding paths like /usr/local, /opt, /home."
                });
            } else if (path_str.find("/usr/include") == std::string::npos &&
                       path_str.find("/usr/lib") == std::string::npos) {
                findings.push_back({
                    FindingSeverity::Warning, FindingCategory::AbsolutePath,
                    FindingSource::FileApi,
                    target.name, "include_path", path_str,
                    "Absolute include path (non-standard location): " + path_str,
                    "Replace with a CMake variable, find_package() result, or relative path."
                });
            }
        }
    }
}

static void check_defines(
    const RawTarget& target,
    const CompileGroup& cg,
    std::vector<Finding>& findings)
{
    for (auto& def : cg.defines) {
        for (auto& pm : PLATFORM_MACROS) {
            if (def == pm || def.starts_with(pm + "=") || def.starts_with(pm + " ")) {
                findings.push_back({
                    FindingSeverity::Warning, FindingCategory::PlatformSpecificMacro,
                    FindingSource::FileApi,
                    target.name, "define", def,
                    "Platform-specific preprocessor macro: " + def,
                    "Prefer CMake's platform detection (CMAKE_SYSTEM_NAME, etc.) and "
                    "use target_compile_definitions() conditionally."
                });
            }
        }
    }
}

static void check_compile_flags(
    const RawTarget& target,
    const CompileGroup& cg,
    std::vector<Finding>& findings)
{
    for (auto& frag : cg.fragments) {
        if (frag.fragment.empty()) continue;

        if (is_arch_flag(frag.fragment)) {
            findings.push_back({
                FindingSeverity::Warning, FindingCategory::NonPortableCompilerFlag,
                FindingSource::FileApi,
                target.name, "compile_flag", frag.fragment,
                "Architecture-specific compiler flag: " + frag.fragment,
                "Guard architecture-specific flags with CMake conditional logic "
                "(e.g., if(CMAKE_SYSTEM_PROCESSOR MATCHES \"x86_64\"))."
            });
        } else if (is_compiler_specific_flag(frag.fragment)) {
            findings.push_back({
                FindingSeverity::Warning, FindingCategory::CompilerFlag,
                FindingSource::FileApi,
                target.name, "compile_flag", frag.fragment,
                "Compiler-specific flag: " + frag.fragment,
                "Check that this flag is supported on all target compilers/platforms. "
                "Use generator expressions or CMake if() guards."
            });
        }

        if (contains_generator_expr(frag.fragment)) {
            findings.push_back({
                FindingSeverity::Info, FindingCategory::GeneratorExpression,
                FindingSource::FileApi,
                target.name, "compile_flag", frag.fragment,
                "Compile flag contains a generator expression: " + frag.fragment,
                "Verify this resolves correctly on all target platforms."
            });
        }
    }
}

/// check_link_fragments needs to know source/build roots to downgrade
/// build-tree RPATH entries from warning to info.
static void check_link_fragments(
    const RawTarget& target,
    std::vector<Finding>& findings,
    const std::string& source_root,
    const std::string& build_root)
{
    for (auto& lf : target.link_fragments) {
        if (lf.fragment.empty()) continue;

        // ── RPATH / runtime path flags ──────────────────────────────────────
        if (is_rpath_flag(lf.fragment)) {
            // Extract the actual path embedded in the flag to classify it
            std::string rpath_val = extract_rpath_value(lf.fragment);
            bool is_local = is_local_tree_path(rpath_val, source_root, build_root);

            if (is_local) {
                // Build-tree RPATH: informational — expected during development,
                // but should be replaced with a proper install RPATH.
                findings.push_back({
                    FindingSeverity::Info, FindingCategory::RuntimePath,
                    FindingSource::FileApi,
                    target.name, "link_fragment", lf.fragment,
                    "Build-tree RPATH detected: " + lf.fragment,
                    "This RPATH points inside the build/source directory and is typical for "
                    "build-tree executables. Verify that CMAKE_INSTALL_RPATH is set correctly "
                    "for installed binaries."
                });
            } else {
                // External absolute RPATH: portability warning
                findings.push_back({
                    FindingSeverity::Warning, FindingCategory::RuntimePath,
                    FindingSource::FileApi,
                    target.name, "link_fragment", lf.fragment,
                    "Hard-coded RPATH to external location: " + lf.fragment,
                    "Hard-coded RPATH to a non-project path reduces portability. "
                    "Use CMAKE_INSTALL_RPATH with $ORIGIN or relocatable paths."
                });
            }
            continue;
        }

        if (lf.role == "libraryPath" || lf.role == "libraries") {
            if (is_absolute_path(lf.fragment) && is_hardcoded_system_path(lf.fragment)) {
                findings.push_back({
                    FindingSeverity::Warning, FindingCategory::HardcodedSystemPath,
                    FindingSource::FileApi,
                    target.name, "link_library_path", lf.fragment,
                    "Hard-coded library search path: " + lf.fragment,
                    "Use find_library() or find_package() instead of hard-coding paths."
                });
            }
        }

        if (contains_generator_expr(lf.fragment)) {
            findings.push_back({
                FindingSeverity::Info, FindingCategory::GeneratorExpression,
                FindingSource::FileApi,
                target.name, "link_fragment", lf.fragment,
                "Link fragment contains a generator expression.",
                "Verify this resolves correctly on all target platforms."
            });
        }

        if (lf.role == "libraries" || lf.fragment.starts_with("-l")) {
            std::string libname = extract_lib_name(lf.fragment);
            if (is_platform_lib(libname)) {
                findings.push_back({
                    FindingSeverity::Warning, FindingCategory::PlatformSpecificLibrary,
                    FindingSource::FileApi,
                    target.name, "link_library", lf.fragment,
                    "Platform-specific or OS-dependent library: " + libname,
                    "Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or "
                    "use find_package(Threads), find_library(), etc."
                });
            }

            if (lf.fragment.size() > 1 && lf.fragment[0] == '/') {
                if (is_vendor_custom_prefix(lf.fragment)) {
                    findings.push_back({
                        FindingSeverity::Warning, FindingCategory::SuspiciousPath,
                        FindingSource::FileApi,
                        target.name, "link_library", lf.fragment,
                        "Vendor/custom prefix in library path: " + lf.fragment,
                        "Use find_library() to locate the library portably."
                    });
                } else if (is_hardcoded_system_path(lf.fragment)) {
                    findings.push_back({
                        FindingSeverity::Warning, FindingCategory::HardcodedSystemPath,
                        FindingSource::FileApi,
                        target.name, "link_library", lf.fragment,
                        "Absolute path to library: " + lf.fragment,
                        "Use find_library() to locate the library portably."
                    });
                } else {
                    findings.push_back({
                        FindingSeverity::Warning, FindingCategory::AbsolutePath,
                        FindingSource::FileApi,
                        target.name, "link_library", lf.fragment,
                        "Absolute path to library (possibly project-local): " + lf.fragment,
                        "Ensure this path is valid on all target systems."
                    });
                }
            }
        }
    }
}

// ─── Dependency graph ─────────────────────────────────────────────────────────

static std::string classify_dependency_kind(
    const std::string& dep_id,
    const std::unordered_map<std::string, std::string>& id_to_type)
{
    auto it = id_to_type.find(dep_id);
    if (it == id_to_type.end()) return "unknown";
    auto& type = it->second;
    if (type == "UTILITY") return "utility";
    if (type == "INTERFACE_LIBRARY") return "interface";
    if (type.find("LIBRARY") != std::string::npos || type == "EXECUTABLE") return "internal";
    return "unknown";
}

// ─── compile_commands.json analysis ──────────────────────────────────────────

static void analyze_compile_commands(
    const CompileCommandsData& cc_data,
    const std::string& source_root,
    const std::string& build_root,
    std::vector<Finding>& findings)
{
    if (!cc_data.found || !cc_data.parsed) return;

    // Deduplicate by (flag, kind) for project-level findings
    std::set<std::pair<std::string,std::string>> seen;

    for (auto& obs : cc_data.observations) {
        auto key = std::make_pair(obs.flag, obs.kind);
        if (seen.count(key)) continue;
        seen.insert(key);

        if (obs.kind == "include") {
            // 1. Project-local paths (source tree or build tree) → info, not warning
            if (is_local_tree_path(obs.flag, source_root, build_root)) {
                findings.push_back({
                    FindingSeverity::Info, FindingCategory::AbsolutePath,
                    FindingSource::CompileCommands,
                    "", "include_path", obs.flag,
                    "Project-local include path in compile_commands.json: " + obs.flag,
                    "This path is within the project source or build tree and is expected."
                });
                continue;
            }
            // 2. Vendor/custom prefixes → warning
            if (is_vendor_custom_prefix(obs.flag)) {
                findings.push_back({
                    FindingSeverity::Warning, FindingCategory::SuspiciousPath,
                    FindingSource::CompileCommands,
                    "", "include_path", obs.flag,
                    "Vendor/custom include path in compile_commands.json: " + obs.flag,
                    "This path is specific to a local environment. Use find_package() or "
                    "target_include_directories() with CMake variables."
                });
            // 3. Other hardcoded system paths → warning
            } else if (is_hardcoded_system_path(obs.flag)) {
                findings.push_back({
                    FindingSeverity::Warning, FindingCategory::HardcodedSystemPath,
                    FindingSource::CompileCommands,
                    "", "include_path", obs.flag,
                    "Hard-coded include path in compile_commands.json: " + obs.flag,
                    "Avoid hard-coding paths like /usr/local, /opt, /home in compile commands."
                });
            }
            // 4. Standard system includes (/usr/include, /usr/lib) → silently skip
        } else if (obs.kind == "define") {
            for (auto& pm : PLATFORM_MACROS) {
                if (obs.flag == pm || obs.flag.starts_with(pm + "=")) {
                    findings.push_back({
                        FindingSeverity::Warning, FindingCategory::PlatformSpecificMacro,
                        FindingSource::CompileCommands,
                        "", "define", obs.flag,
                        "Platform-specific macro in compile_commands.json: " + obs.flag,
                        "Prefer CMake's platform detection via CMAKE_SYSTEM_NAME."
                    });
                    break;
                }
            }
        } else if (obs.kind == "sysroot") {
            findings.push_back({
                FindingSeverity::Warning, FindingCategory::ToolchainHint,
                FindingSource::CompileCommands,
                "", "sysroot", obs.flag,
                "Sysroot specified in compile_commands.json: " + obs.flag,
                "Sysroot settings indicate cross-compilation or non-standard environment. "
                "Document the toolchain requirements clearly."
            });
        } else if (obs.kind == "arch_flag") {
            findings.push_back({
                FindingSeverity::Warning, FindingCategory::CompilerFlag,
                FindingSource::CompileCommands,
                "", "compile_flag", obs.flag,
                "Architecture-specific flag in compile_commands.json: " + obs.flag,
                "Guard architecture-specific flags with CMake platform conditions."
            });
        } else if (obs.kind == "compiler_flag") {
            if (obs.flag.find("-stdlib=") != std::string::npos ||
                obs.flag.find("-static-libstdc++") != std::string::npos ||
                obs.flag.find("-static-libgcc") != std::string::npos ||
                obs.flag.find("--target=") != std::string::npos) {
                findings.push_back({
                    FindingSeverity::Warning, FindingCategory::CompilerFlag,
                    FindingSource::CompileCommands,
                    "", "compile_flag", obs.flag,
                    "Compiler-specific flag in compile_commands.json: " + obs.flag,
                    "Verify this flag is supported on all target compilers."
                });
            } else {
                findings.push_back({
                    FindingSeverity::Info, FindingCategory::CompileCommand,
                    FindingSource::CompileCommands,
                    "", "compile_flag", obs.flag,
                    "Non-portable compile flag in compile_commands.json: " + obs.flag,
                    "Check portability on all target platforms."
                });
            }
        } else if (obs.kind == "absolute_path") {
            // Skip project-local absolute paths silently
            if (is_local_tree_path(obs.flag, source_root, build_root)) continue;
            if (is_vendor_custom_prefix(obs.flag) || is_hardcoded_system_path(obs.flag)) {
                findings.push_back({
                    FindingSeverity::Info, FindingCategory::AbsolutePath,
                    FindingSource::CompileCommands,
                    "", "absolute_path", obs.flag,
                    "Absolute path token in compile_commands.json: " + obs.flag,
                    "Absolute paths reduce portability. Use CMake variables or find_package()."
                });
            }
        }
    }
}

// ─── CMakeCache.txt analysis ──────────────────────────────────────────────────

/// CMake internal/metadata variable names that are not portability issues.
/// These are auto-generated by CMake and reflect the current build environment,
/// not user decisions about paths or flags.
static bool is_cmake_internal_var(const std::string& name, const std::string& type) {
    // INTERNAL and STATIC typed variables are CMake bookkeeping
    if (type == "INTERNAL" || type == "STATIC") return true;

    // Exact internal names
    static const std::vector<std::string> EXACT_INTERNAL = {
        "CMAKE_CACHEFILE_DIR",
        "CMAKE_FIND_PACKAGE_REDIRECTS_DIR",
        "CMAKE_HOME_DIRECTORY",
        "CMAKE_ROOT",
        "CMAKE_GENERATOR",
    };
    for (auto& n : EXACT_INTERNAL) {
        if (name == n) return true;
    }

    // *_SOURCE_DIR and *_BINARY_DIR are per-project/subproject CMake metadata
    if (name.size() > 11 && name.ends_with("_SOURCE_DIR")) return true;
    if (name.size() > 11 && name.ends_with("_BINARY_DIR")) return true;

    // *-ADVANCED flags are CMake GUI bookkeeping
    if (name.ends_with("-ADVANCED")) return true;

    return false;
}

static void analyze_cmake_cache(
    const CMakeCacheData& cache_data,
    const std::string& source_root,
    const std::string& build_root,
    std::vector<Finding>& findings)
{
    if (!cache_data.found || !cache_data.parsed) return;

    for (auto& var : cache_data.variables) {
        const auto& name  = var.name;
        const auto& type  = var.type;
        const auto& value = var.value;

        // Skip CMake internal/metadata variables entirely — they are not
        // portability issues and only add noise.
        if (is_cmake_internal_var(name, type)) continue;

        if (value.empty()) continue;

        // ── Compiler paths ────────────────────────────────────────────────
        if (name == "CMAKE_C_COMPILER" || name == "CMAKE_CXX_COMPILER") {
            if (is_hardcoded_system_path(value) || is_vendor_custom_prefix(value)) {
                findings.push_back({
                    FindingSeverity::Warning, FindingCategory::ToolchainHint,
                    FindingSource::CMakeCache,
                    "", "cmake_cache_variable", name + "=" + value,
                    "Non-standard compiler path in CMakeCache: " + name + " = " + value,
                    "Document the compiler requirement or use a toolchain file "
                    "(CMAKE_TOOLCHAIN_FILE) for cross-compilation."
                });
            } else {
                findings.push_back({
                    FindingSeverity::Info, FindingCategory::CacheVariable,
                    FindingSource::CMakeCache,
                    "", "cmake_cache_variable", name + "=" + value,
                    "Compiler: " + name + " = " + value,
                    "Ensure this compiler is available on all target build environments."
                });
            }
            continue;
        }

        // ── Toolchain file ────────────────────────────────────────────────
        if (name == "CMAKE_TOOLCHAIN_FILE") {
            findings.push_back({
                FindingSeverity::Info, FindingCategory::ToolchainHint,
                FindingSource::CMakeCache,
                "", "cmake_cache_variable", name + "=" + value,
                "Toolchain file in use: " + value,
                "Cross-compilation toolchain detected. Ensure it is distributed with the project "
                "or documented in the build instructions."
            });
            continue;
        }

        // ── Sysroot ───────────────────────────────────────────────────────
        if (name == "CMAKE_OSX_SYSROOT" || name == "CMAKE_SYSROOT") {
            findings.push_back({
                FindingSeverity::Warning, FindingCategory::ToolchainHint,
                FindingSource::CMakeCache,
                "", "cmake_cache_variable", name + "=" + value,
                "Platform sysroot configured: " + name + " = " + value,
                "Sysroot is platform-specific. Document the SDK/sysroot requirement."
            });
            continue;
        }

        // ── Compiler flags ────────────────────────────────────────────────
        if (name == "CMAKE_C_FLAGS" || name == "CMAKE_CXX_FLAGS" ||
            name == "CMAKE_EXE_LINKER_FLAGS" || name == "CMAKE_SHARED_LINKER_FLAGS") {
            if (!value.empty()) {
                bool has_arch = false;
                for (auto& af : ARCH_FLAGS) {
                    if (value.find(af) != std::string::npos) { has_arch = true; break; }
                }
                FindingSeverity sev = has_arch ? FindingSeverity::Warning : FindingSeverity::Info;
                FindingCategory cat = has_arch ? FindingCategory::NonPortableCompilerFlag
                                               : FindingCategory::CacheVariable;
                findings.push_back({
                    sev, cat,
                    FindingSource::CMakeCache,
                    "", "cmake_cache_variable", name + "=" + value,
                    "Build flags in CMakeCache: " + name + " = " + value,
                    has_arch
                        ? "Architecture-specific flags detected. Guard with CMAKE_SYSTEM_PROCESSOR checks."
                        : "These flags are set in the cache and may differ on other systems."
                });
            }
            continue;
        }

        // ── Directory/path variables ──────────────────────────────────────
        // Skip if it's a build-tree or source-tree path (CMake internal metadata)
        if ((name.find("_DIR") != std::string::npos ||
             name.find("_INCLUDE_DIR") != std::string::npos ||
             name.find("_LIBRARY") != std::string::npos ||
             name.find("_ROOT") != std::string::npos) &&
            !value.empty() && value[0] == '/') {

            // If it lives inside source or build tree, it's metadata — skip silently
            if (is_local_tree_path(value, source_root, build_root)) continue;

            if (is_vendor_custom_prefix(value)) {
                findings.push_back({
                    FindingSeverity::Warning, FindingCategory::SuspiciousPath,
                    FindingSource::CMakeCache,
                    "", "cmake_cache_variable", name + "=" + value,
                    "Vendor/custom path in CMakeCache: " + name + " = " + value,
                    "This path is specific to a local environment. Use a toolchain file or "
                    "document the dependency location requirement."
                });
            } else if (is_hardcoded_system_path(value)) {
                findings.push_back({
                    FindingSeverity::Warning, FindingCategory::HardcodedSystemPath,
                    FindingSource::CMakeCache,
                    "", "cmake_cache_variable", name + "=" + value,
                    "Hard-coded path in CMakeCache: " + name + " = " + value,
                    "Hard-coded paths reduce portability. Use find_package() or "
                    "CMAKE_PREFIX_PATH to locate dependencies."
                });
            } else {
                findings.push_back({
                    FindingSeverity::Info, FindingCategory::CacheVariable,
                    FindingSource::CMakeCache,
                    "", "cmake_cache_variable", name + "=" + value,
                    "Absolute path variable in CMakeCache: " + name + " = " + value,
                    "Verify this path is consistent across build environments."
                });
            }
            continue;
        }

        // ── CMAKE_PREFIX_PATH / CMAKE_FIND_ROOT_PATH ──────────────────────
        if (name == "CMAKE_PREFIX_PATH" || name == "CMAKE_FIND_ROOT_PATH") {
            if (!value.empty()) {
                bool suspicious = is_vendor_custom_prefix(value) || is_hardcoded_system_path(value);
                // Still skip if it's a local tree path
                if (is_local_tree_path(value, source_root, build_root)) continue;
                findings.push_back({
                    suspicious ? FindingSeverity::Warning : FindingSeverity::Info,
                    suspicious ? FindingCategory::SuspiciousPath : FindingCategory::CacheVariable,
                    FindingSource::CMakeCache,
                    "", "cmake_cache_variable", name + "=" + value,
                    "Search path in CMakeCache: " + name + " = " + value,
                    "Custom search paths are environment-specific. Consider documenting "
                    "or using a toolchain file."
                });
            }
            continue;
        }

        // ── CMAKE_SYSTEM_NAME ─────────────────────────────────────────────
        if (name == "CMAKE_SYSTEM_NAME") {
            findings.push_back({
                FindingSeverity::Info, FindingCategory::CacheVariable,
                FindingSource::CMakeCache,
                "", "cmake_cache_variable", name + "=" + value,
                "Target system: " + value,
                "Analysis was performed targeting: " + value + ". "
                "Findings may differ on other target platforms."
            });
            continue;
        }

        // ── CMAKE_INSTALL_PREFIX ──────────────────────────────────────────
        if (name == "CMAKE_INSTALL_PREFIX") {
            // /usr/local is the default — flag it as info only (it's the CMake default,
            // not a user-hardcoded problematic path)
            findings.push_back({
                FindingSeverity::Info, FindingCategory::CacheVariable,
                FindingSource::CMakeCache,
                "", "cmake_cache_variable", name + "=" + value,
                "Install prefix: " + value,
                "CMAKE_INSTALL_PREFIX is " + value + ". "
                "Ensure this is appropriate for all target environments."
            });
            continue;
        }
    }
}

// ─── Main analyze function ─────────────────────────────────────────────────────

AnalysisResult analyze(const RawProject& project,
                       const CompileCommandsData& cc_data,
                       const CMakeCacheData& cache_data,
                       const std::filesystem::path& build_dir,
                       bool verbose,
                       bool include_utility_targets)
{
    AnalysisResult result;
    result.project_name = project.name;
    result.cmake_version = project.cmake_version;
    result.compile_commands = cc_data;
    result.cmake_cache = cache_data;

    // String forms for path comparison helpers
    std::string source_root = project.source_dir.string();
    std::string build_root  = build_dir.string();

    // ─── Filter CTest service targets ─────────────────────────────────
    for (auto& t : project.targets) {
        if (!include_utility_targets && is_ctest_service_target(t)) {
            result.service_targets.push_back(t);
            log_verbose("  Excluded CTest service target: " + t.name);
        } else {
            result.targets.push_back(t);
        }
    }

    if (!result.service_targets.empty()) {
        log_info("Excluded " + std::to_string(result.service_targets.size())
            + " CTest dashboard service target(s). Use --include-utility-targets to include them.");
    }

    if (source_root.empty() && !result.source_dir.empty()) {
        source_root = result.source_dir.string();
    }

    auto is_project_local = [&](const std::string& path_str) -> bool {
        return is_local_tree_path(path_str, source_root, build_root);
    };

    // Build id→type map
    std::unordered_map<std::string, std::string> id_to_type;
    for (auto& t : project.targets) {
        id_to_type[t.id] = t.type;
    }

    log_info("Analyzing project: " + project.name + ", targets: " + std::to_string(result.targets.size())
        + " (excluded service: " + std::to_string(result.service_targets.size()) + ")");

    // ─── Analyze each target (File API) ───────────────────────────────
    for (auto& target : result.targets) {
        log_verbose("  Checking target: " + target.name + " [" + target.type + "]");

        for (auto& cg : target.compile_groups) {
            check_include_paths(target, cg, result.findings, is_project_local);
            check_defines(target, cg, result.findings);
            check_compile_flags(target, cg, result.findings);
        }

        check_link_fragments(target, result.findings, source_root, build_root);

        for (auto& dep : target.dependencies) {
            if (dep.id.empty()) continue;
            DependencyEdge edge;
            edge.from_target = target.name;
            for (auto& t2 : project.targets) {
                if (t2.id == dep.id) { edge.to_target = t2.name; break; }
            }
            if (edge.to_target.empty()) edge.to_target = dep.id;
            edge.kind = classify_dependency_kind(dep.id, id_to_type);
            result.edges.push_back(edge);
        }
    }

    // ─── Analyze compile_commands.json ────────────────────────────────
    if (cc_data.found) {
        if (cc_data.parsed) {
            analyze_compile_commands(cc_data, source_root, build_root, result.findings);
        } else {
            result.findings.push_back({
                FindingSeverity::Info, FindingCategory::CompileCommand,
                FindingSource::CompileCommands,
                "", "compile_commands_json", cc_data.parse_error,
                "compile_commands.json found but could not be parsed: " + cc_data.parse_error,
                "Ensure cmake is configured with CMAKE_EXPORT_COMPILE_COMMANDS=ON."
            });
        }
    }

    // ─── Analyze CMakeCache.txt ────────────────────────────────────────
    if (cache_data.found) {
        if (cache_data.parsed) {
            analyze_cmake_cache(cache_data, source_root, build_root, result.findings);
        } else {
            result.findings.push_back({
                FindingSeverity::Info, FindingCategory::CacheVariable,
                FindingSource::CMakeCache,
                "", "cmake_cache_txt", cache_data.parse_error,
                "CMakeCache.txt found but could not be parsed: " + cache_data.parse_error,
                "Check the CMakeCache.txt file for corruption."
            });
        }
    }

    // ─── Deduplicate info findings across targets ────────────────────────
    // Info findings that carry the same (source, category, value) key but differ
    // only by target_name are collapsed into one project-level finding.  This
    // prevents inflated info counts in projects with many targets that all share
    // the same include paths or RPATH (e.g. fmt with 25 test executables all
    // having BUILD_INTERFACE include paths).  Warnings and errors are intentionally
    // kept per-target so the developer knows exactly which target needs attention.
    {
        // Group info findings by dedup key: source + category + value + message prefix
        std::unordered_map<std::string, std::vector<size_t>> info_groups;
        for (size_t i = 0; i < result.findings.size(); ++i) {
            const auto& f = result.findings[i];
            if (f.severity != FindingSeverity::Info) continue;
            // Dedup key: source + category + value (same path/flag) + first 80 chars of message
            std::string key = source_str(f.source) + "\x00"
                            + category_str(f.category) + "\x00"
                            + f.value + "\x00"
                            + f.message.substr(0, 80);
            info_groups[key].push_back(i);
        }

        // For groups with multiple targets: collapse into one project-level finding
        std::set<size_t> to_remove;
        std::vector<Finding> collapsed;
        for (auto& [key, indices] : info_groups) {
            if (indices.size() <= 1) continue;
            // Count distinct targets (some may be empty = project-level)
            std::set<std::string> targets;
            for (auto idx : indices) targets.insert(result.findings[idx].target_name);
            if (targets.size() <= 1) continue;  // same target repeated — keep as-is

            // Build a collapsed finding: take first as template, clear target, add count note
            Finding rep = result.findings[indices[0]];
            rep.target_name = ""; // project-level
            std::string target_list;
            size_t shown = 0;
            for (auto& t : targets) {
                if (!t.empty()) {
                    if (!target_list.empty()) target_list += ", ";
                    target_list += t;
                    if (++shown == 3 && targets.size() > 4) {
                        target_list += ", ...";
                        break;
                    }
                }
            }
            rep.recommendation += " [" + std::to_string(targets.size())
                + " targets: " + target_list + "]";

            collapsed.push_back(std::move(rep));
            for (auto idx : indices) to_remove.insert(idx);
        }

        if (!to_remove.empty()) {
            std::vector<Finding> new_findings;
            for (size_t i = 0; i < result.findings.size(); ++i) {
                if (to_remove.count(i) == 0) new_findings.push_back(result.findings[i]);
            }
            for (auto& c : collapsed) new_findings.push_back(std::move(c));
            result.findings = std::move(new_findings);
        }
    }

    // ─── Count severities ──────────────────────────────────────────────
    for (auto& f : result.findings) {
        switch (f.severity) {
            case FindingSeverity::Warning: result.warning_count++; break;
            case FindingSeverity::Error:   result.error_count++; break;
            case FindingSeverity::Info:    result.info_count++; break;
        }
    }

    log_info("Analysis complete: "
        + std::to_string(result.error_count) + " errors, "
        + std::to_string(result.warning_count) + " warnings, "
        + std::to_string(result.info_count) + " info, "
        + std::to_string(result.edges.size()) + " dependency edges");

    return result;
}

} // namespace anaport
