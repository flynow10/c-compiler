//
// Created by Natalie Wagner on 5/6/26.
//

#include "function.hpp"

IR::Block * IR::Function::add_block(Label label) {
    return blocks.emplace_back(std::make_unique<Block>(label)).get();
}
