//
// Created by Natalie Wagner on 4/18/26.
//

#ifndef C_COMPILER_ASMGEN_HPP
#define C_COMPILER_ASMGEN_HPP
#include <iosfwd>
#include <ostream>

#include "../ir/ir_context.hpp"

class ASMGen {
protected:
    std::ostream& output;
public:
    explicit ASMGen(std::ostream& output) : output(output) {
    }

    virtual ~ASMGen() = default;

    void gen(const IR::IRContext &ir);
protected:
    void add_label(const std::string &label);
    void add_instruction(const std::string &instruction);

    virtual void gen_preamble() = 0;
    virtual void gen_function(const IR::IRContext &ctx, const IR::Function *func) = 0;
};

#endif //C_COMPILER_ASMGEN_HPP
