//
// Created by Natalie Wagner on 4/18/26.
//

#include "sema.hpp"

#include "../casting.hpp"

void Sema::acceptAST(AST::Node *ast) {
    if (const auto tu = dyn_cast<AST::TranslationUnit>(ast)) {
        for (const auto & node: *tu) {
            
            throw std::runtime_error("Translation units must only contain declarations and function declarations.");
        }
    } else {
        throw std::runtime_error("AST must begin with a translation unit.");
    }
}