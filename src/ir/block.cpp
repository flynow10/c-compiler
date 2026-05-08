//
// Created by Natalie Wagner on 5/5/26.
//

#include "block.hpp"

#include "../casting.hpp"


bool IR::Block::is_fallthrough() const {
    return !isa<JumpInst, BreakInst, ReturnInst>(this->instructions.back().get());
}
