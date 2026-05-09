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
        void is_preceded_by(Block *preceder);
    private:
        void succeeds(Block *successor);
    public:
        [[nodiscard]] const std::vector<std::unique_ptr<Instruction>> &get_instructions() const;
        [[nodiscard]] bool is_fallthrough() const;
        [[nodiscard]] Label get_label() const;

        friend std::ostream &operator<<(std::ostream &os, const Block &block);
    };

    template<typename IType>
    IType * Block::add_instruction(IType &&inst) {
        return static_cast<IType* >(instructions.emplace_back(std::make_unique<IType>(std::forward<IType>(inst))).get());
    }
}

#endif //C_COMPILER_BLOCK_HPP
