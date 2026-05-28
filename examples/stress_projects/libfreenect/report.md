# Portability Analysis Report

**Project:** libfreenect  
**CMake version:** 3.31.6  
**Generated at:** 2026-05-26T22:57:07Z  
**Tool:** analyze-portability v1.1.0  

---

## Summary

| Metric | Count |
|--------|-------|
| Targets analyzed | 11 |
| Total findings | 29 |
| Emitted findings | 29 |
| Errors | 0 |
| Warnings | 14 |
| Info | 15 |
| Dependency edges | 9 |

### Findings by Category

| Category | Count |
|----------|-------|
| absolute_path | 15 |
| cache_variable | 5 |
| compile_command | 1 |
| platform_specific_library | 7 |
| runtime_path | 1 |

### Findings by Source

| Source | Count |
|--------|-------|
| cmake_cache | 5 |
| compile_commands | 5 |
| file_api | 19 |

### Data Sources

| Source | Found | Parsed | Details |
|--------|-------|--------|---------|
| CMake File API | yes | yes | — |
| compile_commands.json | yes | yes | 27 entries, 87 observations |
| CMakeCache.txt | yes | yes | 226 total variables, 91 interesting |

## Targets

| Name | Type | Language(s) | Sources |
|------|------|-------------|--------|
| `fakenect` | SHARED_LIBRARY | C | 3 |
| `fakenect-record` | EXECUTABLE | C | 2 |
| `freenect` | SHARED_LIBRARY | C | 8 |
| `freenect-camtest` | EXECUTABLE | C | 1 |
| `freenect-regtest` | EXECUTABLE | C | 1 |
| `freenect-tiltdemo` | EXECUTABLE | C | 1 |
| `freenect-wavrecord` | EXECUTABLE | C | 1 |
| `freenect_sync` | SHARED_LIBRARY | C | 1 |
| `freenect_sync_static` | STATIC_LIBRARY | C | 1 |
| `freenectstatic` | STATIC_LIBRARY | C | 8 |
| `uninstall` | UTILITY | — | 2 |

## Dependency Graph (edges)

| From | To | Kind |
|------|----|------|
| `fakenect-record` | `freenect` | internal |
| `freenect-camtest` | `freenect` | internal |
| `freenect-regtest` | `freenect` | internal |
| `freenect-regtest` | `freenect_sync` | internal |
| `freenect-tiltdemo` | `freenect` | internal |
| `freenect-tiltdemo` | `freenect_sync` | internal |
| `freenect-wavrecord` | `freenect` | internal |
| `freenect_sync` | `freenect` | internal |
| `freenect_sync_static` | `freenect` | internal |

## Portability Findings

### File API (CMake Targets)

#### Target: `(project-level)`

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/home/user/workspace/portability_stress_projects/libfreenect/include`
  - Project-local absolute include path (resolved BUILD_INTERFACE): /home/user/workspace/portability_stress_projects/libfreenect/include
  - _Recommendation:_ This is typical for BUILD_INTERFACE generator expressions and is generally fine. [10 targets: fakenect, fakenect-record, freenect, ...]

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/home/user/workspace/portability_stress_projects/libfreenect/fakenect/../src`
  - Project-local absolute include path (resolved BUILD_INTERFACE): /home/user/workspace/portability_stress_projects/libfreenect/fakenect/../src
  - _Recommendation:_ This is typical for BUILD_INTERFACE generator expressions and is generally fine. [2 targets: fakenect, fakenect-record]

- *INFO* [runtime_path] **Field:** `link_fragment` — **Value:** `-Wl,-rpath,/tmp/anaport_NcePzG/lib:`
  - Build-tree RPATH detected: -Wl,-rpath,/tmp/anaport_NcePzG/lib:
  - _Recommendation:_ This RPATH points inside the build/source directory and is typical for build-tree executables. Verify that CMAKE_INSTALL_RPATH is set correctly for installed binaries. [6 targets: fakenect-record, freenect-camtest, freenect-regtest, ...]

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/home/user/workspace/portability_stress_projects/libfreenect/src`
  - Project-local absolute include path (resolved BUILD_INTERFACE): /home/user/workspace/portability_stress_projects/libfreenect/src
  - _Recommendation:_ This is typical for BUILD_INTERFACE generator expressions and is generally fine. [2 targets: freenect, freenectstatic]

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/home/user/workspace/portability_stress_projects/libfreenect/examples/../wrappers/c_sync`
  - Project-local absolute include path (resolved BUILD_INTERFACE): /home/user/workspace/portability_stress_projects/libfreenect/examples/../wrappers/c_sync
  - _Recommendation:_ This is typical for BUILD_INTERFACE generator expressions and is generally fine. [4 targets: freenect-camtest, freenect-regtest, freenect-tiltdemo, freenect-wavrecord]

#### Target: `fakenect`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `fakenect-record`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

- **WARNING** [absolute_path] **Field:** `link_library` — **Value:** `/usr/lib/x86_64-linux-gnu/libusb-1.0.so`
  - Absolute path to library (possibly project-local): /usr/lib/x86_64-linux-gnu/libusb-1.0.so
  - _Recommendation:_ Ensure this path is valid on all target systems.

