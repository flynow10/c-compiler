//
// Created by Natalie Wagner on 5/6/26.
//

#include "function.hpp"

#include <ostream>

IR::Block * IR::Function::add_block(Label label) {
    return blocks.emplace_back(std::make_unique<Block>(label)).get();
}

IR::Block * IR::Function::add_block(Label label, Block *after) {
    auto block = std::make_unique<Block>(label);
    auto position = std::ranges::find_if(blocks, [&after](auto &&b){return b.get() == after;}) + 1;
    return blocks.insert(position, std::move(block))->get();
}

IR::Register *IR::Function::add_argument(const Register &argument) {
    return &arguments.emplace_back(argument);
}

std::ostream & IR::operator<<(std::ostream &os, const Function &function) {
    os << "fn %" << function.identifier << "(";
    for (int i = 0; i < function.arguments.size(); ++i) {
        os << function.arguments.at(i);
        if (i != function.arguments.size() - 1) {
            os << ", ";
        }
    }
    os << ") {" << std::endl;
    for (const auto & block : function.blocks) {
        os << *block ;
    }
    return os << "}" << std::endl;
}
