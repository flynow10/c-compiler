//
// Created by Natalie Wagner on 5/4/26.
//

#ifndef RISCV_TARGET_HPP
#define RISCV_TARGET_HPP
#include "../ASM_gen.hpp"

using namespace AST;

class RISCVTarget : ASMGen{
public:
    explicit RISCVTarget(std::ostream &output)
        : ASMGen(output) {
    }

    void gen_ast(TranslationUnit *translation_unit) override;
    void gen_decl(Decl *decl);
    void gen_function_decl(FunctionDecl *function_decl);
};


#endif //RISCV_TARGET_HPP
