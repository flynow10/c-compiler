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
    }

    ss << " " << source1 << ", " << source2 << std::endl;
    return ss.str();
}

std::string IR::MoveInst::print() const {
    std::stringstream ss;
    ss << "\t" << dest << " = mov " << source << std::endl;
    return ss.str();
}

std::string IR::LoadLabelInst::print() const {
    std::stringstream ss;
    ss << "\t" << dest << " = set @" << label << std::endl;
    return ss.str();
}

std::string IR::LoadImmInst::print() const {
    std::stringstream ss;
    ss << "\t" << dest << " = 0x" << std::hex << value << std::endl;
    return ss.str();
}

std::string IR::LoadInst::print() const {
    std::stringstream ss;
    ss << "\t" << dest << " = load " << source << "(" << offset << ")" << std::endl;
    return ss.str();
}

std::string IR::JumpInst::print() const {
    std::stringstream ss;
    ss << "\tjmp to @" << label << std::endl;
    return ss.str();
}

std::string IR::BreakInst::print() const {
    std::stringstream ss;
    ss << "\tbr to @" << label << " if " << condition << " != 0" << std::endl;
    return ss.str();
}

std::string IR::ReturnInst::print() const {
    std::stringstream ss;
    ss << "\tret " << return_value << std::endl;
    return ss.str();
}


std::ostream & IR::operator<<(std::ostream &os, const Instruction &inst) {
    return os << inst.print();
}
