//
// Created by Natalie Wagner on 5/4/26.
//

#include "riscv_target.hpp"

#include <ranges>

void RISCVTarget::gen_preamble() {
    output << ".text" << "\n";
}

void RISCVTarget::gen_function(const IR::IRContext &ctx, const IR::Function *function) {
    const auto &functionName = function->get_identifier();
    add_label(functionName);
    push_reg("ra");
    add_instruction("mv t0, sp");
    push_reg("t0");

    for (const auto & block : function->get_blocks()) {
        add_label(get_function_label(block->get_label(), functionName));
        for (const auto & instruction : block->get_instructions()) {
            gen_instruction(ctx, function, instruction.get());
        }
    }
    pop_reg("t0");
    add_instruction("mv sp, t0");
    pop_reg("ra");
    add_instruction("ret");
}

void RISCVTarget::gen_instruction(const IR::IRContext &ctx, const IR::Function *function, const IR::Instruction *instruction) {
    if (auto *loadImm = dyn_cast<IR::LoadImmInst>(instruction)) {
        auto dest= get_stack_offset(loadImm->get_dest());
        add_instruction("li t0, " + std::to_string(loadImm->get_value()));
        set_reg_on_stack("t0", dest);
    } else if (auto *arithInst = dyn_cast<IR::ArithInst>(instruction)) {
        gen_arith_instruction(ctx, arithInst);
    } else if (auto *moveInst = dyn_cast<IR::MoveInst>(instruction)) {
        auto dest = get_stack_offset(moveInst->get_dest());
        auto source = get_stack_offset(moveInst->get_source());
        load_reg_on_stack("t0", source);
        set_reg_on_stack("t0", dest);
        // add_instruction("mv " + dest + ", " + source);
    } else if (auto *compareInst = dyn_cast<IR::CompareInst>(instruction)) {
        gen_compare_instruction(ctx, compareInst);
    } else if (auto *jumpInst = dyn_cast<IR::JumpInst>(instruction)) {
        auto label = get_function_label(jumpInst->get_label(), function->get_identifier());
        add_instruction("j " + label);
    } else if (auto *branchInst = dyn_cast<IR::BranchInst>(instruction)) {
        auto label = get_function_label(branchInst->get_label(), function->get_identifier());
        auto branchReg = get_stack_offset(branchInst->get_condition());
        load_reg_on_stack("t0", branchReg);
        add_instruction("beqz t0, " + label);
    }
}

void RISCVTarget::gen_arith_instruction(const IR::IRContext &ctx, const IR::ArithInst *arithInst) {
    std::string operation;
    std::string outputReg = "t0";
    // std::string outputReg = get_or_allocate(arithInst->get_dest());
    load_reg_on_stack("t1", get_stack_offset(arithInst->get_source1()));
    load_reg_on_stack("t2", get_stack_offset(arithInst->get_source2()));
    switch (arithInst->get_operation()) {
        case IR::ArithInst::Operation::ADD:
            operation = "add";
            break;
        case IR::ArithInst::Operation::SUBTRACT:
            operation = "sub";
            break;
        case IR::ArithInst::Operation::MULTIPLY:
            operation = "mul";
            break;
        case IR::ArithInst::Operation::DIVIDE:
            operation = "div";
            break;
        case IR::ArithInst::Operation::MODULO:
            operation = "rem";
            break;
        case IR::ArithInst::Operation::SHIFT_LEFT:
            operation = "sll";
            break;
        case IR::ArithInst::Operation::SHIFT_RIGHT:
            operation = "sra";
            break;
        case IR::ArithInst::Operation::AND:
            operation = "and";
            break;
        case IR::ArithInst::Operation::OR:
            operation = "or";
            break;
        case IR::ArithInst::Operation::XOR:
            operation = "xor";
            break;
    }
    add_instruction(operation + " t0, t1, t2");
    set_reg_on_stack("t0", get_stack_offset(arithInst->get_dest()));
}

