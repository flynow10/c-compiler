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


    std::map<std::string, std::string> used_registers;

    std::map<std::string, size_t> register_stack_map;

public:
    explicit RISCVTarget(std::ostream &output)
        : ASMGen(output) {
    }

    size_t stack_pointer = 0x7fffeffc;

protected:
    void gen_preamble() override;

    // Generation
    void gen_function(const IR::IRContext &ctx, const IR::Function *function) override;
    void gen_instruction(const IR::IRContext &ctx, const IR::Function *function, const IR::Instruction *instruction);
    void gen_arith_instruction(const IR::IRContext &ctx, const IR::ArithInst *arithInst);
    void gen_compare_instruction(const IR::IRContext &ctx, const IR::CompareInst *compareInst);

    // Register allocation
    std::string get_or_allocate(const IR::Register &reg);
    std::string allocate_register(const IR::Register &reg);
    void free_register(const IR::Register &reg);
    [[nodiscard]] std::string find_register(const IR::Register &reg) const;

    // Stack alloc approach
    void set_reg_on_stack(const std::string &reg, size_t offset);
    void load_reg_on_stack(const std::string &reg, size_t offset);
    size_t get_stack_offset(const IR::Register &reg);

    // Stack
    void push_reg(const std::string& reg);
    void pop_reg(const std::string& reg);


    static std::string get_function_label(IR::Label label, const std::string &functionName);
};


#endif //RISCV_TARGET_HPP
