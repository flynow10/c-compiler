//
// Created by Natalie Wagner on 4/18/26.
//

#include "symbol_table.hpp"

symbol_table::Entry symbol_table::Entry::create(const std::string &identifier, const PrimitiveType stype,
                                                const short indirection, Entry *ctype) {
    Entry entry = { .identifier = identifier, .stype = stype, .indirection = indirection, .ctype = ctype };
    if (indirection != 0) {
        entry.size = 4;
    } else {
        switch (stype) {
            case Long:
                entry.size = 8;
                break;
            case Int:
                entry.size = 4;
                break;
            case Short:
                entry.size = 2;
                break;
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

symbol_table::Entry * symbol_table::addStruct(const std::string &identifier) {
    if (structs.contains(identifier)) {
        throw std::runtime_error("Redefinition of struct with identifier \"" + identifier + "\"");
    }
    structs[identifier] = Entry{ .identifier = identifier, .stype = Struct, .size = 0 };
    return &structs[identifier];
}

symbol_table::Entry * symbol_table::addSymbol(const std::string &identifier, const PrimitiveType stype,
    const short indirection, Entry *ctype) {
    if (symbols.contains(identifier)) {
        throw std::runtime_error("Redefinition of symbol \"" + identifier + "\"");
    }
    symbols[identifier] = Entry::create(identifier, stype, indirection, ctype);
    return &symbols[identifier];
}

symbol_table::Entry * symbol_table::addMember(const std::string &structIdentifier, const std::string &memberIdentifier,
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

symbol_table::Entry * symbol_table::findStruct(const std::string &identifier) {
    if (!structs.contains(identifier)) {
        if (parent_scope != nullptr) {
            return parent_scope->findStruct(identifier);
        }
        throw std::runtime_error("Cannot find struct with identifier \"" + identifier + "\"");
    }

    return &structs[identifier];
}

symbol_table::Entry * symbol_table::findSymbol(const std::string &identifier) {
    if (symbols.contains(identifier)) {
        throw std::runtime_error("Redefinition of symbol \"" + identifier + "\"");
    }
    return &symbols[identifier];
}
