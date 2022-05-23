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


project(HashMap VERSION 1.0)

add_library(HashMap
    hashmap.c
)

target_include_directories(HashMap PUBLIC
    "../debug"
)

