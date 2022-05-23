cmake_minimum_required(VERSION 3.10)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED true)

add_compile_options(
    # Include debug info
    -g

    -Werror

    -Wall
    -Wextra
    -Wpedantic

    -Wfloat-equal
    -Wcast-qual
    -Wconversion
    -Wsign-conversion

    -Wno-gnu-folding-constant

    -fshort-enums
)

add_subdirectory(lib/cut)
add_subdirectory(lib/hashmap)
add_subdirectory(lib/path)

project(Lala)

add_library(LalaLib
    src/constant.c
    src/heap.c
    src/lexer.c
    src/op_code.c
    src/parser.c
    src/scope.c
    src/stack.c
    src/token.c
    src/value_type.c
    src/vm.c
)
target_link_libraries(LalaLib PUBLIC
    HashMap
    Path
)
target_include_directories(LalaLib PUBLIC
    "lib/ccf"
    "lib/debug"
    "lib/hashmap"
    "lib/path"
    "src"
)

add_executable(lala
    src/main.c
)
target_link_libraries(lala PUBLIC
    LalaLib
)
target_include_directories(lala PUBLIC
    "src"
)

add_executable(LalaTest
    test/lexer_test.c
    test/parser_test.c
    test/vm_test.c
)
target_link_libraries(LalaTest PUBLIC
    Cut
    LalaLib
)
target_include_directories(LalaTest PUBLIC
    "lib/cut"
    "lib/debug"
    "src"
)

