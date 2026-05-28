# libuvc — Notes on Portability Analysis

**Repository:** https://github.com/libuvc/libuvc  
**Project version:** 0.0.7  
**Languages:** C  
**Analysis tool:** analyze-portability v1.1.0  
**Analysis date:** 2026-05-26  

---

## Configure Status

**SUCCESS** — cmake configure completed with exit code 0. Full analysis reports generated:
- `report.json`, `report.md`, `dependencies.dot`

Dependencies found on Ubuntu: `libusb-1.0` (via pkg-config), `libjpeg-dev`.
The project declared itself as Release build (defaulted by CMakeLists.txt).

---

## Why This Project Was Selected

libuvc is a cross-platform C library for USB Video Class (UVC) devices — essentially
the Linux/macOS counterpart to Windows DirectShow for camera access. It is the most
directly comparable cross-platform alternative to directshow_camera and represents a
project that **intends** to be portable, using standard CMake patterns.

---

## Portability Characteristics

1. **Designed for portability** — libuvc explicitly uses `pkg-config` and `find_package`
   to locate libusb-1.0 and libjpeg. The CMakeLists.txt has proper `if(UNIX AND NOT APPLE)`
   guards for pthread linking.

2. **libusb absolute path in link fragment** — Despite using pkg-config, CMake still resolves
   the final link fragment to `/usr/lib/x86_64-linux-gnu/libusb-1.0.so` and
   `/usr/lib/x86_64-linux-gnu/libjpeg.so`. The tool correctly flags these as portability
   warnings because the x86_64-linux-gnu path is arch/distro specific.

3. **Build-tree RPATH** — The `example` executable has a build-tree RPATH
   (`-Wl,-rpath,/tmp/anaport_.../`). The tool correctly identifies this as an info-level
   finding (expected for build-tree, but install RPATH should be verified).

4. **Generated include in build dir** — `libuvc` generates `libuvc/libuvc_config.h` into
   the build directory during configure. This creates an include path to `/tmp/anaport_.../include`,
   which the tool flags at INFO level (expected build-interface path).

5. **Deprecated cmake_minimum_required** — Uses `cmake_minimum_required(VERSION 3.1)`, below
   the modern minimum. CMake 3.31 issued a deprecation warning.

---

## Tool Findings Summary

| Category | Count | Severity |
|----------|-------|----------|
| absolute_path (libusb + libjpeg SO path) | 2 | warning |
| runtime_path (build-tree RPATH) | 1 | info |
| absolute_path (project-local includes) | 3 | info |
| cache_variable (compiler, install, lib paths) | 4 | info |
| compile_command (-fPIC) | 1 | info |
| dependency_edges | 1 | — |
| **TOTAL** | **12** | 0 errors, 2 warnings, 10 info |

---

## What the Tool Understood Correctly

- Parsed all 3 targets: `uvc` (SHARED), `uvc_static` (STATIC), `example` (EXECUTABLE)
- Detected the `example → uvc` dependency edge correctly
- Correctly flagged absolute system library paths in link fragments (libusb, libjpeg)
- compile_commands.json parsed: 19 entries, 67 observations
- CMakeCache.txt: 218 variables, 61 interesting — captured `LibUSB_LIBRARY` and `JPEG_INCLUDE_DIR`

---

## Tool Limitations Observed

- The tool does not detect that `cmake_minimum_required(VERSION 3.1)` is deprecated —
  this is a CMake policy concern, not directly a portability issue, but it is relevant context.
- The `if(UNIX AND NOT APPLE)` Threads guard in CMakeLists.txt is correct CMake practice,
  but the tool cannot validate the logic — it only sees the resolved state on Linux.
- The pkg-config-based libusb find is the correct approach, but the tool still warns about
  the absolute path that pkg-config produces. This is technically correct (the path is
  arch-specific) but the recommendation to "use find_package()" is misleading since libuvc
  already does that — the issue is that CMake File API reports the resolved path, not the
  find_package() call.

---

## Comparison with directshow_camera

libuvc is the correct cross-platform pattern for camera access. Where directshow_camera is
completely non-portable (Windows-only APIs, no abstraction layer), libuvc achieves the same
goal (USB camera capture) while being genuinely cross-platform. The tool detected only 2
warnings for libuvc vs. being unable to analyze directshow_camera at all — this contrast
illustrates the tool's ability to differentiate portability levels.

---

## Overall Assessment

libuvc is a **highly portable** project. Its 2 warnings (absolute library paths from pkg-config)
are genuine but minor — they reflect the system state at configure time, not architectural
decisions. The tool correctly scored this project as nearly clean. This makes libuvc a good
**baseline/reference** project for the stress evaluation.
