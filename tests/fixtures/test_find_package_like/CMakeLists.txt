cmake_minimum_required(VERSION 3.16)
project(FindPackageLike VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use find_package(Threads) — portable way to link pthreads
find_package(Threads REQUIRED)

# Interface library to simulate an external "Config-style" package
add_library(external_config INTERFACE)
target_compile_definitions(external_config INTERFACE
    EXTERNAL_LIB_ENABLED=1
    EXTERNAL_LIB_VERSION="1.2.3"
)
target_include_directories(external_config INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include_external>
)

# Create the simulated include directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include_external)

# Main library using Threads
add_library(netlib STATIC
    src/netlib.cpp
)
target_link_libraries(netlib
    PUBLIC
        Threads::Threads
        external_config
)

# Main executable
add_executable(net_app
    src/main.cpp
)
target_link_libraries(net_app
    PRIVATE
        netlib
)
