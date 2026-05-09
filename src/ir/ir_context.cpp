//
// Created by Natalie Wagner on 5/6/26.
//

#include "ir_context.hpp"

#include <ranges>
#include <sys/stat.h>

#include "types.hpp"
#include "../parsing/ast.hpp"
#include "../sema/sema.hpp"

void IR::IRContext::lower_AST(const TranslationUnit *ast) {
    lower_globals(ast);

    for (const auto & node : *ast) {
        if (auto *functionDecl = dyn_cast<FunctionDecl>(node.get())) {
            lower_function(functionDecl);
        }
    }
}

void IR::IRContext::lower_globals(const TranslationUnit *ast) {
    auto *symtable = ast->get_symbol_table_raw();
    for (const auto &[id, entry] : symtable->symbols) {
        if (!entry.type.is_function()) {
            MemSize size = entry.type.get_size();
            add_global(id, size);
        }
    }
}

void IR::IRContext::lower_function(const FunctionDecl *decl) {
    auto *functionEntry = decl->get_function_entry();
    id_reg_map.clear();
    function_counter = 0;
    current_function = functions.emplace_back(std::make_unique<Function>(functionEntry->identifier)).get();
    current_block = current_function->add_block(function_counter++);
    auto *body = decl->get_body();
    lower_statement(body);
}

void IR::IRContext::lower_statement(const Statement *stmt) {
    if (auto *compoundStmt = dyn_cast<CompoundStatement>(stmt)) {
        for (const auto & node : *compoundStmt) {
            lower_statement(cast<Statement>(node.get()));
        }
    } else if (auto *decl = dyn_cast<Decl>(stmt)) {
        auto *declList = decl->get_declarators();
        for (int i = 0; i < declList->get_size(); ++i) {
            auto initDecl = cast<InitDeclarator>((*declList)[i]);
            lower_init_decl(initDecl);
        }
    } else if (auto *controlStmt = dyn_cast<ControlStatement>(stmt)) {
        lower_control(controlStmt);
    } else if (auto *selectionStmt = dyn_cast<SelectionStatement>(stmt)) {
        lower_selection(selectionStmt);
    } else if (auto *whileStmt = dyn_cast<WhileStatement>(stmt)) {
        lower_while(whileStmt);
    } else  if (auto *doStmt = dyn_cast<DoStatement>(stmt)) {
        lower_do(doStmt);
    } else if (auto *forStmt = dyn_cast<ForStatement>(stmt)) {
        lower_for(forStmt);
    } else if (auto *expressionStmt = dyn_cast<ExpressionStatement>(stmt)) {
        lower_expression(expressionStmt->get_expression());
    }
}

void IR::IRContext::lower_init_decl(const InitDeclarator *initDecl) {
    auto *entry = initDecl->get_symbol_entry();
    Register::Type type;
    if (entry->type.is_void_type()) {
        return;
    }
    if (entry->type.is_struct()) {
        type = Register::Type::Struct;
    } else if (entry->type.is_array() || entry->type.is_pointer() || entry->type.is_function()) {
        type = Register::Type::Ptr;
    } else {
        type = Register::Type::Int;
    }
    auto identifier = entry->identifier;
    auto findId = [](auto &&id){
        return [&id](auto &&reg) {
            return reg.second.back().name == id;
        };
    };
    if (std::ranges::find_if(id_reg_map, findId(entry->identifier)) != id_reg_map.end()) {
        int count = 0;
        std::string newId = entry->identifier + "." + std::to_string(count);
        while (std::ranges::find_if(id_reg_map, findId(newId)) != id_reg_map.end()) {
            count++;
            newId = identifier + "." + std::to_string(count);
        }
        identifier = newId;
    }
    const auto destReg = Register(identifier, type, entry->type.get_size());
    if (initDecl->has_initializer()) {
        const Register exprReg = lower_expression(cast<Expression>(initDecl->get_initializer()));
        current_block->add_instruction(MoveInst(destReg, exprReg));
    }
    id_reg_map[entry].push_back(destReg);
}

