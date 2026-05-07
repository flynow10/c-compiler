//
// Created by Natalie Wagner on 5/4/26.
//

#include "riscv_target.hpp"

void RISCVTarget::gen_ast(TranslationUnit *translation_unit) {
    for (const auto & node : *translation_unit) {
        if (isa<Decl>(node.get())) {
            gen_decl(cast<Decl>(node.get()));
        } else {

        }
    }
}

void RISCVTarget::gen_decl(Decl *decl) {
}

void RISCVTarget::gen_function_decl(FunctionDecl *function_decl) {

}
