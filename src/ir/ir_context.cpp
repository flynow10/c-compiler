//
// Created by Natalie Wagner on 5/6/26.
//

#include "ir_context.hpp"

#include "../parsing/ast.hpp"
#include "../sema/sema.hpp"

void IR::IRContext::lower_AST(TranslationUnit *ast) {
    lower_globals(ast);

    for (const auto & node : *ast) {
        if (auto *functionDecl = dyn_cast<FunctionDecl>(node.get())) {
            lower_function(functionDecl);
        }
    }
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

void IR::IRContext::lower_function(FunctionDecl *decl) {
    auto *functionEntry = decl->get_function_entry();
    id_reg_map.clear();
    function_counter = 0;
    current_function = functions.emplace_back(std::make_unique<Function>(functionEntry->identifier)).get();
    current_block = current_function->add_block(0);
    auto *body = decl->get_body();
    lower_statement(body);
}

void IR::IRContext::lower_statement(Statement *stmt) {
    if (auto *compoundStmt = dyn_cast<CompoundStatement>(stmt)) {
        for (const auto & node : *compoundStmt) {
            lower_statement(cast<Statement>(node.get()));
        }
    } else if (auto *decl = dyn_cast<Decl>(stmt)) {
        auto *declList = decl->get_declarators();
        for (int i = 0; i < declList->get_size(); ++i) {
            auto initDecl = cast<InitDeclarator>((*declList)[i]);
            auto *entry = initDecl->get_symbol_entry();
            const Register destReg = get_next_reg();
            id_reg_map[entry->identifier].push_back(destReg);
            const Register exprReg = lower_expression(cast<Expression>(initDecl->get_initializer()));
            current_block->add_instruction<MoveInst>(MoveInst(destReg, exprReg));
        }
    }
}

IR::Register IR::IRContext::lower_expression(Expression *expr) {
    if (auto *binOp = dyn_cast<BinOp>(expr)) {
        return lower_binary_op(binOp);
    }
    if (auto *idNode = dyn_cast<Identifier>(expr)) {
        return lower_identifier(idNode);
    }
}

IR::Register IR::IRContext::lower_binary_op(BinOp *binOp) {
    auto source1 = lower_expression(binOp->get_lhs());
    auto source2 = lower_expression(binOp->get_rhs());
    auto destReg = get_next_reg();
    ArithInst::Operation operation;
    switch (binOp->get_operation()) {
        case BinOp::EQUAL:
        case BinOp::NOT_EQUAL:
        case BinOp::LESS_THAN:
        case BinOp::GREATER_THAN:
        case BinOp::LESS_EQUAL:
        case BinOp::GREATER_EQUAL:
        case BinOp::LEFT_SHIFT:
        case BinOp::RIGHT_SHIFT:
        case BinOp::LOGIC_OR:
        case BinOp::LOGIC_AND:
            throw std::runtime_error("Not implemented");
        case BinOp::INCLUSIVE_OR:
            operation = ArithInst::Operation::OR;
            break;
        case BinOp::EXCLUSIVE_OR:
            operation = ArithInst::Operation::XOR;
            break;
        case BinOp::AND:
            operation = ArithInst::Operation::AND;
            break;
        case BinOp::ADD:
            operation = ArithInst::Operation::ADD;
            break;
        case BinOp::SUB:
            operation = ArithInst::Operation::SUBTRACT;
            break;
        case BinOp::MUL:
            operation = ArithInst::Operation::MULTIPLY;
            break;
        case BinOp::DIV:
            operation = ArithInst::Operation::DIVIDE;
            break;
        case BinOp::MOD:
            operation = ArithInst::Operation::MODULO;
            break;
    }
    current_block->add_instruction(ArithInst(destReg, source1, source2, operation));
    return destReg;
}

IR::Register IR::IRContext::lower_identifier(Identifier *idNode) {
    std::string id = idNode->get_value();
    auto type = idNode->get_type_info();
    if (Sema::is_scalar_type(type.type)) {
        if (id_reg_map.contains(id)) {
            return id_reg_map.at(id).back();
        }

        if (globals.contains(id)) {
            return load_global_value(id);
        }
    }
}

IR::Global * IR::IRContext::add_global(const std::string& identifier, MemSize size) {
    globals[identifier] = std::make_unique<Global>(identifier,global_counter ++, size);
    return globals[identifier].get();
}

IR::Register IR::IRContext::load_global_value(const std::string &identifier) {
    const auto global = globals.at(identifier).get();
    const auto labelReg = get_next_reg();
    const auto valueReg = get_next_reg();
    current_block->add_instruction(LoadLabelInst(labelReg, global->label));
    current_block->add_instruction(LoadInst(valueReg, labelReg));
    return valueReg;
}

IR::Register IR::IRContext::get_next_reg() {
    return reg_counter++;
}