#### Target: `freenect`

- **WARNING** [absolute_path] **Field:** `link_library` — **Value:** `/usr/lib/x86_64-linux-gnu/libusb-1.0.so`
  - Absolute path to library (possibly project-local): /usr/lib/x86_64-linux-gnu/libusb-1.0.so
  - _Recommendation:_ Ensure this path is valid on all target systems.

#### Target: `freenect-camtest`

- **WARNING** [absolute_path] **Field:** `link_library` — **Value:** `/usr/lib/x86_64-linux-gnu/libusb-1.0.so`
  - Absolute path to library (possibly project-local): /usr/lib/x86_64-linux-gnu/libusb-1.0.so
  - _Recommendation:_ Ensure this path is valid on all target systems.

#### Target: `freenect-regtest`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

- **WARNING** [absolute_path] **Field:** `link_library` — **Value:** `/usr/lib/x86_64-linux-gnu/libusb-1.0.so`
  - Absolute path to library (possibly project-local): /usr/lib/x86_64-linux-gnu/libusb-1.0.so
  - _Recommendation:_ Ensure this path is valid on all target systems.

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lpthread`
  - Platform-specific or OS-dependent library: pthread
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `freenect-tiltdemo`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lm`
  - Platform-specific or OS-dependent library: m
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

- **WARNING** [absolute_path] **Field:** `link_library` — **Value:** `/usr/lib/x86_64-linux-gnu/libusb-1.0.so`
  - Absolute path to library (possibly project-local): /usr/lib/x86_64-linux-gnu/libusb-1.0.so
  - _Recommendation:_ Ensure this path is valid on all target systems.

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lpthread`
  - Platform-specific or OS-dependent library: pthread
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

#### Target: `freenect-wavrecord`

- **WARNING** [absolute_path] **Field:** `link_library` — **Value:** `/usr/lib/x86_64-linux-gnu/libusb-1.0.so`
  - Absolute path to library (possibly project-local): /usr/lib/x86_64-linux-gnu/libusb-1.0.so
  - _Recommendation:_ Ensure this path is valid on all target systems.

#### Target: `freenect_sync`

- **WARNING** [platform_specific_library] **Field:** `link_library` — **Value:** `-lpthread`
  - Platform-specific or OS-dependent library: pthread
  - _Recommendation:_ Wrap in platform checks (if(UNIX), if(APPLE), if(WIN32)) or use find_package(Threads), find_library(), etc.

- **WARNING** [absolute_path] **Field:** `link_library` — **Value:** `/usr/lib/x86_64-linux-gnu/libusb-1.0.so`
  - Absolute path to library (possibly project-local): /usr/lib/x86_64-linux-gnu/libusb-1.0.so
  - _Recommendation:_ Ensure this path is valid on all target systems.

### compile_commands.json

#### Target: `(project-level)`

- *INFO* [compile_command] **Field:** `compile_flag` — **Value:** `-fPIC`
  - Non-portable compile flag in compile_commands.json: -fPIC
  - _Recommendation:_ Check portability on all target platforms.

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/home/user/workspace/portability_stress_projects/libfreenect/examples/../wrappers/c_sync`
  - Project-local include path in compile_commands.json: /home/user/workspace/portability_stress_projects/libfreenect/examples/../wrappers/c_sync
  - _Recommendation:_ This path is within the project source or build tree and is expected.

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/home/user/workspace/portability_stress_projects/libfreenect/fakenect/../src`
  - Project-local include path in compile_commands.json: /home/user/workspace/portability_stress_projects/libfreenect/fakenect/../src
  - _Recommendation:_ This path is within the project source or build tree and is expected.

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/home/user/workspace/portability_stress_projects/libfreenect/include`
  - Project-local include path in compile_commands.json: /home/user/workspace/portability_stress_projects/libfreenect/include
  - _Recommendation:_ This path is within the project source or build tree and is expected.

- *INFO* [absolute_path] **Field:** `include_path` — **Value:** `/home/user/workspace/portability_stress_projects/libfreenect/src`
  - Project-local include path in compile_commands.json: /home/user/workspace/portability_stress_projects/libfreenect/src
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

- *INFO* [cache_variable] **Field:** `cmake_cache_variable` — **Value:** `LIBUSB_1_INCLUDE_DIR=/usr/include/libusb-1.0`
  - Absolute path variable in CMakeCache: LIBUSB_1_INCLUDE_DIR = /usr/include/libusb-1.0
  - _Recommendation:_ Verify this path is consistent across build environments.

- *INFO* [cache_variable] **Field:** `cmake_cache_variable` — **Value:** `LIBUSB_1_LIBRARY=/usr/lib/x86_64-linux-gnu/libusb-1.0.so`
  - Absolute path variable in CMakeCache: LIBUSB_1_LIBRARY = /usr/lib/x86_64-linux-gnu/libusb-1.0.so
  - _Recommendation:_ Verify this path is consistent across build environments.

---

*Generated by analyze-portability v1.1.0 — a CMake portability analysis tool for ВКР.*
