//
// Created by Natalie Wagner on 5/4/26.
//

#include "riscv_target.hpp"

#include <ranges>

#include "../../ir/instruction.hpp"
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
    return_address_location = reserve_next_alloca(4);
    set_reg_on_stack("ra", return_address_location);

    const auto &arguments = function->get_arguments();
    for (int i = 0; i < arguments.size(); ++i) {
        if (i > 7) {
            throw std::runtime_error("Too many arguments to function");
        }
        const auto &argument = arguments[i];
        ir_to_machine_reg[argument.name] = "a" + std::to_string(i);
    }

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
        gen_load_instruction(ctx, loadInst);
    } else if (auto *storeInst = dyn_cast<IR::StoreInst>(instruction)) {
        gen_store_instruction(ctx, storeInst);
    } else if (auto *returnInst = dyn_cast<IR::ReturnInst>(instruction)) {
        gen_return_instruction(ctx, returnInst);
    } else if (auto *callInst = dyn_cast<IR::CallInst>(instruction)) {
        gen_call_instruction(ctx, callInst);
    }
}

const std::vector CommutativeOperations = {
    IR::ArithInst::Operation::ADD,
    IR::ArithInst::Operation::MULTIPLY,
    IR::ArithInst::Operation::AND,
    IR::ArithInst::Operation::OR,
    IR::ArithInst::Operation::XOR,
};

