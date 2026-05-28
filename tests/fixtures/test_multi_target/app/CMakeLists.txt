add_executable(multi_app
    src/main.cpp
)

target_link_libraries(multi_app
    PRIVATE
        lib_b
        lib_a
)
