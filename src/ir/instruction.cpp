//
// Created by Natalie Wagner on 5/6/26.
//

#include <sstream>

#include "instruction.hpp"

std::string IR::ArithInst::print() const {
    std::stringstream ss;
    ss << "\t" << this->dest << " = ";
    switch (this->op) {
        case Operation::ADD:
            ss << "add";
            break;
        case Operation::SUBTRACT:
            ss << "sub";
            break;
        case Operation::MULTIPLY:
            ss << "mul";
            break;
        case Operation::DIVIDE:
            ss << "div";
            break;
        case Operation::MODULO:
            ss << "mod";
            break;
        case Operation::OR:
            ss << "or";
            break;
        case Operation::AND:
            ss << "and";
            break;
        case Operation::XOR:
            ss << "xor";
            break;
        case Operation::SHIFT_RIGHT:
            ss << "shr";
            break;
        case Operation::SHIFT_LEFT:
            ss << "shl";
            break;
    }

    ss << " " << *source1 << ", " << *source2 << std::endl;
    return ss.str();
}

std::string IR::CompareInst::print() const {
    std::stringstream ss;
    ss << "\t" << dest << " = cmp ";
    switch (this->op) {
        case Operation::EQUAL:
            ss << "eq";
            break;
        case Operation::NOT_EQUAL:
            ss << "ne";
            break;
        case Operation::LESS_THAN:
            ss << "slt";
            break;
        case Operation::LESS_THAN_EQUAL:
            ss << "sle";
            break;
        case Operation::GREATER_THAN:
            ss << "sgt";
            break;
        case Operation::GREATER_THAN_EQUAL:
            ss << "sge";
            break;
        case Operation::LESS_THAN_UNSIGNED:
            ss << "ult";
            break;
        case Operation::LESS_THAN_EQUAL_UNSIGNED:
            ss << "ule";
            break;
        case Operation::GREATER_THAN_UNSIGNED:
            ss << "ugt";
            break;
        case Operation::GREATER_THAN_EQUAL_UNSIGNED:
            ss << "uge";
            break;
    }
    ss << " " << *source1 << ", " << *source2 << std::endl;
    return ss.str();
}

std::string IR::UnaryInst::print() const {
    std::stringstream ss;
    ss << "\t" << dest << " = ";
    switch (this->op) {
        case Operation::INVERT:
            ss << "invert";
            break;
        case Operation::NEGATE:
            ss << "negate";
            break;
    }
    ss << " " << *source << std::endl;
    return ss.str();
}

std::string IR::AllocateInst::print() const {
    std::stringstream ss;
    ss << "\t" << dest << " = allocate " << size * 8 << std::endl;
    return ss.str();
}

std::string IR::LoadInst::print() const {
    std::stringstream ss;
    ss << "\t" << dest << " = load " << *source << std::endl;
    return ss.str();
}

std::string IR::StoreInst::print() const {
    std::stringstream ss;
    ss << "\tstore " << *source << " at " << dest << std::endl;
    return ss.str();
}

std::string IR::JumpInst::print() const {
    std::stringstream ss;
    ss << "\tjmp to " << *jump_point << std::endl;
    return ss.str();
}

std::string IR::BranchInst::print() const {
    std::stringstream ss;
    ss << "\tbr to " << *branch_point << " if " << condition << " = 0" << std::endl;
    return ss.str();
}

std::ostream &printArgList(std::ostream &os, const std::vector<IR::Register> &args) {
    return os;
}

std::string IR::CallInst::print() const {
    std::stringstream ss;
    ss << "\t" << dest << " = call " << *func_ptr;
    for (const auto &arg : args) {
        ss << ", " << *arg;
    }
    ss << std::endl;
    return ss.str();
}

std::string IR::ReturnInst::print() const {
    std::stringstream ss;
    ss << "\tret " << *return_value << std::endl;
    return ss.str();
}


std::ostream & IR::operator<<(std::ostream &os, const Instruction &inst) {
    return os << inst.print();
}
