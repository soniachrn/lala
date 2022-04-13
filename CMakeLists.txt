cmake_minimum_required(VERSION 3.10)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED true)

add_compile_options(
    -g

    -Werror

    -Wall
    -Wextra
    -Wpedantic

    -Wfloat-equal
    -Wcast-qual
    -Wconversion
    -Wzero-as-null-pointer-constant
    -Wextra-semi
    -Wsign-conversion

    -Wno-gnu-statement-expression
    -Wno-gnu-folding-constant
)

add_subdirectory(lib/cut)

project(Lala)

add_library(Lala
    src/lexer.c
    src/parser.c
    src/stack.c
    src/token.c
    src/vm.c
)

add_executable(LalaTest
    test/lexer_test.c
    test/parser_test.c
    test/vm_test.c
)

target_link_libraries(LalaTest PUBLIC
    Cut
    Lala
)

target_include_directories(LalaTest PUBLIC
    "lib/cut"
    "src"
)

