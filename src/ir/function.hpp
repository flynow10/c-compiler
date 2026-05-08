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

        Block *add_block(Label label);
    };

}



#endif //FUNCTION_HPP
