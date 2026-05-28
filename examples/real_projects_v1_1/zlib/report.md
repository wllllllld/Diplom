# Portability Analysis Report

**Project:** zlib  
**CMake version:** 3.31.6  
**Generated at:** 2026-05-26T22:02:30Z  
**Tool:** analyze-portability v1.1.0  

---

## Summary

| Metric | Count |
|--------|-------|
| Targets analyzed | 9 |
| Total findings | 12 |
| Emitted findings | 12 |
| Errors | 0 |
| Warnings | 4 |
| Info | 8 |
| Dependency edges | 7 |

### Findings by Category

| Category | Count |
|----------|-------|
| absolute_path | 4 |
| cache_variable | 2 |
| compile_command | 1 |
| platform_specific_library | 4 |
| runtime_path | 1 |

### Findings by Source

| Source | Count |
|--------|-------|
| cmake_cache | 2 |
| compile_commands | 3 |
| file_api | 7 |

### Data Sources

| Source | Found | Parsed | Details |
|--------|-------|--------|---------|
| CMake File API | yes | yes | — |
| compile_commands.json | yes | yes | 37 entries, 133 observations |
| CMakeCache.txt | yes | yes | 215 total variables, 55 interesting |

## Targets

| Name | Type | Language(s) | Sources |
|------|------|-------------|--------|
| `infcover` | EXECUTABLE | C | 1 |
| `minigzip` | EXECUTABLE | C | 1 |
| `static_minigzip` | EXECUTABLE | C | 1 |
| `zlib` | SHARED_LIBRARY | C | 26 |
| `zlib_example` | EXECUTABLE | C | 1 |
| `zlib_example64` | EXECUTABLE | C | 1 |
| `zlib_static_example` | EXECUTABLE | C | 1 |
| `zlib_static_example64` | EXECUTABLE | C | 1 |
| `zlibstatic` | STATIC_LIBRARY | C | 26 |

## Dependency Graph (edges)

| From | To | Kind |
|------|----|------|
| `infcover` | `zlibstatic` | internal |
| `minigzip` | `zlib` | internal |
| `static_minigzip` | `zlibstatic` | internal |
| `zlib_example` | `zlib` | internal |
| `zlib_example64` | `zlib` | internal |
| `zlib_static_example` | `zlibstatic` | internal |
| `zlib_static_example64` | `zlibstatic` | internal |

## Portability Findings

### File API (CMake Targets)

#### Target: `(project-level)`

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/home/user/workspace/real_cmake_projects/zlib`
  - Project-local absolute include path (resolved BUILD_INTERFACE): /home/user/workspace/real_cmake_projects/zlib
  - _Recommendation:_ This is typical for BUILD_INTERFACE generator expressions and is generally fine. [9 targets: infcover, minigzip, static_minigzip, ...]

- *INFO* [runtime_path] **Field:** `link_fragment` — **Value:** `-Wl,-rpath,/tmp/anaport_ZrPuHk`
  - Build-tree RPATH detected: -Wl,-rpath,/tmp/anaport_ZrPuHk
  - _Recommendation:_ This RPATH points inside the build/source directory and is typical for build-tree executables. Verify that CMAKE_INSTALL_RPATH is set correctly for installed binaries. [3 targets: minigzip, zlib_example, zlib_example64]

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/tmp/anaport_ZrPuHk`
  - Project-local absolute include path (resolved BUILD_INTERFACE): /tmp/anaport_ZrPuHk
  - _Recommendation:_ This is typical for BUILD_INTERFACE generator expressions and is generally fine. [9 targets: infcover, minigzip, static_minigzip, ...]

#### Target: `infcover`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `../libz.a`
  - Platform-specific or OS-dependent library: z
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `static_minigzip`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `../libz.a`
  - Platform-specific or OS-dependent library: z
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `zlib_static_example`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `../libz.a`
  - Platform-specific or OS-dependent library: z
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `zlib_static_example64`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `../libz.a`
  - Platform-specific or OS-dependent library: z
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

### compile_commands.json

#### Target: `(project-level)`

- *INFO* [compile_command] **Field:** `compile_flag` — **Value:** `-fPIC`
  - Non-portable compile flag in compile_commands.json: -fPIC
  - _Recommendation:_ Check portability on all target platforms.

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/home/user/workspace/real_cmake_projects/zlib`
  - Project-local include path in compile_commands.json: /home/user/workspace/real_cmake_projects/zlib
  - _Recommendation:_ This path is within the project source or build tree and is expected.

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/tmp/anaport_ZrPuHk`
  - Project-local include path in compile_commands.json: /tmp/anaport_ZrPuHk
  - _Recommendation:_ This path is within the project source or build tree and is expected.

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
