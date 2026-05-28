# Portability Analysis Report

**Project:** FMT  
**CMake version:** 3.31.6  
**Generated at:** 2026-05-26T22:02:28Z  
**Tool:** analyze-portability v1.1.0  

---

## Summary

| Metric | Count |
|--------|-------|
| Targets analyzed | 26 |
| Total findings | 7 |
| Emitted findings | 7 |
| Errors | 0 |
| Warnings | 0 |
| Info | 7 |
| Dependency edges | 56 |

### Findings by Category

| Category | Count |
|----------|-------|
| absolute_path | 4 |
| cache_variable | 3 |

### Findings by Source

| Source | Count |
|--------|-------|
| cmake_cache | 3 |
| compile_commands | 2 |
| file_api | 2 |

### Data Sources

| Source | Found | Parsed | Details |
|--------|-------|--------|---------|
| CMake File API | yes | yes | — |
| compile_commands.json | yes | yes | 51 entries, 158 observations |
| CMakeCache.txt | yes | yes | 212 total variables, 76 interesting |

## Targets

| Name | Type | Language(s) | Sources |
|------|------|-------------|--------|
| `args-test` | EXECUTABLE | CXX | 1 |
| `assert-test` | EXECUTABLE | CXX | 1 |
| `base-test` | EXECUTABLE | CXX | 1 |
| `c-test` | EXECUTABLE | C | 1 |
| `chrono-test` | EXECUTABLE | CXX | 1 |
| `color-test` | EXECUTABLE | CXX | 1 |
| `compile-test` | EXECUTABLE | CXX | 1 |
| `enforce-checks-test` | EXECUTABLE | CXX | 1 |
| `fmt` | STATIC_LIBRARY | CXX | 18 |
| `fmt-c` | STATIC_LIBRARY | CXX | 1 |
| `format-impl-test` | EXECUTABLE | CXX | 7 |
| `format-test` | EXECUTABLE | CXX | 2 |
| `gtest` | STATIC_LIBRARY | CXX | 4 |
| `gtest-extra-test` | EXECUTABLE | CXX | 1 |
| `no-builtin-types-test` | EXECUTABLE | CXX | 6 |
| `os-test` | EXECUTABLE | CXX | 1 |
| `ostream-test` | EXECUTABLE | CXX | 1 |
| `perf-sanity` | EXECUTABLE | CXX | 1 |
| `posix-mock-test` | EXECUTABLE | CXX | 6 |
| `printf-test` | EXECUTABLE | CXX | 1 |
| `ranges-test` | EXECUTABLE | CXX | 2 |
| `scan-test` | EXECUTABLE | CXX | 6 |
| `std-test` | EXECUTABLE | CXX | 1 |
| `test-main` | STATIC_LIBRARY | CXX | 4 |
| `unicode-test` | EXECUTABLE | CXX | 6 |
| `xchar-test` | EXECUTABLE | CXX | 1 |

## Dependency Graph (edges)

