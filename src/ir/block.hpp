//
// Created by Natalie Wagner on 5/5/26.
//

#ifndef C_COMPILER_BLOCK_HPP
#define C_COMPILER_BLOCK_HPP
#include <vector>

#include "instruction.hpp"

namespace IR {
    class Block {
        Label label;
        std::vector<std::unique_ptr<Instruction>> statements;
        std::vector<Block *> prev;
        std::vector<Block *> next;

        explicit Block(int label) : label(label) {}
    };
}

#endif //C_COMPILER_BLOCK_HPP
