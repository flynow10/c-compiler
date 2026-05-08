//
// Created by Natalie Wagner on 4/18/26.
//

#include "sema.hpp"

Entry Entry::create(const std::string &identifier, QualType type) {
    return { .identifier = identifier, .type = type };
}

Entry * SymbolTable::addStruct(const std::string &identifier) {
    if (structs.contains(identifier)) {
        throw std::runtime_error("Redefinition of struct with identifier \"" + identifier + "\"");
    }
    structs[identifier] = Entry{ .identifier = identifier, .is_complete = false, };
    return &structs[identifier];
}

Entry * SymbolTable::addSymbol(const Entry &entry) {
    if (symbols.contains(entry.identifier)) {
        throw std::runtime_error("Redefinition of symbol \"" + entry.identifier + "\"");
    }
    symbols[entry.identifier] = entry;
    return &symbols[entry.identifier];
}

Entry * SymbolTable::addSymbol(const std::string &identifier, QualType type) {
    return addSymbol(Entry::create(identifier, type));
}

Entry * SymbolTable::tryFindStruct(const std::string &identifier) {

    if (!structs.contains(identifier)) {
        if (parent_scope != nullptr) {
            return parent_scope->findStruct(identifier);
        }
        return nullptr;
    }

    return &structs[identifier];
}

Entry * SymbolTable::tryFindSymbol(const std::string &identifier) {
    if (!symbols.contains(identifier)) {
        if (parent_scope != nullptr) {
            return parent_scope->findSymbol(identifier);
        }
        return nullptr;
    }
    return &symbols[identifier];
}

Entry * SymbolTable::findStruct(const std::string &identifier) {
    Entry * entry = tryFindStruct(identifier);
    if (!entry) {
        throw std::runtime_error("Cannot find struct with identifier \"" + identifier + "\"");
    }
    return entry;
}

Entry * SymbolTable::findSymbol(const std::string &identifier) {
    Entry * entry = tryFindSymbol(identifier);
    if (!entry) {
        throw std::runtime_error("Cannot find symbol with identifier \"" + identifier + "\"");
    }
    return entry;
}

bool SymbolTable::isLocallyDefinedSymbol(const std::string &identifier) const {
    return symbols.contains(identifier);
}

bool SymbolTable::isLocallyDefinedStruct(const std::string &identifier) const {
    return structs.contains(identifier);
}

bool SymbolTable::isInLoop() const {
    if (table_type == Loop) {
        return true;
    }
    if (parent_scope != nullptr) {
        return parent_scope->isInLoop();
    }
    return false;
}

bool SymbolTable::isInFunction() const {
    if (table_type == Function) {
        return true;
    }
    if (parent_scope != nullptr) {
        return parent_scope->isInFunction();
    }
    return false;
}
