//
// Created by Natalie Wagner on 4/18/26.
//

#include "symbol_table.hpp"

SymbolTable::Entry SymbolTable::Entry::create(const std::string &identifier, const PrimitiveType stype,
                                                const short indirection, Entry *ctype) {
    Entry entry = { .identifier = identifier, .stype = stype, .indirection = indirection, .ctype = ctype };
    if (indirection != 0) {
        entry.size = 4;
    } else {
        switch (stype) {
            case UnsignedLong:
            case Long:
                entry.size = 8;
                break;
            case UnsignedInt:
            case Int:
                entry.size = 4;
                break;
            case UnsignedShort:
            case Short:
                entry.size = 2;
                break;
            case UnsignedChar:
            case Char:
                entry.size = 1;
                break;
            case Struct:
                if (ctype == nullptr) {
                    throw std::runtime_error("Struct definitions must have a reference to the struct type.");
                }
                entry.size = ctype->size;
                break;
            case Void:
                throw std::runtime_error("Void type must be an pointer.");
        }
    }
    return entry;
}

SymbolTable::Entry * SymbolTable::addStruct(const std::string &identifier) {
    if (structs.contains(identifier)) {
        throw std::runtime_error("Redefinition of struct with identifier \"" + identifier + "\"");
    }
    structs[identifier] = Entry{ .identifier = identifier, .stype = Struct, .incomplete = true, .size = 0 };
    return &structs[identifier];
}

SymbolTable::Entry * SymbolTable::addSymbol(const std::string &identifier, const PrimitiveType stype,
    const short indirection, Entry *ctype) {
    if (symbols.contains(identifier)) {
        throw std::runtime_error("Redefinition of symbol \"" + identifier + "\"");
    }
    symbols[identifier] = Entry::create(identifier, stype, indirection, ctype);
    return &symbols[identifier];
}

SymbolTable::Entry * SymbolTable::addMember(const std::string &structIdentifier, const std::string &memberIdentifier,
    const PrimitiveType stype, const short indirection, Entry *ctype) {
    if (!structs.contains(structIdentifier)) {
        throw std::runtime_error("Cannot add member to struct which does not exist \"" + structIdentifier + "\"");
    }
    Entry & structEntry = structs[structIdentifier];

    if (structEntry.members.contains(memberIdentifier)) {
        throw std::runtime_error("Redefinition of member \"" + memberIdentifier + "\" in struct \"" + structIdentifier + "\"");
    }

    const Entry member = Entry::create(memberIdentifier, stype, indirection, ctype);
    structEntry.members[memberIdentifier] = member;
    structEntry.size += member.size;
    return &structEntry.members[memberIdentifier];
}

SymbolTable::Entry * SymbolTable::tryFindStruct(const std::string &identifier) {

    if (!structs.contains(identifier)) {
        if (parent_scope != nullptr) {
            return parent_scope->findStruct(identifier);
        }
        return nullptr;
    }

    return &structs[identifier];
}

SymbolTable::Entry * SymbolTable::tryFindSymbol(const std::string &identifier) {
    if (!symbols.contains(identifier)) {
        if (parent_scope != nullptr) {
            return parent_scope->findSymbol(identifier);
        }
        return nullptr;
    }
    return &symbols[identifier];
}

SymbolTable::Entry * SymbolTable::findStruct(const std::string &identifier) {
    Entry * entry = tryFindStruct(identifier);
    if (!entry) {
        throw std::runtime_error("Cannot find struct with identifier \"" + identifier + "\"");
    }
    return entry;
}

SymbolTable::Entry * SymbolTable::findSymbol(const std::string &identifier) {
    Entry * entry = tryFindSymbol(identifier);
    if (!entry) {
        throw std::runtime_error("Cannot find symbol with identifier \"" + identifier + "\"");
    }
    return entry;
}
