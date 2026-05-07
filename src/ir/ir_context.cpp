//
// Created by Natalie Wagner on 5/6/26.
//

#include "ir_context.hpp"

#include "../parsing/ast.hpp"
#include "../sema/sema.hpp"

void IR::IRContext::lower_AST(TranslationUnit *ast) {
    lower_globals(ast);
}

void IR::IRContext::lower_globals(TranslationUnit *ast) {
    auto *symtable = ast->get_symbol_table_raw();
    for (const auto &[id, entry] : symtable->symbols) {
        if (!entry.type.is_function()) {
            MemSize size = entry.type.get_size();
            add_global(id, size);
        }
    }
}

IR::Global * IR::IRContext::add_global(std::string identifier, MemSize size) {
    auto &global = globals.emplace_back(std::make_unique<Global>(identifier,global_counter ++, size));
    return global.get();
}
