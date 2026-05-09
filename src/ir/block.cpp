//
// Created by Natalie Wagner on 5/5/26.
//

#include "block.hpp"

#include <ostream>

#include "../casting.hpp"


void IR::Block::is_preceded_by(Block *preceder) {
    this->prev.push_back(preceder);
    preceder->succeeds(this);
}

void IR::Block::succeeds(Block *successor) {
    this->next.push_back(successor);
}

bool IR::Block::is_fallthrough() const {
    return !isa<JumpInst, BreakInst, ReturnInst>(this->instructions.back().get());
}

IR::Label IR::Block::get_label() const {
    return label;
}

std::ostream & IR::operator<<(std::ostream &os, const Block &block) {
    os << "\t@" << block.label << ": (precedes";
    for (const auto & next : block.next) {
        os << " @" << next->label;
    }
    os << ")" << std::endl;
    for (const auto & instruction : block.instructions) {
        os << *instruction;
    }
    os << std::endl;
    return os;
}
