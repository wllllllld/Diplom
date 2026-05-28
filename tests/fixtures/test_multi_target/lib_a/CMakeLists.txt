add_library(lib_a STATIC
    src/lib_a.cpp
)

target_include_directories(lib_a
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_definitions(lib_a
    PUBLIC
        LIB_A_ENABLED=1
)
