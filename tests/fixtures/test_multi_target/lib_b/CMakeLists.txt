add_library(lib_b SHARED
    src/lib_b.cpp
)

target_include_directories(lib_b
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(lib_b
    PUBLIC
        lib_a
)

target_compile_definitions(lib_b
    PRIVATE
        LIB_B_INTERNAL=1
)
