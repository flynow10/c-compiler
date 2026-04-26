//
// Created by Natalie Wagner on 4/18/26.
//

#ifndef SEMA_HPP
#define SEMA_HPP
#include <memory>

#include "symbol_table.hpp"
#include "symbol_table.hpp"
#include "../parsing/ast.hpp"

using namespace AST;

class Sema {
private:
    SymbolTable globalTable = SymbolTable(nullptr);
    SymbolTable *localTable = &globalTable;
public:
    void acceptAST(Node *ast);
    void acceptDecl(Decl *decl);
    SymbolTable::Entry acceptDeclSpecifiers(DeclSpecifiers *specifiers, bool couldBeForwardDecl);
    SymbolTable::Entry *acceptStructSpecifier(StructSpecifier *specifier, bool couldBeForwardDecl);
    void acceptStructDecl(const std::string &identifier, StructDeclList *declList);
    void acceptInitDeclarator(InitDeclarator *initDeclarator, const SymbolTable::Entry &entryPrototype);
    SymbolTable::Entry acceptDeclarator(Declarator *declarator, SymbolTable::Entry entryPrototype);
    void acceptFunctionDecl(FunctionDecl *functionDecl);
};

#endif //SEMA_HPP
