cmake_minimum_required(VERSION 4.1.2)
project(c_compiler)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-fsanitize=address -g)
add_link_options(-fsanitize=address -g)

add_executable(c_compiler src/main.cpp
        src/lexer.cpp
        src/parsing/parse.cpp
        src/parsing/ast.cpp
        src/sema/Sema.cpp
        src/sema/Sema.hpp
)
add_executable(c_compiler_test test/main.cpp
        test/execute.cpp
        test/execute.hpp)
