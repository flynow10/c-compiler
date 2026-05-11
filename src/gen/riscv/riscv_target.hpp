//
// Created by Natalie Wagner on 5/4/26.
//

#ifndef RISCV_TARGET_HPP
#define RISCV_TARGET_HPP
#include "../ASM_gen.hpp"

using namespace AST;

class RISCVTarget : public ASMGen {
    static constexpr std::string Registers[] = {
        "zero", "ra", "sp", "gp", "tp",
        "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3",
        "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6",
        "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
    };

    static constexpr std::string TempRegisters[] = {
        "t0", "t1", "t2", "s0", "s1", "s2", "s3", "s4", "s5",
        "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5",
        "t6"
    };

    size_t next_alloca_pointer = 0;
    size_t max_stack_alloc = 0;
    size_t return_address_location = 0;

    std::map<std::string, size_t> register_stack_map;
    std::vector<std::string> used_registers;
    std::map<std::string, std::string> ir_to_machine_reg;

public:
    explicit RISCVTarget(std::ostream &output)
        : ASMGen(output) {
    }

protected:
    void gen_preamble() override;

    // Generation
    void gen_function(const IR::IRContext &ctx, const IR::Function *function) override;
    void gen_instruction(const IR::IRContext &ctx, const IR::Function *function, const IR::Instruction *instruction);
    void gen_arith_instruction(const IR::IRContext &ctx, const IR::ArithInst *arithInst);
    void gen_compare_instruction(const IR::IRContext &ctx, const IR::CompareInst *compareInst);
    void gen_return_instruction(const IR::IRContext &ctx, const IR::ReturnInst *returnInst);
    void gen_call_instruction(const IR::IRContext &ctx, const IR::CallInst *callInst);

    // Register allocation
    size_t compute_stack_allocations(const IR::IRContext &ctx, const IR::Function *function);
    size_t reserve_next_alloca(size_t allocaSize);
    size_t get_aligned_stack_size(size_t size);
    void set_reg_on_stack(const std::string &reg, size_t offset);
    void load_reg_on_stack(const std::string &reg, size_t offset);
    std::string get_or_allocate_machine_reg(const IR::Register &reg);

    void free_machine_reg(const std::string &reg);
    std::string allocate_machine_reg();

    static std::string get_function_label(IR::Label label, const std::string &functionName);
};


#endif //RISCV_TARGET_HPP
