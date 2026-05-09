//
// Created by Natalie Wagner on 5/5/26.
//

#include "block.hpp"

#include <ostream>

#include "../casting.hpp"


void IR::Block::is_dominated_by(Block *dominator) {
    this->prev.push_back(dominator);
    dominator->dominates_over(this);
}

void IR::Block::dominates_over(Block *successor) {
    this->next.push_back(successor);
}

bool IR::Block::is_fallthrough() const {
    return !isa<JumpInst, BreakInst, ReturnInst>(this->instructions.back().get());
}

IR::Label IR::Block::get_label() const {
    return label;
}

std::ostream & IR::operator<<(std::ostream &os, const Block &block) {
    os << "\t@" << block.label << ":" << std::endl;
    for (const auto & instruction : block.instructions) {
        os << *instruction;
    }
    os << std::endl;
    return os;
}