void IR::IRContext::lower_control(const ControlStatement *controlStmt) {
    if (controlStmt->is_return()) {
        if (controlStmt->has_return_expr()) {
            const Register returnReg = lower_expression(controlStmt->get_return_expression());
            current_block->add_instruction(ReturnInst(returnReg));
        } else {
            const Register returnReg = get_next_temp_reg();
            current_block->add_instruction(LoadImmInst(returnReg, 0));
            current_block->add_instruction(ReturnInst(returnReg));
        }
    } else if (controlStmt->is_break()) {
        auto *jumpPoint = loop_end.top();
        current_block->add_instruction(JumpInst(jumpPoint->get_label()));
        jumpPoint->is_preceded_by(current_block);
        // TODO: Consider how adding jump instructions changes block precedence
    } else {
        auto *jumpPoint = loop_begin.top();
        current_block->add_instruction(JumpInst(jumpPoint->get_label()));
        jumpPoint->is_preceded_by(current_block);
    }
}

void IR::IRContext::lower_selection(const SelectionStatement *stmt) {
    const Register conditionReg = lower_condition(stmt->get_condition());
    Block *thenBlock = current_function->add_block(function_counter++, current_block);
    Block *elseBlock;
    if (stmt->has_else()) {
        elseBlock = current_function->add_block(function_counter++, thenBlock);
    }
    Block *afterCondition = current_function->add_block(function_counter++, stmt->has_else() ? elseBlock : thenBlock);
    thenBlock->is_preceded_by(current_block);
    // TODO: Handle conditional blocks correctly
    if (stmt->has_else()) {
        current_block->add_instruction(BreakInst(conditionReg, elseBlock->get_label()));
        elseBlock->is_preceded_by(current_block);
    } else {
        current_block->add_instruction(BreakInst(conditionReg, afterCondition->get_label()));
        afterCondition->is_preceded_by(current_block);
    }

    current_block = thenBlock;
    lower_statement(stmt->get_then());
    afterCondition->is_preceded_by(current_block);
    if (stmt->has_else()) {
        current_block->add_instruction(JumpInst(afterCondition->get_label()));
        current_block = elseBlock;
        lower_statement(stmt->get_else());
        afterCondition->is_preceded_by(current_block);
    }

    current_block = afterCondition;
}

void IR::IRContext::lower_while(const WhileStatement *stmt) {
    Block * body = current_function->add_block(function_counter++, current_block);
    Block * afterLoop = current_function->add_block(function_counter++, body);
    body->is_preceded_by(current_block);
    loop_begin.push(body);
    loop_end.push(afterLoop);

    current_block = body;
    const Register conditionReg = lower_condition(stmt->get_condition());
    current_block->add_instruction(BreakInst(conditionReg, afterLoop->get_label()));
    afterLoop->is_preceded_by(current_block);

    lower_statement(stmt->get_body());
    current_block->add_instruction(JumpInst(body->get_label()));
    body->is_preceded_by(current_block);

    current_block = afterLoop;

    loop_begin.pop();
    loop_end.pop();
}

void IR::IRContext::lower_do(const DoStatement *stmt) {
    Block *body = current_function->add_block(function_counter++, current_block);
    Block *condition = current_function->add_block(function_counter++, body);
    Block *afterLoop = current_function->add_block(function_counter++, condition);
    body->is_preceded_by(current_block);

    loop_begin.push(condition);
    loop_end.push(afterLoop);

    current_block = body;
    lower_statement(stmt->get_body());
    condition->is_preceded_by(current_block);

    current_block = condition;
    const Register conditionReg = lower_condition(stmt->get_condition());
    const Register invertedCondition = get_next_temp_reg();
    current_block->add_instruction(UnaryInst(invertedCondition, conditionReg, UnaryInst::Operation::NEGATE));
    current_block->add_instruction(BreakInst(invertedCondition, body->get_label()));
    body->is_preceded_by(current_block);

    afterLoop->is_preceded_by(current_block);
    current_block = afterLoop;

    loop_begin.pop();
    loop_end.pop();
}

