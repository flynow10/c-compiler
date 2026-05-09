//
// Created by Natalie Wagner on 5/6/26.
//

#include "global.hpp"

#include <ostream>

std::ostream & IR::operator<<(std::ostream &os, const Global &global) {
    os << "@" << global.label << ":" << std::endl;
    return os << "reserve " << global.size << "bytes" << std::endl;
}
