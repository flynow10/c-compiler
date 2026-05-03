//
// Created by Natalie Wagner on 4/18/26.
//

#include "sema.hpp"

#include "../casting.hpp"
#include "../parsing/ast.hpp"

using namespace AST;

void Sema::accept_ast(Node *ast) {
    if (const auto tu = dyn_cast<TranslationUnit>(ast)) {
        tu->set_symbol_table(std::make_shared<SymbolTable>(nullptr));
        global_table = tu->get_symbol_table_raw();
        local_table = global_table;
        for (const auto & node: *tu) {
            if (isa<Decl>(node)) {
                accept_decl(cast<Decl>(node.get()));
            } else if (isa<FunctionDecl>(node)) {
                accept_function_decl(cast<FunctionDecl>(node.get()));
            } else {
                throw std::runtime_error("Translation units must only contain declarations and function declarations.");
            }
        }
    } else {
        throw std::runtime_error("AST must begin with a translation unit.");
    }
}

const std::unordered_map<TypeSpecifier::TypeSpecifierType, PrimitiveType> TypeSpecMap = {
    {TypeSpecifier::LONG, Long},
    {TypeSpecifier::INT, Int},
    {TypeSpecifier::SHORT, Short},
    {TypeSpecifier::CHAR, SignedChar},
    {TypeSpecifier::STRUCT, Struct},
    {TypeSpecifier::VOID, Void},
};

void Sema::accept_decl(Decl *decl) {
    auto *declaratorList = cast<InitDeclaratorList>(decl->get_declarators());
    bool couldBeForwardDecl = declaratorList->get_size() == 0;
    auto declSpecs = cast<DeclSpecifiers>(decl->get_decl_specs());
    auto type = accept_decl_specifiers(cast<DeclSpecifiers>(declSpecs), couldBeForwardDecl);

    for (int i = 0; i < declaratorList->get_size(); ++i) {
        auto *initDeclarator = (*declaratorList)[i];
        assert(isa<InitDeclarator>(initDeclarator));
        SymbolTable::Entry entry = accept_init_declarator(cast<InitDeclarator>(initDeclarator), type);
        local_table->addSymbol(entry);
    }
}

PrimitiveType convertFromTypeSpec(TypeSpecifier::TypeSpecifierType type, bool isSigned) {
    switch (type) {
        case TypeSpecifier::VOID:
            return Void;
        case TypeSpecifier::CHAR:
            if (isSigned)
                return SignedChar;
            return UnsignedChar;
        case TypeSpecifier::SHORT:
            if (isSigned)
                return Short;
            return UnsignedShort;
        case TypeSpecifier::INT:
            if (isSigned)
                return Int;
            return UnsignedInt;
        case TypeSpecifier::LONG:
            if (isSigned)
                return Long;
            return UnsignedLong;
        case TypeSpecifier::STRUCT:
            return Struct;
        default:
            throw std::runtime_error("Unexpected type specifier type");
    }
}

void throwDeclareMultipleTypes() noexcept(false) {
    throw std::runtime_error("Cannot currently specify multiple types in one declaration");
}

QualType Sema::accept_decl_specifiers(DeclSpecifiers *specifiers, bool couldBeForwardDecl = false) {
    Type *type = nullptr;
    std::optional<TypeSpecifier::TypeSpecifierType> sType;
    bool signMarked = false;
    bool isSigned = true;
    bool isConst = false;

    for (int i = 0; i < specifiers->get_size(); ++i) {
        auto *node = specifiers->operator[](i);
        if (auto *structSpec = dyn_cast<StructSpecifier>(node)) {
            if (sType.has_value()) {
                throwDeclareMultipleTypes();
            }
            type = accept_struct_specifier(structSpec, couldBeForwardDecl);
            sType = TypeSpecifier::STRUCT;
            continue;
        }
        if (const auto *typeSpec = dyn_cast<TypeSpecifier>(node)) {
            const auto typeSpecType = typeSpec->get_type();
            if (typeSpecType == TypeSpecifier::UNSIGNED || typeSpecType == TypeSpecifier::SIGNED) {
                if (signMarked) {
                    throw std::runtime_error("Declarations cannot contain multiple signedness specifiers");
                }
                isSigned = typeSpecType == TypeSpecifier::SIGNED;
                signMarked = true;
                continue;
            }

            if (sType.has_value()) {
                throwDeclareMultipleTypes();
            }
            sType = typeSpecType;
            continue;
        }
        if (isa<TypeQualifier>(node)) {
            if (isConst) {
                throw std::runtime_error("Declarations cannot contain multiple const qualifiers");
            }
            isConst = true;
        }
    }

    if (!sType.has_value()) {
        throw std::runtime_error("At least one type specifier must be included in a declaration");
    }

    if (signMarked && (sType == Struct || sType == Void)) {
        throw std::runtime_error("Only integer types can be marked as signed or unsigned");
    }

    if (type != nullptr) {
        return {type, isConst};
    }
    return {Type::get(*this, convertFromTypeSpec(sType.value(), isSigned)), isConst};
}

