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
    auto *declaratorList = cast<InitDeclaratorList>(decl->get_declarators());
    bool couldBeForwardDecl = declaratorList->get_size() == 0;
    auto declSpecs = cast<DeclSpecifiers>(decl->get_decl_specs());
    auto entryPrototype = acceptDeclSpecifiers(cast<DeclSpecifiers>(declSpecs), couldBeForwardDecl);

    for (int i = 0; i < declaratorList->get_size(); ++i) {
        auto *initDeclarator = (*declaratorList)[i];
        assert(isa<InitDeclarator>(initDeclarator));
        acceptInitDeclarator(cast<InitDeclarator>(initDeclarator), entryPrototype);
    }
}

SymbolTable::PrimitiveType convertFromTypeSpec(TypeSpecifier::TypeSpecifierType type, bool isSigned) {
    switch (type) {
        case TypeSpecifier::VOID:
            return SymbolTable::Void;
        case TypeSpecifier::CHAR:
            if (isSigned)
                return SymbolTable::Char;
            return SymbolTable::UnsignedChar;
        case TypeSpecifier::SHORT:
            if (isSigned)
                return SymbolTable::Short;
            return SymbolTable::UnsignedShort;
        case TypeSpecifier::INT:
            if (isSigned)
                return SymbolTable::Int;
            return SymbolTable::UnsignedInt;
        case TypeSpecifier::LONG:
            if (isSigned)
                return SymbolTable::Long;
            return SymbolTable::UnsignedLong;
        case TypeSpecifier::STRUCT:
            return SymbolTable::Struct;
        default:
            throw std::runtime_error("Unexpected type specifier type");
    }
}

void throwDeclareMultipleTypes() noexcept(false) {
    throw std::runtime_error("Cannot currently specify multiple types in one declaration");
}

SymbolTable::Entry Sema::acceptDeclSpecifiers(DeclSpecifiers *specifiers, bool couldBeForwardDecl = false) {
    std::optional<SymbolTable::PrimitiveType> stype;
    SymbolTable::Entry * ctype = nullptr;
    bool typeMarked = false;
    bool signMarked = false;
    bool isSigned = true;
    bool isStatic = false;
    for (int i = 0; i < specifiers->get_size(); ++i) {
        auto *node = specifiers->operator[](i);
        if (auto *structSpec = dyn_cast<StructSpecifier>(node)) {
            if (typeMarked) {
                throwDeclareMultipleTypes();
            }
            ctype = acceptStructSpecifier(structSpec, couldBeForwardDecl);
            stype = SymbolTable::Struct;
            typeMarked = true;
            continue;
        }
        if (const auto *typeSpec = dyn_cast<TypeSpecifier>(node)) {
            const auto type = typeSpec->get_type();
            if (type == TypeSpecifier::UNSIGNED || type == TypeSpecifier::SIGNED) {
                if (signMarked) {
                    throw std::runtime_error("Declarations cannot contain multiple signedness specifiers");
                }
                isSigned = type == TypeSpecifier::SIGNED;
                signMarked = true;
                continue;
            }

            if (typeMarked) {
                throwDeclareMultipleTypes();
            }
            stype = convertFromTypeSpec(type, isSigned);
            typeMarked = true;
            continue;
        }
        if (isa<TypeQualifier>(node)) {
            if (isStatic) {
                throw std::runtime_error("Declarations cannot contain multiple static qualifiers");
            }
            isStatic = true;
        }
    }
    if (!stype.has_value()) {
        throw std::runtime_error("At least one type specifier must be included in a declaration");
    }

    if (signMarked && (stype == SymbolTable::PrimitiveType::Struct || stype == SymbolTable::PrimitiveType::Void)) {
        throw std::runtime_error("Only integer types can be marked as signed or unsigned");
    }

    return {.stype = stype.value(), .ctype = ctype, .is_static = isStatic};
}

SymbolTable::Entry *Sema::acceptStructSpecifier(StructSpecifier *specifier, bool couldBeForwardDecl) {
    if (!specifier->has_struct_name()) {
        throw std::runtime_error("Anonymous structs are not currently supported");
    }
    const auto &identifier = specifier->get_identifier();
    if (specifier->has_declaration()) {
        if (auto *structDecl = dyn_cast<StructDeclList>(specifier->get_declaration())) {
            acceptStructDecl(specifier->get_identifier(), cast<StructDeclList>(structDecl));
        }
    } else if (couldBeForwardDecl) {
        localTable->addStruct(identifier);
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
        const auto *structDecl = cast<StructDecl>(node.get());

        auto entryParams = acceptDeclSpecifiers(cast<DeclSpecifiers>(structDecl->get_spec_qual()));
        auto *declarator = cast<Declarator>(structDecl->get_declarator());
        SymbolTable::Entry member = acceptDeclarator(declarator, entryParams);
        entry->members.emplace(member.identifier, member);
    }
}

void Sema::acceptInitDeclarator(InitDeclarator *initDeclarator, const SymbolTable::Entry &entryPrototype) {
    acceptDeclarator(cast<Declarator>(initDeclarator->get_declarator()), entryPrototype);

}

// Assume declarator is not abstract
SymbolTable::Entry Sema::acceptDeclarator(Declarator *declarator, SymbolTable::Entry entryPrototype) {
    assert(!declarator->is_abstract());

    auto *directDeclarator = declarator->get_direct_declarator();
    if (isa<DirectDeclarator>(directDeclarator)) {
        entryPrototype.identifier = cast<DirectDeclarator>(directDeclarator)->get_identifier();
    } else {
        assert(isa<Declarator>(directDeclarator));
        entryPrototype = acceptDeclarator(cast<Declarator>(directDeclarator), entryPrototype);
    }

    if (declarator->has_pointer()) {
        auto *pointer = cast<Pointer>(declarator->get_pointer());
        entryPrototype.indirection ++;
        while (pointer->has_next_pointer()) {
            entryPrototype.indirection ++;
            pointer = pointer->get_next_pointer();
        }
    }

    return entryPrototype;
}
