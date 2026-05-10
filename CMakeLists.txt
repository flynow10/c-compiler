cmake_minimum_required(VERSION 4.1.2)
project(c_compiler)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-fsanitize=address -g)
add_link_options(-fsanitize=address -g)

add_executable(c_compiler src/main.cpp
        src/lexer.cpp
        src/parsing/parse.cpp
        src/parsing/ast.cpp
        src/parsing/ast.hpp
        src/sema/sema.cpp
        src/sema/sema.hpp
        src/gen/mips/mips_target.cpp
        src/gen/mips/mips_target.hpp
        src/gen/ASM_gen.cpp
        src/gen/ASM_gen.hpp
        src/sema/symbol_table.cpp
        src/casting.hpp
        src/sema/expressions.cpp
        src/sema/types.cpp
        src/sema/types.hpp
        src/hash_combine.hpp
        src/sema/exceptions.cpp
        src/sema/exceptions.hpp
        src/gen/riscv/riscv_target.cpp
        src/gen/riscv/riscv_target.hpp
        src/ir/block.cpp
        src/ir/block.hpp
        src/ir/instruction.cpp
        src/ir/instruction.hpp
        src/ir/function.cpp
        src/ir/function.hpp
        src/ir/ir_context.cpp
        src/ir/ir_context.hpp
        src/ir/global.cpp
        src/ir/global.hpp
        src/ir/types.hpp
        src/ir/types.cpp
)
add_executable(c_compiler_test test/main.cpp
        test/execute.cpp
        test/execute.hpp)
