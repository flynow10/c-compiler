//
// Created by Natalie Wagner on 5/6/26.
//

#ifndef IR_CONTEXT_HPP
#define IR_CONTEXT_HPP

#include <ostream>

#include "function.hpp"
#include "global.hpp"
#include "types.hpp"

#include "../parsing/ast.hpp"


namespace IR {

    class IRContext {
        Label global_counter = 0;
        Label function_counter = 0;
        unsigned int temp_reg_counter = 0;

        std::map<std::string, std::unique_ptr<Global>> globals;
        std::vector<std::unique_ptr<Function>> functions;

        Function * current_function = nullptr;
        Block * current_block = nullptr;

        std::stack<Block *> loop_begin;
        std::stack<Block *> loop_end;
        std::unordered_map<const Entry *, std::vector<Register>> id_reg_map;

    public:
        [[nodiscard]] const size_t get_num_functions() const;
        [[nodiscard]] const Function * get_function(size_t index) const;

    public:
        void lower_AST(const AST::TranslationUnit * ast);
        void lower_globals(const AST::TranslationUnit * ast);
        void lower_function(const AST::FunctionDecl *decl);
        void lower_statement(const AST::Statement *stmt);
        void lower_init_decl(const AST::InitDeclarator *initDecl);
        void lower_control(const AST::ControlStatement *controlStmt);
        void lower_selection(const AST::SelectionStatement *stmt);
        void lower_while(const AST::WhileStatement *stmt);
        void lower_do(const AST::DoStatement *stmt);
        void lower_for(const AST::ForStatement *stmt);

        Register lower_condition(const AST::Expression *expr);
        Register lower_expression(const AST::Expression *expr);
        Register lower_assignment(const AST::Assignment *assignment);
        Register lower_binary_op(const AST::BinOp *bin_op);
        Register lower_unary_op(const AST::UnaryOp *unary_op);
        Register lower_post_assignment(const AST::PostAssignment *assignment);
        Register lower_identifier(const AST::Identifier *idNod);
        Register lower_constant(const AST::Constant *constantNode);

        Global *add_global(const std::string& identifier, MemSize size);
        Register load_global_value(const std::string& identifier);

        Register get_next_temp_reg(Register::Type type = Register::Type::Int, MemSize size = 4);

        friend std::ostream & operator<<(std::ostream &os, const IRContext &ctx);
    };

}

#endif //IR_CONTEXT_HPP