void IR::IRContext::lower_for(const ForStatement *stmt) {
    if (stmt->has_init()) {
        auto *initNode = stmt->get_initialization();
        if (isa<Expression>(initNode)) {
            lower_expression(cast<Expression>(initNode));
        } else {
            lower_statement(cast<Statement>(initNode));
        }
    }

    Block *body = current_function->add_block(function_counter++, current_block);
    Block *increment = current_function->add_block(function_counter++, body);
    Block *afterLoop = current_function->add_block(function_counter++, increment);
    body->is_preceded_by(current_block);

    loop_begin.push(increment);
    loop_end.push(afterLoop);

    current_block = body;
    if (stmt->has_condition()) {
        const Register conditionReg = lower_condition(stmt->get_condition());
        current_block->add_instruction(BreakInst(conditionReg, afterLoop->get_label()));
        afterLoop->is_preceded_by(body);
    }
    lower_statement(stmt->get_body());

    increment->is_preceded_by(current_block);
    current_block = increment;
    if (stmt->has_increment()) {
        lower_expression(stmt->get_increment());
    }
    current_block->add_instruction(JumpInst(body->get_label()));
    body->is_preceded_by(current_block);

    current_block = afterLoop;

    loop_begin.pop();
    loop_end.pop();
}

IR::Register IR::IRContext::lower_condition(const Expression *expr) {
    // TODO: Implement proper expression lowering
    return lower_expression(expr);
}

IR::Register IR::IRContext::lower_expression(const Expression *expr) {
    if (auto *assignment = dyn_cast<Assignment>(expr)) {
        return lower_assignment(assignment);
    }
    if (auto *binOp = dyn_cast<BinOp>(expr)) {
        return lower_binary_op(binOp);
    }
    if (auto *idNode = dyn_cast<Identifier>(expr)) {
        return lower_identifier(idNode);
    }
    if (auto *constantNode = dyn_cast<Constant>(expr)) {
        return lower_constant(constantNode);
    }
    throw std::runtime_error("Not implemented");
}

IR::Register IR::IRContext::lower_assignment(const Assignment *assignment) {
    if (assignment->get_operation() != Assignment::EQUAL) {
        throw std::runtime_error("Not implemented");
    }
    Register assignTo = lower_expression(assignment->get_lhs());
    Register expr = lower_expression(assignment->get_rhs());
    current_block->add_instruction(MoveInst(assignTo, expr));
    return assignTo;
}

IR::Register IR::IRContext::lower_binary_op(const BinOp *binOp) {
    auto source1 = lower_expression(binOp->get_lhs());
    auto source2 = lower_expression(binOp->get_rhs());
    auto destReg = get_next_temp_reg();
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

IR::Register IR::IRContext::lower_identifier(const Identifier *idNode) {
    const std::string& id = idNode->get_value();
    auto type = idNode->get_type_info();

    if (Sema::is_scalar_type(type.type)) {
        if (id_reg_map.contains(idNode->get_symbol_entry())) {
            return id_reg_map.at(idNode->get_symbol_entry()).back();
        }

        if (globals.contains(id)) {
            return load_global_value(id);
        }
    }
    throw std::runtime_error("Not implemented");
}

bool parse_int(const std::string& str, int& output) {
    try {
        // Ensure full string is used in converting to a number
        size_t pos;
        output = std::stoi(str, &pos, 10);
        if (pos != str.length()) {
            return false;
        }
        return true;
    } catch ([[maybe_unused]] std::invalid_argument& e) {
        return false;
    }
}

IR::Register IR::IRContext::lower_constant(const Constant *constantNode) {
    auto &valueStr = constantNode->get_value();
    // TODO: Need better constant parsing
    int value;
    parse_int(valueStr,value);
    Register destReg = get_next_temp_reg();
    current_block->add_instruction(LoadImmInst(destReg, value));
    return destReg;
}

IR::Global * IR::IRContext::add_global(const std::string& identifier, MemSize size) {
    globals[identifier] = std::make_unique<Global>(identifier,global_counter ++, size);
    return globals[identifier].get();
}

IR::Register IR::IRContext::load_global_value(const std::string &identifier) {
    const auto global = globals.at(identifier).get();
    const auto labelReg = get_next_temp_reg();
    const auto valueReg = get_next_temp_reg();
    current_block->add_instruction(LoadLabelInst(labelReg, global->label));
    current_block->add_instruction(LoadInst(valueReg, labelReg));
    return valueReg;
}

IR::Register IR::IRContext::get_next_temp_reg(const Register::Type type, const MemSize size) {
    return {std::to_string(temp_reg_counter++), type, size};
}

std::ostream & IR::operator<<(std::ostream &os, const IRContext &ctx) {
    for (const auto &global: ctx.globals | std::views::values) {
        os << *global << std::endl;
    }
    os << std::endl;
    for (const auto & function : ctx.functions) {
        os << *function << std::endl;
    }
    return os;
}
