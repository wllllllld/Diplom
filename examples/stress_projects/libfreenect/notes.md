# libfreenect — Notes on Portability Analysis

**Repository:** https://github.com/OpenKinect/libfreenect  
**Project version:** 0.7.5  
**Languages:** C  
**Analysis tool:** analyze-portability v1.1.0  
**Analysis date:** 2026-05-26  

---

## Configure Status

**SUCCESS** — cmake configure completed with exit code 0. Full analysis reports generated:
- `report.json`, `report.md`, `dependencies.dot`

The project configured on Ubuntu with `libusb-1.0-0-dev` installed.
OpenGL/GLUT examples were skipped (not found), which is expected.

---

## Why This Project Is Interesting for Portability

libfreenect is an open-source driver for the Xbox Kinect sensor. It claims to support
Windows, Linux, and macOS, but its portability is nuanced:

1. **Hardware-specific USB protocol** — The library depends on `libusb-1.0`, which is
   cross-platform but requires different udev rules / driver setup per OS.

2. **Hardcoded system library paths** — On Linux with the standard libusb install, the tool
   correctly detects that the linker fragments contain absolute paths like
   `/usr/lib/x86_64-linux-gnu/libusb-1.0.so`. This is a genuine portability issue:
   the library path is architecture-specific (x86_64) and would differ on ARM, aarch64, etc.

3. **pthread explicit linking** — Several targets link `-lpthread` directly rather than using
   `find_package(Threads)` + `Threads::Threads`. The tool detected this as `platform_specific_library`.

4. **Math library** — Multiple targets link `-lm` directly rather than using `target_link_libraries`
   with the portable `m` approach.

5. **RPATH in build tree** — The tool detected `-Wl,-rpath,/tmp/anaport_.../lib:` in linker
   fragments, flagging that `CMAKE_INSTALL_RPATH` should be verified for install.

6. **Platform detection module** — The project uses a custom `FindOS.cmake` module that
   detects Linux, macOS, and Windows separately and sets platform-specific compile flags.
   While this is done correctly (with `if(PROJECT_OS_LINUX)` guards), it's an older approach
   compared to CMake's built-in `CMAKE_SYSTEM_NAME`.

---

## Tool Findings Summary

| Category | Count | Severity |
|----------|-------|----------|
| absolute_path (libusb SO path) | 7 | warning |
| platform_specific_library (pthread, m) | 7 | warning |
| runtime_path (build-tree RPATH) | 1 | info |
| absolute_path (project-local includes) | 4 | info |
| cache_variable (compiler, install, libusb paths) | 5 | info |
| compile_command (-fPIC) | 1 | info |
| **TOTAL** | **29** | 0 errors, 14 warnings, 15 info |

---

## What the Tool Understood Correctly

- Detected all 11 targets (10 build targets + 1 UTILITY)
- Correctly identified `uninstall` as UTILITY target (excluded from main analysis)
- Detected the absolute `/usr/lib/x86_64-linux-gnu/libusb-1.0.so` path in link fragments
  as a portability concern (correct: this path is distro/arch specific)
- Detected `-lpthread` and `-lm` as platform-specific library links (correct)
- Built the full 9-edge dependency graph (freenect → freenect_sync → example targets)
- Read compile_commands.json (27 entries, 87 observations) and CMakeCache.txt correctly

---

## Tool Limitations Observed

- The tool flags project-local absolute include paths (e.g., `/home/user/workspace/portability_stress_projects/libfreenect/include`) as `absolute_path` at INFO level. This is correct behavior (they are BUILD_INTERFACE paths), but the info messages add noise.
- The tool cannot analyze the `FindOS.cmake` or platform-conditional CMake logic inside `CMakeLists.txt`. It only sees what CMake resolved for the current platform (Linux). The Windows-specific code paths (`if(PROJECT_OS_WIN32)`) are invisible to the analysis.
- `-fPIC` is flagged as a "non-portable compile flag" at INFO level, which is debatable — it's standard for shared libraries on Linux/macOS.

---

## Overall Assessment

libfreenect is a **moderately portable** project — it genuinely supports multiple platforms via
conditional CMake logic, but has concrete portability issues in how it expresses library paths
and platform-specific link dependencies. The tool correctly detected the most impactful issues
(absolute libusb path, direct pthread/m linking) and provided actionable recommendations.