| From | To | Kind |
|------|----|------|
| `args-test` | `fmt` | internal |
| `args-test` | `test-main` | internal |
| `args-test` | `gtest` | internal |
| `assert-test` | `fmt` | internal |
| `assert-test` | `test-main` | internal |
| `assert-test` | `gtest` | internal |
| `base-test` | `fmt` | internal |
| `base-test` | `test-main` | internal |
| `base-test` | `gtest` | internal |
| `c-test` | `fmt` | internal |
| `c-test` | `fmt-c` | internal |
| `chrono-test` | `fmt` | internal |
| `chrono-test` | `test-main` | internal |
| `chrono-test` | `gtest` | internal |
| `color-test` | `fmt` | internal |
| `color-test` | `test-main` | internal |
| `color-test` | `gtest` | internal |
| `compile-test` | `fmt` | internal |
| `compile-test` | `test-main` | internal |
| `compile-test` | `gtest` | internal |
| `enforce-checks-test` | `fmt` | internal |
| `enforce-checks-test` | `test-main` | internal |
| `enforce-checks-test` | `gtest` | internal |
| `fmt-c` | `fmt` | internal |
| `format-impl-test` | `gtest` | internal |
| `format-test` | `fmt` | internal |
| `format-test` | `test-main` | internal |
| `format-test` | `gtest` | internal |
| `gtest-extra-test` | `fmt` | internal |
| `gtest-extra-test` | `test-main` | internal |
| `gtest-extra-test` | `gtest` | internal |
| `no-builtin-types-test` | `gtest` | internal |
| `os-test` | `fmt` | internal |
| `os-test` | `test-main` | internal |
| `os-test` | `gtest` | internal |
| `ostream-test` | `fmt` | internal |
| `ostream-test` | `test-main` | internal |
| `ostream-test` | `gtest` | internal |
| `perf-sanity` | `fmt` | internal |
| `posix-mock-test` | `gtest` | internal |
| `printf-test` | `fmt` | internal |
| `printf-test` | `test-main` | internal |
| `printf-test` | `gtest` | internal |
| `ranges-test` | `fmt` | internal |
| `ranges-test` | `test-main` | internal |
| `ranges-test` | `gtest` | internal |
| `scan-test` | `gtest` | internal |
| `std-test` | `fmt` | internal |
| `std-test` | `test-main` | internal |
| `std-test` | `gtest` | internal |
| `test-main` | `fmt` | internal |
| `test-main` | `gtest` | internal |
| `unicode-test` | `gtest` | internal |
| `xchar-test` | `fmt` | internal |
| `xchar-test` | `test-main` | internal |
| `xchar-test` | `gtest` | internal |

## Portability Findings

### File API (CMake Targets)

#### Target: `(project-level)`

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/home/user/workspace/real_cmake_projects/fmt/test/gtest/.`
  - Project-local absolute include path (resolved BUILD_INTERFACE): /home/user/workspace/real_cmake_projects/fmt/test/gtest/.
  - _Recommendation:_ This is typical for BUILD_INTERFACE generator expressions and is generally fine. [22 targets: args-test, assert-test, base-test, ...]

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/home/user/workspace/real_cmake_projects/fmt/include`
  - Project-local absolute include path (resolved BUILD_INTERFACE): /home/user/workspace/real_cmake_projects/fmt/include
  - _Recommendation:_ This is typical for BUILD_INTERFACE generator expressions and is generally fine. [25 targets: args-test, assert-test, base-test, ...]

### compile_commands.json

#### Target: `(project-level)`

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/home/user/workspace/real_cmake_projects/fmt/include`
  - Project-local include path in compile_commands.json: /home/user/workspace/real_cmake_projects/fmt/include
  - _Recommendation:_ This path is within the project source or build tree and is expected.

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/home/user/workspace/real_cmake_projects/fmt/test/gtest/.`
  - Project-local include path in compile_commands.json: /home/user/workspace/real_cmake_projects/fmt/test/gtest/.
  - _Recommendation:_ This path is within the project source or build tree and is expected.

### CMakeCache.txt

#### Target: `(project-level)`

- *INFO* [cache_variable] **Field:** `cmake_cache_variable` — **Value:** `CMAKE_CXX_COMPILER=/usr/bin/c++`
  - Compiler: CMAKE_CXX_COMPILER = /usr/bin/c++
  - _Recommendation:_ Ensure this compiler is available on all target build environments.

- *INFO* [cache_variable] **Field:** `cmake_cache_variable` — **Value:** `CMAKE_C_COMPILER=/usr/bin/cc`
  - Compiler: CMAKE_C_COMPILER = /usr/bin/cc
  - _Recommendation:_ Ensure this compiler is available on all target build environments.

- *INFO* [cache_variable] **Field:** `cmake_cache_variable` — **Value:** `CMAKE_INSTALL_PREFIX=/usr/local`
  - Install prefix: /usr/local
  - _Recommendation:_ CMAKE_INSTALL_PREFIX is /usr/local. Ensure this is appropriate for all target environments.

---

*Generated by analyze-portability v1.1.0 — a CMake portability analysis tool for ВКР.*
