cmake_minimum_required(VERSION 3.16)
project(analyze-portability
    VERSION 1.1.0
    DESCRIPTION "CMake portability analysis tool for C/C++ projects"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Dependencies ──────────────────────────────────────────────────────────────
find_package(nlohmann_json 3.2.0 REQUIRED)

# ── Library: anaport core ────────────────────────────────────────────────────
add_library(anaport_lib STATIC
    src/logger.cpp
    src/options.cpp
    src/workspace.cpp
    src/cmake_invoker.cpp
    src/file_api_parser.cpp
    src/compile_commands_parser.cpp
    src/cmake_cache_parser.cpp
    src/analyzer.cpp
    src/report_generator.cpp
)

target_include_directories(anaport_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(anaport_lib
    PUBLIC
        nlohmann_json::nlohmann_json
)

target_compile_options(anaport_lib PRIVATE
    -Wall -Wextra -Wpedantic
)

# ── Executable ────────────────────────────────────────────────────────────────
add_executable(analyze-portability
    src/main.cpp
)

target_link_libraries(analyze-portability
    PRIVATE
        anaport_lib
)

target_compile_options(analyze-portability PRIVATE
    -Wall -Wextra -Wpedantic
)

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS analyze-portability
    RUNTIME DESTINATION bin
)

# ── Summary ───────────────────────────────────────────────────────────────────
message(STATUS "")
message(STATUS "=== analyze-portability ===")
message(STATUS "  Version   : ${PROJECT_VERSION}")
message(STATUS "  C++ std   : ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Binary    : analyze-portability")
message(STATUS "")
