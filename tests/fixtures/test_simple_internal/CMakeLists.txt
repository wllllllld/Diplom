cmake_minimum_required(VERSION 3.16)
project(SimpleInternal VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Internal static library
add_library(mathlib STATIC
    src/mathlib.cpp
)
target_include_directories(mathlib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Internal shared library
add_library(strutils SHARED
    src/strutils.cpp
)
target_include_directories(strutils
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Main executable
add_executable(simple_app
    src/main.cpp
)
target_link_libraries(simple_app
    PRIVATE
        mathlib
        strutils
)
