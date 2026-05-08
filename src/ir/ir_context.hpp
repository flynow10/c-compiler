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
        Label function_counter = 0;
        Register reg_counter = 0;

        std::map<std::string, std::unique_ptr<Global>> globals;
        std::vector<std::unique_ptr<Function>> functions;

        Function * current_function = nullptr;
        Block * current_block = nullptr;
        std::unordered_map<std::string, std::vector<Register>> id_reg_map;

    public:
        void lower_AST(AST::TranslationUnit * ast);
        void lower_globals(AST::TranslationUnit * ast);
        void lower_function(AST::FunctionDecl *decl);
        void lower_statement(AST::Statement *stmt);

        Register lower_expression(AST::Expression *expr);
        Register lower_binary_op(AST::BinOp *bin_op);
        Register lower_identifier(AST::Identifier *idNod);

        Global *add_global(const std::string& identifier, MemSize size);
        Register load_global_value(const std::string& identifier);

        Register get_next_reg();
    };

}

#endif //IR_CONTEXT_HPP
