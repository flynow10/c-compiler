//
// Created by Natalie Wagner on 5/4/26.
//

#include "riscv_target.hpp"

#include <ranges>

#include "../../ir/instruction.hpp"

void RISCVTarget::gen_preamble() {
    output << ".text" << "\n";
}

void RISCVTarget::gen_function(const IR::IRContext &ctx, const IR::Function *function) {
    const auto &functionName = function->get_identifier();
    add_label(functionName);

    next_alloca_pointer = 0;
    ir_to_machine_reg.clear();
    used_registers.clear();
    register_stack_map.clear();

    max_stack_alloc = compute_stack_allocations(ctx, function) + 4;
    const size_t alignedStackAlloc = get_aligned_stack_size(max_stack_alloc);
    add_instruction("addi sp, sp, -" + std::to_string(alignedStackAlloc));
    return_address_location= reserve_next_alloca(4);
    set_reg_on_stack("ra", return_address_location);

    for (const auto & block : function->get_blocks()) {
        add_label(get_function_label(block->get_label(), functionName));
        for (const auto & instruction : block->get_instructions()) {
            gen_instruction(ctx, function, instruction.get());
        }
    }
}

void RISCVTarget::gen_instruction(const IR::IRContext &ctx, const IR::Function *function, const IR::Instruction *instruction) {
    if (auto *arithInst = dyn_cast<IR::ArithInst>(instruction)) {
        gen_arith_instruction(ctx, arithInst);
    } else if (auto *compareInst = dyn_cast<IR::CompareInst>(instruction)) {
        gen_compare_instruction(ctx, compareInst);
    } else if (auto *jumpInst = dyn_cast<IR::JumpInst>(instruction)) {
        if (auto *label = dyn_cast<IR::LabelArgument>(jumpInst->get_jump_point())) {
            auto asmLabel = get_function_label(label->label, function->get_identifier());
            add_instruction("j " + asmLabel);
        } else {
            auto ptrReg = dyn_cast<IR::RegisterArgument>(jumpInst->get_jump_point());
            auto machineReg = get_or_allocate_machine_reg(ptrReg->reg);
            add_instruction("jr " + machineReg);
        }
    } else if (auto *branchInst = dyn_cast<IR::BranchInst>(instruction)) {
        auto &conditionReg= branchInst->get_condition();
        auto machineCondition= get_or_allocate_machine_reg(conditionReg);
        if (auto *label = dyn_cast<IR::LabelArgument>(branchInst->get_branch_point())) {
            auto asmlabel = get_function_label(label->label, function->get_identifier());
            add_instruction("beqz " + machineCondition + ", " + asmlabel);
        } else {
            throw std::runtime_error("Branch point should be a label");
        }
    } else if (auto *allocaInst = dyn_cast<IR::AllocateInst>(instruction)) {
        size_t stackPos = reserve_next_alloca(allocaInst->get_size());
        register_stack_map[allocaInst->get_dest().name] = stackPos;
    } else if (auto *loadInst = dyn_cast<IR::LoadInst>(instruction)) {
        auto destReg = get_or_allocate_machine_reg(loadInst->get_dest());
        auto *source = loadInst->get_source();
        if (auto *regArg = dyn_cast<IR::RegisterArgument>(source)) {
            load_reg_on_stack(destReg, register_stack_map.at(regArg->reg.name));
        } else if (auto *globalArg = dyn_cast<IR::GlobalArgument>(source)) {
            throw std::runtime_error("Not implemented");
        } else {
            throw std::runtime_error("Not implemented");
        }
    } else if (auto *storeInst = dyn_cast<IR::StoreInst>(instruction)) {
        size_t stackOffset = register_stack_map.at(storeInst->get_dest().name);
        auto *source = storeInst->get_source();
        if (auto *regArg = dyn_cast<IR::RegisterArgument>(source)) {
            auto sourceReg = get_or_allocate_machine_reg(regArg->reg);
            set_reg_on_stack(sourceReg, stackOffset);
        } else if (auto *immediate = dyn_cast<IR::ImmediateArgument>(source)) {
            auto tempReg = allocate_machine_reg();
            add_instruction("li " + tempReg + ", " + std::to_string(immediate->imm.value));
            set_reg_on_stack(tempReg, stackOffset);
            free_machine_reg(tempReg);
        }
    } else if (auto *returnInst = dyn_cast<IR::ReturnInst>(instruction)) {
        gen_return_instruction(ctx, returnInst);
    }
}

const std::vector<IR::ArithInst::Operation> CommutativeOperations = {
    IR::ArithInst::Operation::ADD,
    IR::ArithInst::Operation::MULTIPLY,
    IR::ArithInst::Operation::AND,
    IR::ArithInst::Operation::OR,
    IR::ArithInst::Operation::XOR,
};

