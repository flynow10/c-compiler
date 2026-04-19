//
// Created by Natalie Wagner on 4/18/26.
//

#ifndef SYMBOLTABLE_HPP
#define SYMBOLTABLE_HPP
#include <map>
#include <string>
#include <vector>


struct symbol_table {
public:
    enum PrimitiveType {
        Void,
        Long,
        Int,
        Short,
        Char,
        Struct,
    };
    struct Entry {
    public:
        std::string identifier;
        PrimitiveType stype;
        short indirection = 0;
        // Used to reference struct types
        Entry *ctype = nullptr;
        std::map<std::string, Entry> members;
        int size = 0;

        Entry *findMember(const std::string &member) {
            if (!members.contains(member)) {
                throw std::runtime_error("Identifier \"" + member + "\" is not a member of \"" + this->identifier + "\"");
            }

            return &members[member];
        }

        static Entry create(const std::string &identifier, PrimitiveType stype, short indirection, Entry *ctype);
    };

    symbol_table(symbol_table &) = delete;
    symbol_table & operator=(const symbol_table &) = delete;

    std::map<std::string, Entry> structs;
    std::map<std::string, Entry> symbols;
    symbol_table *parent_scope = nullptr;

    explicit symbol_table(symbol_table * parent) : parent_scope(parent) {
    }


    Entry *addStruct(const std::string &identifier);

    Entry *addSymbol(const std::string &identifier, PrimitiveType stype, short indirection, Entry *ctype);

    Entry *addMember(const std::string &structIdentifier, const std::string &memberIdentifier, PrimitiveType stype, short indirection, Entry *ctype);

    Entry *findStruct(const std::string &identifier);

    Entry *findSymbol(const std::string &identifier);
};



#endif //SYMBOLTABLE_HPP
