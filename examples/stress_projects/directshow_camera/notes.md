# directshow_camera — Notes on Portability Analysis

**Repository:** https://github.com/kcwongjoe/directshow_camera  
**CMake project version:** 2.0.0  
**Analysis tool:** analyze-portability v1.1.0  
**Analysis date:** 2026-05-26  

---

## Configure Status

**FAILED** — cmake configure exits with code 1 due to platform-specific Windows dependencies.

The tool handled this gracefully: it emitted `[WARN] cmake configure exited with code 1` and
`[ERR] CMake File API reply dir not found`, then exited cleanly (process exit code 0).
No reports (report.json, report.md, dependencies.dot) were generated because the File API
reply directory is never written by CMake when configure fails before all targets are registered.

The configure_or_tool.log captures the full stdout/stderr of both cmake and the tool itself.

---

## Why Configure Fails on Ubuntu/Linux

### Root causes (in order of failure)

1. **Windows-only OpenCV download** — `cmake/InstallOpenCV.cmake` unconditionally invokes
   `FetchContent_Declare(opencv URL https://github.com/opencv/opencv/releases/download/4.9.0/opencv-4.9.0-windows.exe)`.
   On Linux, cmake successfully downloads the Windows executable (≈280 MB), extracts it, but
   then `find_package(OpenCV REQUIRED ...)` fails because the extracted `OpenCVConfig.cmake`
   reports `OpenCV_FOUND=FALSE` — the Windows Pack has no binaries for Linux.

2. **Windows SDK dependency** — `cmake/AddWin10SDK.cmake` looks for `C:/Program Files (x86)/Windows Kits/10`
   and a config file `config/window_sdk_path.txt`. On Linux neither exists; it emits a message
   but does not `message(FATAL_ERROR ...)`, so configure continues.

3. **DirectShow/COM API in source** — `ds_header.h` includes `<dshow.h>`, `<vidcap.h>`, `<ksmedia.h>`
   with `#pragma comment(lib, "strmiids")` and `#pragma managed(push, off)`. These headers only
   exist in the Windows SDK and MSVC. There is no `#ifdef _WIN32` guard at the CMake level;
   the entire project assumes a Windows build environment unconditionally.

4. **MSVC-specific flags** — `CMakeLists.txt` sets `CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest"`.
   The `/std:c++latest` flag is MSVC syntax. GCC/Clang use `-std=c++20`. This flag is passed
   unconditionally on all platforms.

---

## Why This Project Is the Hardest for Portability

directshow_camera is an **intentionally Windows-only** project. Its portability score on Linux would be
the maximum possible (complete non-portability), but the tool cannot generate a numeric score because
configure never completes enough to produce a File API reply. The portability issues include:

| Issue | Severity | Category |
|-------|----------|----------|
| Hardcoded Windows-only FetchContent OpenCV URL (`.exe`) | Critical | platform_specific_cmake |
| DirectShow SDK headers (`dshow.h`, `vidcap.h`, `ksmedia.h`) | Critical | platform_specific_library |
| `#pragma comment(lib, "strmiids")` — MSVC-only pragma | Critical | compiler_flag |
| `#pragma managed(push, off)` — MSVC/C++/CLI only | Critical | compiler_flag |
| `/std:c++latest` — MSVC-only compiler flag | High | non_portable_compiler_flag |
| Windows SDK path search (`C:/Program Files (x86)/Windows Kits/10`) | High | hardcoded_system_path |
| No cross-platform abstraction layer or `if(WIN32)` guards in CMake | High | platform_specific_cmake |
| OpenCV downloaded as Windows binary package | High | platform_specific_cmake |

---

## Tool Behavior Assessment

### Expected behavior or tool deficiency?

This is **expected behavior** for the configure failure path. The tool:
- Ran cmake and captured output correctly
- Detected cmake exit code 1
- Emitted appropriate error messages
- Exited with process code 0 (did not crash)
- The log file (`configure_or_tool.log`) contains all relevant diagnostic information

### Limitation identified

The tool currently generates no reports (not even partial ones) when cmake configure fails,
even if some information is available (e.g., from CMakeCache.txt which IS written even on
partial failure). The CMakeCache.txt at `/tmp/anaport_*/CMakeCache.txt` contains variables
such as `FETCHCONTENT_BASE_DIR`, `directshow_camera_SOURCE_DIR` etc., which could be analyzed
even without the File API reply.

A future improvement would be to analyze CMakeCache.txt even when the File API reply is absent,
generating a degraded report. This would allow detecting issues like:
- The presence of `FETCHCONTENT_SOURCE_DIR_OPENCV` pointing to a Windows binary
- `CMAKE_CXX_FLAGS` containing `/std:c++latest`

### No code fix was applied

The configure failure is the expected result for a Windows-only project run on Linux. The tool's
graceful degradation (no crash, clear error messages, exit code 0) is correct behavior. No
changes to analyze-portability were needed.

---

## Summary

directshow_camera is **completely non-portable** to Linux/macOS. It is a pure Windows/DirectShow project
with no abstraction layer. The cmake build system is tightly coupled to Windows SDK, MSVC flags, and
Windows OpenCV binaries. This project correctly represents the "maximum difficulty" boundary for a
portability analysis tool — configure cannot succeed on Linux, so only the log is available.

The tool handled this gracefully and produced a useful diagnostic log.
