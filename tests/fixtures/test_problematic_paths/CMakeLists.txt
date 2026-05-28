cmake_minimum_required(VERSION 3.16)
project(ProblematicPaths VERSION 1.0 LANGUAGES CXX)

# Export compile_commands.json so the new parser can read it
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Simulated "vendor SDK" installed in non-standard paths ────────────────────
# These directories are intentionally hard-coded to trigger portability warnings.
# In a real project, these would be found via find_package() or find_library().
# The interface targets below do NOT require the directories to exist on disk.

add_library(vendor_sdk INTERFACE)
target_include_directories(vendor_sdk INTERFACE
    /opt/vendor/include         # non-standard vendor include path
    /usr/local/include/mylib    # non-standard local install
    /home/user/sdk/include      # user-specific absolute path (very non-portable)
)
target_compile_definitions(vendor_sdk INTERFACE
    VENDOR_SDK_LINUX=1          # Linux-specific define
    __linux__=1                 # Redefining platform macro (very bad practice)
    USE_POSIX_THREADS=1
)

# ── Application with hard-coded link path ─────────────────────────────────────
add_executable(problematic_app
    src/main.cpp
)

target_include_directories(problematic_app PRIVATE
    /opt/vendor/include         # Should be flagged: hard-coded system path
    /usr/local/lib/mylib        # Should be flagged: suspicious path in includes
)

target_compile_definitions(problematic_app PRIVATE
    PLATFORM_LINUX=1
    BUILD_HOST="myworkstation"  # Build-host baked in (not portability per se, but suspicious)
)

target_compile_options(problematic_app PRIVATE
    -march=native               # Non-portable architecture flag
    -mtune=generic
)

# Simulate linking against platform-specific libraries
# Using imported UNKNOWN target so no actual file is needed
add_library(fake_pthread STATIC IMPORTED)
set_target_properties(fake_pthread PROPERTIES
    IMPORTED_LOCATION /usr/lib/x86_64-linux-gnu/libpthread.a
)

add_library(fake_dl STATIC IMPORTED)
set_target_properties(fake_dl PROPERTIES
    IMPORTED_LOCATION /usr/local/lib/libdl.a
)

target_link_libraries(problematic_app PRIVATE
    vendor_sdk
    fake_pthread
    fake_dl
)
