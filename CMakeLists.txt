cmake_minimum_required(VERSION 4.1.2)
project(c_compiler)

set(CMAKE_CXX_STANDARD 20)

add_executable(c_compiler src/main.cpp
        src/parse.cpp
        src/lexer.cpp
        src/ast.cpp
)