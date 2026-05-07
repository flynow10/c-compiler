//
// Created by Natalie Wagner on 5/6/26.
//

#ifndef IR_CONTEXT_HPP
#define IR_CONTEXT_HPP

#include "function.hpp"
#include "global.hpp"
#include "types.hpp"

#include "../parsing/ast.hpp"


namespace IR {

    class IRContext {
        Label global_counter = 0;

        std::vector<std::unique_ptr<Global>> globals;
        std::vector<std::unique_ptr<Function>> functions;
    public:
        void lower_AST(AST::TranslationUnit * ast);
        void lower_globals(AST::TranslationUnit * ast);

        Global *add_global(std::string identifier, MemSize size);
    };

}

#endif //IR_CONTEXT_HPP
