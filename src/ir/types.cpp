//
// Created by Natalie Wagner on 5/10/26.
//
#include "types.hpp"

#include <sstream>

std::ostream & IR::operator<<(std::ostream &os, const Register &reg) {
    if (reg.type == Register::Type::Ptr) {
        os << "ptr ";
    } else if (reg.type == Register::Type::Struct) {
        os << "struct ";
    } else {
        os << "i" << std::to_string(reg.size * 8) << " ";
    }
    return os << "$" << reg.name;
}

// IR::Register::operator RegOrImmediate() const {
//     return RegOrImmediate(*this);
// }

std::string IR::RegisterArgument::print() const {
    std::stringstream ss;
    ss << reg;
    return ss.str();
}

std::string IR::ImmediateArgument::print() const {
    std::stringstream ss;
    ss << "i" << std::to_string(imm.size * 8) << " " << imm.value;
    return ss.str();
}

std::string IR::LabelArgument::print() const {
    return "@" + std::to_string(label);
}

std::string IR::FunctionPtrArgument::print() const {
    return "%" + func_name;
}

std::string IR::GlobalArgument::print() const {
    return "#" + global_id;
}