void RISCVTarget::gen_arith_instruction(const IR::IRContext &ctx, const IR::ArithInst *arithInst) {
    std::string operation;
    std::string outputReg = get_or_allocate_machine_reg(arithInst->get_dest());
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
    auto *source1 = arithInst->get_source1();
    auto *source2 = arithInst->get_source2();
    if (isa<IR::ImmediateArgument>(source1) && isa<IR::RegisterArgument>(source2)
        && std::ranges::find(CommutativeOperations, arithInst->get_operation()) != CommutativeOperations.end()) {
        auto *temp = source1;
        source1 = source2;
        source2 = temp;
    }
    std::string tempImmediateReg;
    std::string argument1;
    std::string argument2;
    if (auto *immArg = dyn_cast<IR::ImmediateArgument>(source1)) {
        tempImmediateReg = allocate_machine_reg();
        add_instruction("li " + tempImmediateReg + ", " + std::to_string(immArg->imm.value));
        argument1 = tempImmediateReg;
    } else {
        auto *regArg = cast<IR::RegisterArgument>(source1);
        argument1 = get_or_allocate_machine_reg(regArg->reg);
    }

    std::string immediateModifier;
    if (auto *immArg = dyn_cast<IR::ImmediateArgument>(source2)) {
        immediateModifier = "i";
        argument2 = std::to_string(immArg->imm.value);
    } else {
        auto *regArg = cast<IR::RegisterArgument>(source2);
        argument2 = get_or_allocate_machine_reg(regArg->reg);
    }

    add_instruction(operation + immediateModifier + " " + outputReg + ", " + argument1 + ", " + argument2);
    if (!tempImmediateReg.empty()) {
        free_machine_reg(tempImmediateReg);
    }
}

const std::vector ReversedOperations = {
    IR::CompareInst::Operation::LESS_THAN_EQUAL,
    IR::CompareInst::Operation::GREATER_THAN,
    IR::CompareInst::Operation::LESS_THAN_EQUAL_UNSIGNED,
    IR::CompareInst::Operation::GREATER_THAN_UNSIGNED,
};

void RISCVTarget::gen_compare_instruction(const IR::IRContext &ctx, const IR::CompareInst *compareInst) {
    auto *source1 = compareInst->get_source1();
    auto *source2 = compareInst->get_source2();
    if (std::ranges::find(ReversedOperations, compareInst->get_operation()) != ReversedOperations.end()) {
        auto *temp = source1;
        source1 = source2;
        source2 = temp;
    }
    std::string outputReg = get_or_allocate_machine_reg(compareInst->get_dest());
    std::string tempImmediateReg;
    std::string argument1;
    std::string argument2;
    if (auto *immArg = dyn_cast<IR::ImmediateArgument>(source1)) {
        tempImmediateReg = allocate_machine_reg();
        add_instruction("li " + tempImmediateReg + ", " + std::to_string(immArg->imm.value));
        argument1 = tempImmediateReg;
    } else {
        auto *regArg = cast<IR::RegisterArgument>(source1);
        argument1 = get_or_allocate_machine_reg(regArg->reg);
    }

    std::string immediateModifier;
    if (auto *immArg = dyn_cast<IR::ImmediateArgument>(source2)) {
        immediateModifier = "i";
        argument2 = std::to_string(immArg->imm.value);
    } else {
        auto *regArg = cast<IR::RegisterArgument>(source2);
        argument2 = get_or_allocate_machine_reg(regArg->reg);
    }

    switch (compareInst->get_operation()) {
        case IR::CompareInst::Operation::EQUAL:
            add_instruction("xor" + immediateModifier + " " + outputReg + ", " + argument1 + ", " + argument2);
            add_instruction("seqz " + outputReg + ", " + outputReg);
            break;
        case IR::CompareInst::Operation::NOT_EQUAL:
            add_instruction("xor" + immediateModifier + " " + outputReg + ", " + argument1 + ", " + argument2);
            add_instruction("snez " + outputReg + ", " + outputReg);
            break;
        case IR::CompareInst::Operation::LESS_THAN:
            add_instruction("slt" + immediateModifier + " " + outputReg + ", " + argument1 + ", " + argument2);
            break;
        case IR::CompareInst::Operation::LESS_THAN_EQUAL:
            // Sources have been flipped
            add_instruction("slt" + immediateModifier + " " + outputReg + ", " + argument1 + ", " + argument2);
            add_instruction("xori " + outputReg + ", " + outputReg + ", 1");
            break;
        case IR::CompareInst::Operation::GREATER_THAN:
            // Sources have been flipped
            add_instruction("slt" + immediateModifier + " " + outputReg + ", " + argument1 + ", " + argument2);
            break;
        case IR::CompareInst::Operation::GREATER_THAN_EQUAL:
            add_instruction("slt" + immediateModifier + " " + outputReg + ", " + argument1 + ", " + argument2);
            add_instruction("xori " + outputReg + ", " + outputReg + ", 1");
            break;
        case IR::CompareInst::Operation::LESS_THAN_UNSIGNED:
            add_instruction("slt" + immediateModifier + "u " + outputReg + ", " + argument1 + ", " + argument2);
            break;
        case IR::CompareInst::Operation::LESS_THAN_EQUAL_UNSIGNED:
            // Sources have been flipped
            add_instruction("slt" + immediateModifier + "u " + outputReg + ", " + argument1 + ", " + argument2);
            add_instruction("xori " + outputReg + ", " + outputReg + ", 1");
            break;
        case IR::CompareInst::Operation::GREATER_THAN_UNSIGNED:
            // Sources have been flipped
            add_instruction("slt" + immediateModifier + "u " + outputReg + ", " + argument1 + ", " + argument2);
            break;
        case IR::CompareInst::Operation::GREATER_THAN_EQUAL_UNSIGNED:
            add_instruction("slt" + immediateModifier + "u " + outputReg + ", " + argument1 + ", " + argument2);
            add_instruction("xori " + outputReg + ", " + outputReg + ", 1");
            break;
    }

    if (!tempImmediateReg.empty()) {
        free_machine_reg(tempImmediateReg);
    }
}

