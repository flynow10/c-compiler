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
        src/gen/mips/mips_target.cpp
        src/gen/mips/mips_target.hpp
        src/gen/ASM_gen.cpp
        src/gen/ASM_gen.hpp
        src/sema/symbol_table.cpp
        src/sema/symbol_table.hpp
        src/casting.hpp
)
add_executable(c_compiler_test test/main.cpp
        test/execute.cpp
        test/execute.hpp)
