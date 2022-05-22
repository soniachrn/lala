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
    # -Wzero-as-null-pointer-constant
    # -Wextra-semi
    -Wsign-conversion
)


project(Path VERSION 1.0)

add_library(Path
    path.c
)