void RISCVTarget::gen_compare_instruction(const IR::IRContext &ctx, const IR::CompareInst *compareInst) {
    std::string dest = "t0";
    std::string source1 = "t1";
    std::string source2 = "t2";
    // std::string outputReg = get_or_allocate(arithInst->get_dest());
    load_reg_on_stack("t1", get_stack_offset(compareInst->get_source1()));
    load_reg_on_stack("t2", get_stack_offset(compareInst->get_source2()));
    // auto dest = get_or_allocate(compareInst->get_dest());
    // auto source1 = find_register(compareInst->get_source1());
    // auto source2 = find_register(compareInst->get_source2());
    switch (compareInst->get_operation()) {
        case IR::CompareInst::Operation::EQUAL:
            add_instruction("xor " + dest + ", " + source1 + ", " + source2);
            add_instruction("seqz " + dest + ", " + dest);
            break;
        case IR::CompareInst::Operation::NOT_EQUAL:
            add_instruction("xor " + dest + ", " + source1 + ", " + source2);
            add_instruction("snez " + dest + ", " + dest);
            break;
        case IR::CompareInst::Operation::LESS_THAN:
            add_instruction("slt " + dest + ", " + source1 + ", " + source2);
            break;
        case IR::CompareInst::Operation::LESS_THAN_EQUAL:
            add_instruction("slt " + dest + ", " + source1 + ", " + source2);
            add_instruction("xori " + dest + ", " + dest + ", 1");
            break;
        case IR::CompareInst::Operation::GREATER_THAN:
            add_instruction("sgt " + dest + ", " + source1 + ", " + source2);
            break;
        case IR::CompareInst::Operation::GREATER_THAN_EQUAL:
            add_instruction("sgt " + dest + ", " + source1 + ", " + source2);
            add_instruction("xori " + dest + ", " + dest + ", 1");
            break;
        case IR::CompareInst::Operation::LESS_THAN_UNSIGNED:
            add_instruction("sltu " + dest + ", " + source1 + ", " + source2);
            break;
        case IR::CompareInst::Operation::LESS_THAN_EQUAL_UNSIGNED:
            add_instruction("sltu " + dest + ", " + source1 + ", " + source2);
            add_instruction("xori " + dest + ", " + dest + ", 1");
            break;
        case IR::CompareInst::Operation::GREATER_THAN_UNSIGNED:
            add_instruction("sgtu " + dest + ", " + source1 + ", " + source2);
            break;
        case IR::CompareInst::Operation::GREATER_THAN_EQUAL_UNSIGNED:
            add_instruction("sgtu " + dest + ", " + source1 + ", " + source2);
            add_instruction("xori " + dest + ", " + dest + ", 1");
            break;
    }
    set_reg_on_stack("t0", get_stack_offset(compareInst->get_dest()));
}

std::string RISCVTarget::get_or_allocate(const IR::Register &reg) {
    if (used_registers.contains(reg.name)) {
        return find_register(reg);
    }
    return allocate_register(reg);
}

std::string RISCVTarget::allocate_register(const IR::Register &irReg) {
    std::string reg;
    for (const auto & currentReg : TempRegisters) {
        bool found = false;
        for (const auto &usedReg: used_registers | std::views::values) {
            if (usedReg == currentReg) {
                found = true;
                break;
            }
        }
        if (!found) {
            used_registers[irReg.name] = currentReg;
            reg = currentReg;
            break;
        }
    }

    if (!reg.empty()) {
        return reg;
    }

    throw std::runtime_error("Failed to allocate register");

}

std::string RISCVTarget::find_register(const IR::Register &reg) const {
    return used_registers.at(reg.name);
}

void RISCVTarget::set_reg_on_stack(const std::string &reg, size_t offset) {
    add_instruction("sw " + reg + ", " + std::to_string(offset - stack_pointer ) + "(sp)");
}

void RISCVTarget::load_reg_on_stack(const std::string &reg, size_t offset) {
    add_instruction("lw " + reg + ", " + std::to_string(offset - stack_pointer) + "(sp)");
}

size_t RISCVTarget::get_stack_offset(const IR::Register &reg) {
    if (register_stack_map.contains(reg.name)) {
        return register_stack_map.at(reg.name);
    }

    add_instruction("addi sp, sp, -4");

    stack_pointer -= 4;
    register_stack_map[reg.name] = stack_pointer;
    return stack_pointer;
}

void RISCVTarget::push_reg(const std::string &reg) {
    // if (stack_pointer % 16 == 0) {
    //     add_instruction("addi sp, sp, -16");
    // }
    add_instruction("addi sp, sp, -4");
    stack_pointer -= 4;
    add_instruction("sw " + reg + ", 0(sp)");
}

void RISCVTarget::pop_reg(const std::string &reg) {
    add_instruction("lw " + reg + ", 0(sp)");
    stack_pointer += 4;
    add_instruction("addi sp, sp, 4");
    // if (stack_pointer % 16 == 0) {
    //     add_instruction("addi sp, sp, 16");
    // }
}

std::string RISCVTarget::get_function_label(IR::Label label, const std::string &functionName) {
    return ".L_" + functionName + "_" + std::to_string(label);
}
