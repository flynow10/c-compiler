//
// Created by Natalie Wagner on 5/6/26.
//

#include "function.hpp"

#include <ostream>

IR::Block * IR::Function::add_block(Label label) {
    return blocks.emplace_back(std::make_unique<Block>(label)).get();
}

std::ostream & IR::operator<<(std::ostream &os, const Function &function) {
    os << "fn " << function.identifier << "() {" << std::endl;
    for (const auto & block : function.blocks) {
        os << *block ;
    }
    return os << "}" << std::endl;
}
