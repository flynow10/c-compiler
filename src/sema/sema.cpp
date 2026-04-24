//
// Created by Natalie Wagner on 4/18/26.
//

#include "sema.hpp"

#include "symbol_table.hpp"
#include "../casting.hpp"
#include "../parsing/ast.hpp"

using namespace AST;

void Sema::acceptAST(Node *ast) {
    if (const auto tu = dyn_cast<TranslationUnit>(ast)) {
        for (const auto & node: *tu) {
            if (isa<Decl>(node)) {
                acceptDecl(cast<Decl>(node.get()));
            } else if (isa<FunctionDecl>(node)) {
                acceptFunctionDecl(cast<FunctionDecl>(node.get()));
            }
            throw std::runtime_error("Translation units must only contain declarations and function declarations.");
        }
    } else {
        throw std::runtime_error("AST must begin with a translation unit.");
    }
}

const std::unordered_map<TypeSpecifier::TypeSpecifierType, SymbolTable::PrimitiveType> TypeSpecMap = {
    {TypeSpecifier::LONG, SymbolTable::Long},
    {TypeSpecifier::INT, SymbolTable::Int},
    {TypeSpecifier::SHORT, SymbolTable::Short},
    {TypeSpecifier::CHAR, SymbolTable::Char},
    {TypeSpecifier::STRUCT, SymbolTable::Struct},
    {TypeSpecifier::VOID, SymbolTable::Void},
};

void Sema::acceptDecl(Decl *decl) {
    SymbolTable::PrimitiveType stype;
    SymbolTable::Entry * ctype = nullptr;
    bool typeMarked = false;
    bool signMarked = false;
    bool isSigned = true;
    bool couldBeForwardDecl = decl->get_num_declarators() == 0;
    for (int i = 0; i < decl->get_num_spec_quals(); ++i) {
        auto *node = decl->get_spec_qual(i);
        if (auto *structSpec = dyn_cast<StructSpecifier>(node)) {
            ctype = acceptStructSpecifier(structSpec, couldBeForwardDecl);
            stype = SymbolTable::Struct;
            typeMarked = true;
        }
        if (auto *typeSpec = dyn_cast<TypeSpecifier>(node)) {

        }
    }
}

SymbolTable::Entry *Sema::acceptStructSpecifier(StructSpecifier *specifier, bool couldBeForwardDecl) {
    if (!specifier->has_struct_name()) {
        throw std::runtime_error("Anonymous structs are not currently supported");
    }
    const auto &identifier = specifier->get_identifier();
    if (specifier->has_declaration()) {
        auto *structDecl = specifier->get_declaration();
        if (isa<StructDeclList>(structDecl)) {
            acceptStructDecl(specifier->get_identifier(), cast<StructDeclList>(structDecl));
        }
    }
    auto *entry = localTable->findStruct(identifier);
    if (!entry) {
        throw std::runtime_error("Could not find struct with identifier \"" + identifier + "\"");
    }
    return entry;
}

void Sema::acceptStructDecl(const std::string &identifier, StructDeclList *declList) {
    auto *entry = localTable->tryFindStruct(identifier);
    if (!entry) {
        entry = localTable->addStruct(identifier);
    }

    if (!entry->incomplete) {
        throw std::runtime_error("Cannot redefine struct with identifier \"" + identifier + "\"");
    }

    for (const auto &node : *declList) {
        if (isa<StructDecl>(node.get())) {
            throw std::runtime_error("Struct decl list must contain Structs");
        }

        auto *structDecl = dyn_cast<StructDecl>(node.get());
        
    }
}
