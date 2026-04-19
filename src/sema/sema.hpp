//
// Created by Natalie Wagner on 4/18/26.
//

#ifndef SEMA_HPP
#define SEMA_HPP
#include <memory>

#include "symbol_table.hpp"
#include "../parsing/ast.hpp"

class Sema {
private:
    symbol_table globalTable = symbol_table(nullptr);
public:
    void acceptAST(AST::Node *ast);
    void acceptDecl(AST::Decl *decl);
    void acceptFunctionDecl(AST::FunctionDecl *functionDecl);
};

#endif //SEMA_HPP