StructType *Sema::accept_struct_specifier(StructSpecifier *specifier, bool couldBeForwardDecl) {
    if (specifier->has_declaration()) {
        auto *structDecl = cast<StructDeclList>(specifier->get_declaration());
        auto *structType = accept_struct_decl(cast<StructDeclList>(structDecl));
        if (specifier->has_struct_name()) {
            const auto &identifier = specifier->get_identifier();
            SymbolTable::Entry *entry = local_table->tryFindStruct(identifier);
            if (!entry) {
                entry = local_table->addStruct(identifier);
            }

            if (entry->is_complete) {
                throw std::runtime_error("Cannot redefine struct with identifier \"" + identifier + "\"");
            }

            entry->type = {structType, false};
        }
        return structType;
    }

    const auto &identifier = specifier->get_identifier();
    if (couldBeForwardDecl) {
        local_table->addStruct(identifier);
        return nullptr;
    }
    auto *entry = local_table->findStruct(identifier);
    if (!entry) {
        throw std::runtime_error("Could not find struct with identifier \"" + identifier + "\"");
    }
    assert(isa<StructType>(entry->type.type));
    return cast<StructType>(entry->type.type);
}

StructType *Sema::accept_struct_decl(StructDeclList *declList) {
    StructType::MemberMap members;
    for (const auto &node : *declList) {
        const auto *structDecl = cast<StructDecl>(node.get());

        auto entryParams = accept_decl_specifiers(cast<DeclSpecifiers>(structDecl->get_spec_qual()));
        auto *declarator = cast<Declarator>(structDecl->get_declarator());
        SymbolTable::Entry member = accept_declarator(declarator, entryParams);

        if (members.contains(member.identifier)) {
            throw std::runtime_error("Redefinition of struct member \"" + member.identifier + "\"");
        }

        members[member.identifier] = member.type;
    }

    return StructType::get(*this, members);
}

SymbolTable::Entry Sema::accept_init_declarator(InitDeclarator *initDeclarator, QualType type) {
    return accept_declarator(cast<Declarator>(initDeclarator->get_declarator()), type);
}

// Assume declarator is not abstract
SymbolTable::Entry Sema::accept_declarator(Declarator *declarator, QualType type) {
    assert(!declarator->is_abstract());
    SymbolTable::Entry entry = {.type = type};
    auto *directDeclarator = declarator->get_direct_declarator();
    if (isa<DirectDeclarator>(directDeclarator)) {
        entry.identifier = cast<DirectDeclarator>(directDeclarator)->get_identifier();
    } else {
        assert(isa<Declarator>(directDeclarator));
        entry = accept_declarator(cast<Declarator>(directDeclarator), type);
    }

    if (declarator->has_pointer()) {
        auto *pointer = cast<AST::Pointer>(declarator->get_pointer());
        do {
            type = {.type = PointerType::get(*this, type), .is_const = pointer->is_const()};
            pointer = pointer->get_next_pointer();
        }
        while (pointer != nullptr);
        entry.type = type;
    }

    // TODO: Handle suffixes

    return entry;
}

void Sema::accept_function_decl(FunctionDecl *functionDecl) {
    auto *declSpecs = functionDecl->get_spec_quals();
    auto *declarator = functionDecl->get_declarator();

    const auto entryPrototype = accept_decl_specifiers(declSpecs);
    const auto entry = accept_declarator(declarator, entryPrototype);
    local_table->addSymbol(entry);

    auto body = functionDecl->get_body();
    accept_compound_statement(body);
}

void Sema::accept_compound_statement(CompoundStatement *compound_statement) {
    compound_statement->set_symbol_table(std::make_shared<SymbolTable>(local_table));
    local_table = compound_statement->get_symbol_table_raw();

    for (auto &node : *compound_statement) {
        assert(isa<Statement>(node.get()));
        const auto statement = cast<Statement>(node.get());
        accept_statement(statement);
    }

    local_table = local_table->parent_scope;
}

void Sema::accept_statement(Statement *statement) {
    if (auto *compoundStmt = dyn_cast<CompoundStatement>(statement)) {
        accept_compound_statement(compoundStmt);
        return;
    }

    if (auto *exprStmt = dyn_cast<ExpressionStatement>(statement)) {
        accept_expression(exprStmt->get_expression());
        return;
    }

    if (auto *declStmt = dyn_cast<Decl>(statement)) {
        accept_decl(declStmt);
        return;
    }
}
