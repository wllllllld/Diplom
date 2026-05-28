# Portability Analysis Report

**Project:** libuvc  
**CMake version:** 3.31.6  
**Generated at:** 2026-05-26T22:57:12Z  
**Tool:** analyze-portability v1.1.0  

---

## Summary

| Metric | Count |
|--------|-------|
| Targets analyzed | 3 |
| Total findings | 12 |
| Emitted findings | 12 |
| Errors | 0 |
| Warnings | 2 |
| Info | 10 |
| Dependency edges | 1 |

### Findings by Category

| Category | Count |
|----------|-------|
| absolute_path | 6 |
| cache_variable | 4 |
| compile_command | 1 |
| runtime_path | 1 |

### Findings by Source

| Source | Count |
|--------|-------|
| cmake_cache | 4 |
| compile_commands | 3 |
| file_api | 5 |

### Data Sources

| Source | Found | Parsed | Details |
|--------|-------|--------|---------|
| CMake File API | yes | yes | — |
| compile_commands.json | yes | yes | 19 entries, 67 observations |
| CMakeCache.txt | yes | yes | 218 total variables, 61 interesting |

## Targets

| Name | Type | Language(s) | Sources |
|------|------|-------------|--------|
| `example` | EXECUTABLE | C | 1 |
| `uvc` | SHARED_LIBRARY | C | 9 |
| `uvc_static` | STATIC_LIBRARY | C | 9 |

## Dependency Graph (edges)

| From | To | Kind |
|------|----|------|
| `example` | `uvc` | internal |

## Portability Findings

### File API (CMake Targets)

#### Target: `(project-level)`

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/tmp/anaport_7ehI2W/include`
  - Project-local absolute include path (resolved BUILD_INTERFACE): /tmp/anaport_7ehI2W/include
  - _Recommendation:_ This is typical for BUILD_INTERFACE generator expressions and is generally fine. [3 targets: example, uvc, uvc_static]

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/home/user/workspace/portability_stress_projects/libuvc/include`
  - Project-local absolute include path (resolved BUILD_INTERFACE): /home/user/workspace/portability_stress_projects/libuvc/include
  - _Recommendation:_ This is typical for BUILD_INTERFACE generator expressions and is generally fine. [3 targets: example, uvc, uvc_static]

#### Target: `example`

- *INFO* [runtime_path] **Field:** `link_fragment` — **Value:** `-Wl,-rpath,/tmp/anaport_7ehI2W`
  - Build-tree RPATH detected: -Wl,-rpath,/tmp/anaport_7ehI2W
  - _Recommendation:_ This RPATH points inside the build/source directory and is typical for build-tree executables. Verify that CMAKE_INSTALL_RPATH is set correctly for installed binaries.

#### Target: `uvc`

- **WARNING** [absolute_path] **Field:** `link_library` — **Value:** `/usr/lib/x86_64-linux-gnu/libusb-1.0.so`
  - Absolute path to library (possibly project-local): /usr/lib/x86_64-linux-gnu/libusb-1.0.so
  - _Recommendation:_ Ensure this path is valid on all target systems.

- **WARNING** [absolute_path] **Field:** `link_library` — **Value:** `/usr/lib/x86_64-linux-gnu/libjpeg.so`
  - Absolute path to library (possibly project-local): /usr/lib/x86_64-linux-gnu/libjpeg.so
  - _Recommendation:_ Ensure this path is valid on all target systems.

### compile_commands.json

#### Target: `(project-level)`

- *INFO* [compile_command] **Field:** `compile_flag` — **Value:** `-fPIC`
  - Non-portable compile flag in compile_commands.json: -fPIC
  - _Recommendation:_ Check portability on all target platforms.

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/home/user/workspace/portability_stress_projects/libuvc/include`
  - Project-local include path in compile_commands.json: /home/user/workspace/portability_stress_projects/libuvc/include
  - _Recommendation:_ This path is within the project source or build tree and is expected.

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/tmp/anaport_7ehI2W/include`
  - Project-local include path in compile_commands.json: /tmp/anaport_7ehI2W/include
  - _Recommendation:_ This path is within the project source or build tree and is expected.

### CMakeCache.txt

#### Target: `(project-level)`

- *INFO* [cache_variable] **Field:** `cmake_cache_variable` — **Value:** `CMAKE_C_COMPILER=/usr/bin/cc`
  - Compiler: CMAKE_C_COMPILER = /usr/bin/cc
  - _Recommendation:_ Ensure this compiler is available on all target build environments.

- *INFO* [cache_variable] **Field:** `cmake_cache_variable` — **Value:** `CMAKE_INSTALL_PREFIX=/usr/local`
  - Install prefix: /usr/local
  - _Recommendation:_ CMAKE_INSTALL_PREFIX is /usr/local. Ensure this is appropriate for all target environments.

- *INFO* [cache_variable] **Field:** `cmake_cache_variable` — **Value:** `JPEG_INCLUDE_DIR=/usr/include`
  - Absolute path variable in CMakeCache: JPEG_INCLUDE_DIR = /usr/include
  - _Recommendation:_ Verify this path is consistent across build environments.

- *INFO* [cache_variable] **Field:** `cmake_cache_variable` — **Value:** `LibUSB_LIBRARY=/usr/lib/x86_64-linux-gnu/libusb-1.0.so`
  - Absolute path variable in CMakeCache: LibUSB_LIBRARY = /usr/lib/x86_64-linux-gnu/libusb-1.0.so
  - _Recommendation:_ Verify this path is consistent across build environments.

---

*Generated by analyze-portability v1.1.0 — a CMake portability analysis tool for ВКР.*