void RISCVTarget::gen_return_instruction(const IR::IRContext &ctx, const IR::ReturnInst *returnInst) {
    auto *returnValue = returnInst->get_return_value();
    if (auto *immediateArg = dyn_cast<IR::ImmediateArgument>(returnValue)) {
        add_instruction("li a0, " + std::to_string(immediateArg->imm.value));
    } else if (auto *regArg = dyn_cast<IR::RegisterArgument>(returnValue)) {
        auto machineReg = get_or_allocate_machine_reg(regArg->reg);
        add_instruction("mv a0, " + machineReg);
    }
    const size_t stackSize = get_aligned_stack_size(max_stack_alloc);
    load_reg_on_stack("ra", return_address_location);
    add_instruction("addi sp, sp, " + std::to_string(stackSize));
    add_instruction("ret");
}

size_t RISCVTarget::compute_stack_allocations(const IR::IRContext &ctx, const IR::Function *function) {
    size_t allocated = 0;
    for (const auto & block : function->get_blocks()) {
        for (const auto & instruction : block->get_instructions()) {
            if (auto *alloca = dyn_cast<IR::AllocateInst>(instruction.get())) {
                allocated += alloca->get_size();
            }
        }
    }
    return allocated;
}

size_t RISCVTarget::reserve_next_alloca(size_t allocaSize) {
    next_alloca_pointer += allocaSize;
    if (next_alloca_pointer > max_stack_alloc) {
        throw std::runtime_error("Stack allocation outside of precomputed range");
    }
    return next_alloca_pointer;
}

size_t RISCVTarget::get_aligned_stack_size(const size_t size) {
    return size + (16 - size % 16) % 16;
}

void RISCVTarget::set_reg_on_stack(const std::string &reg, const size_t offset) {
    add_instruction("sw " + reg + ", " + std::to_string(get_aligned_stack_size(max_stack_alloc) - offset) + "(sp)");
}

void RISCVTarget::load_reg_on_stack(const std::string &reg, const size_t offset) {
    add_instruction("lw " + reg + ", " + std::to_string(get_aligned_stack_size(max_stack_alloc) - offset) + "(sp)");
}

std::string RISCVTarget::get_or_allocate_machine_reg(const IR::Register &reg) {
    if (ir_to_machine_reg.contains(reg.name)) {
        return ir_to_machine_reg.at(reg.name);
    }
    std::string machineReg = allocate_machine_reg();
    ir_to_machine_reg[reg.name] = machineReg;
    return machineReg;
}

void RISCVTarget::free_machine_reg(const std::string &reg) {
    const auto pos = std::ranges::find(used_registers, reg);
    if (pos != used_registers.end()) {
        used_registers.erase(pos);
    }
}

std::string RISCVTarget::allocate_machine_reg() {
    for (const auto &temp_register : TempRegisters) {
        if (std::ranges::find(used_registers, temp_register) == used_registers.end()) {
            used_registers.push_back(temp_register);
            return temp_register;
        }
    }

    throw std::runtime_error("Out of registers!");
    // Uh oh, spilling register
    // std::string spill = used_registers.front();
    // spilled_registers.push(spill);
    // add_instruction("addi sp, sp, -4");
    // add_instruction("sw " + spill + ", 0(sp)");
}

std::string RISCVTarget::get_function_label(IR::Label label, const std::string &functionName) {
    return ".L_" + functionName + "_" + std::to_string(label);
}
