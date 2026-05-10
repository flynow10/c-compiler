//
// Created by Natalie Wagner on 5/6/26.
//

#ifndef FUNCTION_HPP
#define FUNCTION_HPP
#include <utility>

#include "block.hpp"

namespace IR {

    class Function {
        std::string identifier;
        std::vector<Register> arguments;
        std::vector<std::unique_ptr<Block>> blocks;

    public:
        explicit Function(std::string identifier) : identifier(std::move(identifier)) {}

        [[nodiscard]] const std::string &get_identifier() const { return identifier; }

        [[nodiscard]] const std::vector<std::unique_ptr<Block>> &get_blocks() const {
            return blocks;
        }


        Block *add_block(Label label);
        Block *add_block(Label label, Block *after);

        Register *add_argument(const Register &argument);

        friend std::ostream &operator<<(std::ostream &os, const Function &function);
    };

}



#endif //FUNCTION_HPP
