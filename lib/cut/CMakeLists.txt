cmake_minimum_required(VERSION 3.10)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED true)

add_compile_options(
    -Werror

    -Wall
    -Wextra
    -Wpedantic

    -Wfloat-equal
    -Wcast-qual
    -Wconversion
    -Wsign-conversion

    -fshort-enums
)


project(Cut VERSION 1.0)

add_library(Cut
    cut.c
)

target_include_directories(Cut PUBLIC
    "../ccf"
)

add_executable(Example
    example.c
)

target_link_libraries(Example
    Cut
)

