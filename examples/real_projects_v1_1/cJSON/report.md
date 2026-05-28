# Portability Analysis Report

**Project:** cJSON  
**CMake version:** 3.31.6  
**Generated at:** 2026-05-26T22:02:27Z  
**Tool:** analyze-portability v1.1.0  

---

## Summary

| Metric | Count |
|--------|-------|
| Targets analyzed | 24 |
| Total findings | 26 |
| Emitted findings | 26 |
| Errors | 0 |
| Warnings | 21 |
| Info | 5 |
| Dependency edges | 57 |

### Findings by Category

| Category | Count |
|----------|-------|
| cache_variable | 2 |
| compile_command | 2 |
| platform_specific_library | 21 |
| runtime_path | 1 |

### Findings by Source

| Source | Count |
|--------|-------|
| cmake_cache | 2 |
| compile_commands | 2 |
| file_api | 22 |

### Data Sources

| Source | Found | Parsed | Details |
|--------|-------|--------|---------|
| CMake File API | yes | yes | — |
| compile_commands.json | yes | yes | 23 entries, 117 observations |
| CMakeCache.txt | yes | yes | 203 total variables, 54 interesting |

## Targets

| Name | Type | Language(s) | Sources |
|------|------|-------------|--------|
| `cJSON_test` | EXECUTABLE | C | 1 |
| `check` | UTILITY | — | 2 |
| `cjson` | SHARED_LIBRARY | C | 2 |
| `cjson_add` | EXECUTABLE | C | 1 |
| `compare_tests` | EXECUTABLE | C | 1 |
| `fuzz_main` | EXECUTABLE | C | 2 |
| `minify_tests` | EXECUTABLE | C | 1 |
| `misc_tests` | EXECUTABLE | C | 1 |
| `parse_array` | EXECUTABLE | C | 1 |
| `parse_examples` | EXECUTABLE | C | 1 |
| `parse_hex4` | EXECUTABLE | C | 1 |
| `parse_number` | EXECUTABLE | C | 1 |
| `parse_object` | EXECUTABLE | C | 1 |
| `parse_string` | EXECUTABLE | C | 1 |
| `parse_value` | EXECUTABLE | C | 1 |
| `parse_with_opts` | EXECUTABLE | C | 1 |
| `print_array` | EXECUTABLE | C | 1 |
| `print_number` | EXECUTABLE | C | 1 |
| `print_object` | EXECUTABLE | C | 1 |
| `print_string` | EXECUTABLE | C | 1 |
| `print_value` | EXECUTABLE | C | 1 |
| `readme_examples` | EXECUTABLE | C | 1 |
| `uninstall` | UTILITY | — | 2 |
| `unity` | STATIC_LIBRARY | C | 1 |

## Dependency Graph (edges)

| From | To | Kind |
|------|----|------|
| `cJSON_test` | `cjson` | internal |
| `check` | `print_number` | internal |
| `check` | `minify_tests` | internal |
| `check` | `parse_array` | internal |
| `check` | `cJSON_test` | internal |
| `check` | `parse_hex4` | internal |
| `check` | `cjson_add` | internal |
| `check` | `readme_examples` | internal |
| `check` | `parse_number` | internal |
| `check` | `parse_value` | internal |
| `check` | `parse_object` | internal |
| `check` | `print_string` | internal |
| `check` | `parse_with_opts` | internal |
| `check` | `compare_tests` | internal |
| `check` | `print_value` | internal |
| `check` | `misc_tests` | internal |
| `check` | `print_array` | internal |
| `check` | `print_object` | internal |
| `check` | `parse_string` | internal |
| `check` | `parse_examples` | internal |
| `cjson_add` | `unity` | internal |
| `cjson_add` | `cjson` | internal |
| `compare_tests` | `unity` | internal |
| `compare_tests` | `cjson` | internal |
| `fuzz_main` | `cjson` | internal |
| `minify_tests` | `unity` | internal |
| `minify_tests` | `cjson` | internal |
| `misc_tests` | `unity` | internal |
| `misc_tests` | `cjson` | internal |
| `parse_array` | `unity` | internal |
| `parse_array` | `cjson` | internal |
| `parse_examples` | `unity` | internal |
| `parse_examples` | `cjson` | internal |
| `parse_hex4` | `unity` | internal |
| `parse_hex4` | `cjson` | internal |
| `parse_number` | `unity` | internal |
| `parse_number` | `cjson` | internal |
| `parse_object` | `unity` | internal |
| `parse_object` | `cjson` | internal |
| `parse_string` | `unity` | internal |
| `parse_string` | `cjson` | internal |
| `parse_value` | `unity` | internal |
| `parse_value` | `cjson` | internal |
| `parse_with_opts` | `unity` | internal |
| `parse_with_opts` | `cjson` | internal |
| `print_array` | `unity` | internal |
| `print_array` | `cjson` | internal |
| `print_number` | `unity` | internal |
| `print_number` | `cjson` | internal |
| `print_object` | `unity` | internal |
| `print_object` | `cjson` | internal |
| `print_string` | `unity` | internal |
| `print_string` | `cjson` | internal |
| `print_value` | `unity` | internal |
| `print_value` | `cjson` | internal |
| `readme_examples` | `unity` | internal |
| `readme_examples` | `cjson` | internal |

## Portability Findings

### File API (CMake Targets)

#### Target: `(project-level)`

- *INFO* [runtime_path] **Field:** `link_fragment` — **Value:** `-Wl,-rpath,/tmp/anaport_IoCkSE`
  - Build-tree RPATH detected: -Wl,-rpath,/tmp/anaport_IoCkSE
  - _Recommendation:_ This RPATH points inside the build/source directory and is typical for build-tree executables. Verify that CMAKE_INSTALL_RPATH is set correctly for installed binaries. [20 targets: cJSON_test, cjson_add, compare_tests, ...]

#### Target: `cJSON_test`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `cjson`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `cjson_add`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `compare_tests`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `fuzz_main`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `minify_tests`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `misc_tests`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `parse_array`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `parse_examples`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `parse_hex4`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `parse_number`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `parse_object`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `parse_string`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `parse_value`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `parse_with_opts`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `print_array`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `print_number`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `print_object`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `print_string`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `print_value`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `readme_examples`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

### compile_commands.json

#### Target: `(project-level)`

- *INFO* [compile_command] **Field:** `compile_flag` — **Value:** `-fPIC`
  - Non-portable compile flag in compile_commands.json: -fPIC
  - _Recommendation:_ Check portability on all target platforms.

- *INFO* [compile_command] **Field:** `compile_flag` — **Value:** `-fstack-protector-strong`
  - Non-portable compile flag in compile_commands.json: -fstack-protector-strong
  - _Recommendation:_ Check portability on all target platforms.

### CMakeCache.txt

#### Target: `(project-level)`

- *INFO* [cache_variable] **Field:** `cmake_cache_variable` — **Value:** `CMAKE_C_COMPILER=/usr/bin/cc`
  - Compiler: CMAKE_C_COMPILER = /usr/bin/cc
  - _Recommendation:_ Ensure this compiler is available on all target build environments.

- *INFO* [cache_variable] **Field:** `cmake_cache_variable` — **Value:** `CMAKE_INSTALL_PREFIX=/usr/local`
  - Install prefix: /usr/local
  - _Recommendation:_ CMAKE_INSTALL_PREFIX is /usr/local. Ensure this is appropriate for all target environments.

---

*Generated by analyze-portability v1.1.0 — a CMake portability analysis tool for ВКР.*