const std::vector DisallowImmediates= {
    IR::ArithInst::Operation::SUBTRACT,
    IR::ArithInst::Operation::MULTIPLY,
    IR::ArithInst::Operation::DIVIDE,
    IR::ArithInst::Operation::MODULO,
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
    std::string tempImmediateReg1, tempImmediateReg2;
    std::string argument1;
    std::string argument2;
    if (auto *immArg = dyn_cast<IR::ImmediateArgument>(source1)) {
        tempImmediateReg1 = allocate_machine_reg();
        add_instruction("li " + tempImmediateReg1 + ", " + std::to_string(immArg->imm.value));
        argument1 = tempImmediateReg1;
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

    // Handle instruction types which don't offer an immediate variant
    if (!immediateModifier.empty() && std::ranges::find(DisallowImmediates, arithInst->get_operation()) != DisallowImmediates.end()) {
        tempImmediateReg2 = allocate_machine_reg();
        add_instruction("li " + tempImmediateReg2 + ", " + argument2);
        argument2 = tempImmediateReg2;
        immediateModifier = "";
    }

    add_instruction(operation + immediateModifier + " " + outputReg + ", " + argument1 + ", " + argument2);

    if (!tempImmediateReg1.empty()) {
        free_machine_reg(tempImmediateReg1);
    }
    if (!tempImmediateReg2.empty()) {
        free_machine_reg(tempImmediateReg2);
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
        auto machineReg = get_possibly_ptr_machine_reg(regArg->reg);
        add_instruction("mv a0, " + machineReg);
    }
    const size_t stackSize = get_aligned_stack_size(max_stack_alloc);
    load_reg_on_stack("ra", return_address_location);
    add_instruction("addi sp, sp, " + std::to_string(stackSize));
    add_instruction("ret");
}

void RISCVTarget::gen_call_instruction(const IR::IRContext &ctx, const IR::CallInst *callInst) {

    // Save any used registers
    size_t numSaved = used_registers.size();
    if (numSaved > 0) {
        add_instruction("addi sp, sp, -" + std::to_string(numSaved * 4));
        for (int i = 0; i < numSaved; ++i) {
            std::string reg = used_registers[i];
            add_instruction("sw " + reg + ", " + std::to_string(i * 4) + "(sp)");
        }
    }

    // Setup argument registers
    auto &arguments = callInst->get_args();
    for (int i = 0; i < arguments.size(); ++i) {
        auto *argument = arguments[i].get();
        auto argReg = "a" + std::to_string(i);
        if (auto *immediateArg = dyn_cast<IR::ImmediateArgument>(argument)) {
            add_instruction("li " + argReg + ", " + std::to_string(immediateArg->imm.value));
        } else if (auto *regArg = dyn_cast<IR::RegisterArgument>(argument)) {
            auto machineReg = get_possibly_ptr_machine_reg(regArg->reg);
            add_instruction("mv " + argReg + ", " + machineReg);
        }
    }

    auto *funcPtr = callInst->get_func_ptr();
    if (auto *functionRefArg = dyn_cast<IR::FunctionPtrArgument>(funcPtr)) {
        add_instruction("jal " + functionRefArg->func_name);
    } else {
        auto *registerArg = dyn_cast<IR::RegisterArgument>(funcPtr);
        auto machineReg = get_or_allocate_machine_reg(registerArg->reg);
        add_instruction("jalr " + machineReg);
    }

    if (numSaved > 0) {
        for (int i = 0; i < numSaved; ++i) {
            std::string reg = used_registers[i];
            add_instruction("lw " + reg + ", " + std::to_string(i * 4) + "(sp)");
        }
        add_instruction("addi sp, sp, " + std::to_string(numSaved * 4));
    }
    std::string outputReg = get_or_allocate_machine_reg(callInst->get_dest());
    add_instruction("mv " + outputReg + ", a0");
}

void RISCVTarget::gen_load_instruction(const IR::IRContext &ctx, const IR::LoadInst *loadInst) {
    auto destReg = get_possibly_ptr_machine_reg(loadInst->get_dest());
    auto *source = loadInst->get_source();
    if (auto *regArg = dyn_cast<IR::RegisterArgument>(source)) {
        if (register_stack_map.contains(regArg->reg.name)) {
            load_reg_on_stack(destReg, register_stack_map.at(regArg->reg.name));
        } else if (regArg->reg.type == IR::Register::Type::Ptr && ir_to_machine_reg.contains(regArg->reg.name)) {
            auto sourceReg = get_machine_reg(regArg->reg);
            add_instruction("lw " + destReg + ", (" + sourceReg + ")");
        } else {
            throw std::runtime_error("Not implemented");
        }
    } else if (auto *globalArg = dyn_cast<IR::GlobalArgument>(source)) {
        throw std::runtime_error("Not implemented");
    } else {
        throw std::runtime_error("Not implemented");
    }
}

void RISCVTarget::gen_store_instruction(const IR::IRContext &ctx, const IR::StoreInst *storeInst) {
    auto *source = storeInst->get_source();
    std::string sourceReg;
    if (auto *regArg = dyn_cast<IR::RegisterArgument>(source)) {
        sourceReg = get_possibly_ptr_machine_reg(regArg->reg);
    } else if (auto *immediate = dyn_cast<IR::ImmediateArgument>(source)) {
        sourceReg = allocate_machine_reg();
        add_instruction("li " + sourceReg + ", " + std::to_string(immediate->imm.value));
        free_machine_reg(sourceReg);
    }

    const auto &dest = storeInst->get_dest();
    if (register_stack_map.contains(dest.name)) {
        size_t stackOffset = register_stack_map.at(dest.name);
        set_reg_on_stack(sourceReg, stackOffset);
    } else if (dest.type == IR::Register::Type::Ptr && ir_to_machine_reg.contains(dest.name)) {
        auto destPtr = get_machine_reg(dest);
        add_instruction("sw " + sourceReg + ", (" + destPtr + ")");
    } else {
        throw std::runtime_error("Not implemented");
    }
}

std::string RISCVTarget::get_possibly_ptr_machine_reg(const IR::Register &reg) {
    std::string machineReg = get_or_allocate_machine_reg(reg);
    if (reg.type == IR::Register::Type::Ptr && register_stack_map.contains(reg.name)) {
        add_instruction("addi " + machineReg + ", sp, " + std::to_string(register_stack_map.at(reg.name)));
    }
    return machineReg;
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

std::string RISCVTarget::get_machine_reg(const IR::Register &reg) {
    if (!ir_to_machine_reg.contains(reg.name)) {
        throw std::runtime_error("Ir register does not have corrosponding machine register allocated");
    }
    return ir_to_machine_reg.at(reg.name);
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
