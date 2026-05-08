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
        std::vector<std::unique_ptr<Instruction>> instructions;
        std::vector<Block *> prev;
        std::vector<Block *> next;

    public:
        explicit Block(const Label label) : label(label) {}

        template<typename IType>
        IType *add_instruction(IType&& inst);
        [[nodiscard]] bool is_fallthrough() const;
    };

    template<typename IType>
    IType * Block::add_instruction(IType &&inst) {
        instructions.push_back(std::make_unique<IType>(std::forward<IType>(inst)));
    }
}

#endif //C_COMPILER_BLOCK_HPP
