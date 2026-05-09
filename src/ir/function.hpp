//
// Created by Natalie Wagner on 5/6/26.
//

#ifndef FUNCTION_HPP
#define FUNCTION_HPP
#include "block.hpp"

namespace IR {

    class Function {
        std::string identifier;
        std::vector<std::unique_ptr<Block>> blocks;

    public:
        explicit Function(const std::string &identifier) : identifier(identifier) {}

        [[nodiscard]] const std::string &get_identifier() const { return identifier; }

        [[nodiscard]] const std::vector<std::unique_ptr<Block>> &get_blocks() const {
            return blocks;
        }


        Block *add_block(Label label);
        Block *add_block(Label label, Block *after);

        friend std::ostream &operator<<(std::ostream &os, const Function &function);
    };

}



#endif //FUNCTION_HPP
