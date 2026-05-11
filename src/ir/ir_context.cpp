//
// Created by Natalie Wagner on 5/6/26.
//

#include "ir_context.hpp"

#include <ranges>

#include "types.hpp"
#include "../parsing/ast.hpp"
#include "../sema/sema.hpp"

size_t IR::IRContext::get_num_functions() const {
    return functions.size();
}

const IR::Function * IR::IRContext::get_function(const size_t index) const {
    return functions.at(index).get();
}

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
    auto *functionTable = decl->get_symbol_table_raw();
    auto *functionEntry = decl->get_function_entry();
    id_reg_map.clear();
    function_counter = 0;
    temp_reg_counter = 0;
    current_function = functions.emplace_back(std::make_unique<Function>(functionEntry->identifier)).get();
    current_block = current_function->add_block(function_counter++);

    if (decl->_has_parameter_list()) {
        auto *parameters = decl->_get_parameter_list();

        for (int i = 0; i < parameters->get_size(); ++i) {
            auto *parameter = (*parameters)[i];
            // TODO: Proper register typing and sizing
            Register::Type type;
            auto parameterName = parameter->get_declarator()->get_identifier();
            const Entry * argEntry = functionTable->findSymbol(parameterName);
            if (argEntry->type.is_integer()) {
                type = Register::Type::Int;
            } else {
                type = Register::Type::Ptr;
            }
            const auto *argReg = current_function->add_argument({parameterName, type, 4});
            auto argPtr = get_next_temp_reg(Register::Type::Ptr);
            // Move parameter to stack allocation
            current_block->add_instruction(AllocateInst::create(argPtr, argEntry->type.get_size()));
            current_block->add_instruction(StoreInst::create(argPtr, RegisterArgument::create(*argReg)));
            id_reg_map[argEntry].push_back(argPtr);
        }
    }

    auto *body = decl->get_body();
    lower_statement(body);

    if (!isa<ReturnInst>(current_block->get_instructions().back().get())) {
        current_block->add_instruction(ReturnInst::create(ImmediateArgument::create(0, 0)));
    }
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

std::string IR::IRContext::get_next_available_reg_name(const std::string &identifier) {
    auto findId = [](auto &&id){
        return [&id](auto &&reg) {
            return reg.second.back().name == id;
        };
    };
    std::string output = identifier;
    if (std::ranges::find_if(id_reg_map, findId(identifier)) != id_reg_map.end()) {
        int count = 0;
        std::string newId = identifier + "." + std::to_string(count);
        while (std::ranges::find_if(id_reg_map, findId(newId)) != id_reg_map.end()) {
            count++;
            newId = output + "." + std::to_string(count);
        }
        output = newId;
    }
    return output;
}

void IR::IRContext::lower_init_decl(const InitDeclarator *initDecl) {
    auto *entry = initDecl->get_symbol_entry();
    if (entry->type.is_void_type()) {
        return;
    }

    auto identifier = get_next_available_reg_name(entry->identifier);
    const auto destReg = Register(identifier, Register::Type::Ptr, 4);
    current_block->add_instruction(AllocateInst::create(destReg, entry->type.get_size()));

    if (initDecl->has_initializer()) {
        const auto exprReg = lower_expression(cast<Expression>(initDecl->get_initializer()));
        current_block->add_instruction(StoreInst::create(destReg, exprReg.to_argument()));
    }
    id_reg_map[entry].push_back(destReg);
}

void IR::IRContext::lower_control(const ControlStatement *controlStmt) {
    if (controlStmt->is_return()) {
        if (controlStmt->has_return_expr()) {
            const auto returnReg = lower_expression(controlStmt->get_return_expression());
            current_block->add_instruction(ReturnInst::create(returnReg.to_argument()));
        } else {
            current_block->add_instruction(ReturnInst::create(ImmediateArgument::create(0, 4)));
        }
    } else if (controlStmt->is_break()) {
        auto *jumpPoint = loop_end.top();
        current_block->add_instruction(JumpInst::create(LabelArgument::create(jumpPoint->get_label())));
        jumpPoint->is_preceded_by(current_block);
        // TODO: Consider how adding jump instructions changes block precedence
    } else {
        auto *jumpPoint = loop_begin.top();
        current_block->add_instruction(JumpInst::create(LabelArgument::create(jumpPoint->get_label())));
        jumpPoint->is_preceded_by(current_block);
    }
}

void IR::IRContext::lower_selection(const SelectionStatement *stmt) {
    const auto conditionReg = lower_condition(stmt->get_condition());
    Block *thenBlock = current_function->add_block(function_counter++, current_block);
    Block *elseBlock;
    if (stmt->has_else()) {
        elseBlock = current_function->add_block(function_counter++, thenBlock);
    }
    Block *afterCondition = current_function->add_block(function_counter++, stmt->has_else() ? elseBlock : thenBlock);
    thenBlock->is_preceded_by(current_block);
    // TODO: Handle conditional blocks correctly
    if (stmt->has_else()) {
        current_block->add_instruction(BranchInst::create(conditionReg.get_as_register(), LabelArgument::create(elseBlock->get_label())));
        elseBlock->is_preceded_by(current_block);
    } else {
        current_block->add_instruction(BranchInst::create(conditionReg.get_as_register(), LabelArgument::create(afterCondition->get_label())));
        afterCondition->is_preceded_by(current_block);
    }

    current_block = thenBlock;
    lower_statement(stmt->get_then());
    afterCondition->is_preceded_by(current_block);
    if (stmt->has_else()) {
        current_block->add_instruction(JumpInst::create(LabelArgument::create(afterCondition->get_label())));
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
    const auto conditionReg = lower_condition(stmt->get_condition());
    current_block->add_instruction(BranchInst::create(conditionReg.get_as_register(), LabelArgument::create(afterLoop->get_label())));
    afterLoop->is_preceded_by(current_block);

    lower_statement(stmt->get_body());
    current_block->add_instruction(JumpInst::create(LabelArgument::create(body->get_label())));
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
    const auto conditionReg = lower_condition(stmt->get_condition());
    const Register invertedCondition = get_next_temp_reg();
    current_block->add_instruction(UnaryInst::create(invertedCondition, conditionReg.to_argument(), UnaryInst::Operation::NEGATE));
    current_block->add_instruction(BranchInst::create(invertedCondition, LabelArgument::create(body->get_label())));
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
        const auto conditionReg = lower_condition(stmt->get_condition());
        current_block->add_instruction(BranchInst::create(conditionReg.get_as_register(), LabelArgument::create(afterLoop->get_label())));
        afterLoop->is_preceded_by(body);
    }
    lower_statement(stmt->get_body());

    increment->is_preceded_by(current_block);
    current_block = increment;
    if (stmt->has_increment()) {
        lower_expression(stmt->get_increment());
    }
    current_block->add_instruction(JumpInst::create(LabelArgument::create(body->get_label())));
    body->is_preceded_by(current_block);

    current_block = afterLoop;

    loop_begin.pop();
    loop_end.pop();
}

IR::RegOrImmediate IR::IRContext::lower_condition(const Expression *expr) {
    // TODO: Implement proper expression lowering
    return lower_expression(expr);
}

IR::RegOrImmediate IR::IRContext::lower_expression(const Expression *expr) {
    if (auto *assignment = dyn_cast<Assignment>(expr)) {
        return lower_assignment(assignment);
    }
    if (auto *binOp = dyn_cast<BinOp>(expr)) {
        return lower_binary_op(binOp);
    }
    if (auto *unaryOp = dyn_cast<UnaryOp>(expr)) {
        return lower_unary_op(unaryOp);
    }
    if (auto *postAssign = dyn_cast<PostAssignment>(expr)) {
        return lower_post_assignment(postAssign);
    }
    if (auto *idNode = dyn_cast<Identifier>(expr)) {
        return lower_identifier(idNode);
    }
    if (auto *functionCall = dyn_cast<FunctionCall>(expr)) {
        return lower_function_call(functionCall);
    }
    if (auto *constantNode = dyn_cast<Constant>(expr)) {
        return lower_constant(constantNode);
    }
    throw std::runtime_error("Not implemented");
}

IR::RegOrImmediate IR::IRContext::lower_assignment(const Assignment *assignment) {
    auto assignTo = assignment->get_lhs();
    auto assignToPtr = lower_lvalue_ptr(assignTo);
    auto expr = lower_expression(assignment->get_rhs());
    if (assignment->get_operation() != Assignment::EQUAL) {
        auto tempReg = get_next_temp_reg();
        ArithInst::Operation op;
        switch (assignment->get_operation()) {
            case Assignment::ADD:
                op = ArithInst::Operation::ADD;
                break;
            case Assignment::SUB:
                op = ArithInst::Operation::SUBTRACT;
                break;
            case Assignment::MUL:
                op = ArithInst::Operation::MULTIPLY;
                break;
            case Assignment::DIV:
                op = ArithInst::Operation::DIVIDE;
                break;
            case Assignment::LEFT:
                op = ArithInst::Operation::SHIFT_LEFT;
                break;
            case Assignment::RIGHT:
                op = ArithInst::Operation::SHIFT_RIGHT;
                break;
            case Assignment::AND:
                op = ArithInst::Operation::AND;
                break;
            case Assignment::OR:
                op = ArithInst::Operation::OR;
                break;
            case Assignment::XOR:
                op = ArithInst::Operation::XOR;
                break;
            case Assignment::MOD:
                op = ArithInst::Operation::MODULO;
                break;
            default:
                throw std::runtime_error("Unexpected assignment operator here");
        }
        auto prevValue = get_next_temp_reg();
        current_block->add_instruction(LoadInst::create(prevValue, RegisterArgument::create(assignToPtr)));
        current_block->add_instruction(ArithInst::create(tempReg, RegisterArgument::create(prevValue), expr.to_argument(), op));
        expr = RegOrImmediate(tempReg);
    }
    current_block->add_instruction(StoreInst::create(assignToPtr, expr.to_argument()));
    return expr;
}

const std::vector CompareOps = {
    BinOp::EQUAL,
    BinOp::NOT_EQUAL,
    BinOp::LESS_THAN,
    BinOp::GREATER_THAN,
    BinOp::LESS_EQUAL,
    BinOp::GREATER_EQUAL,
};

IR::RegOrImmediate IR::IRContext::lower_binary_op(const BinOp *binOp) {
    auto source1 = lower_expression(binOp->get_lhs());
    auto source2 = lower_expression(binOp->get_rhs());
    auto astOp = binOp->get_operation();

    auto destReg = get_next_temp_reg();
    if (std::ranges::find(CompareOps, astOp) != CompareOps.end()) {
        CompareInst::Operation operation;
        switch (astOp) {
            case BinOp::EQUAL:
                operation = CompareInst::Operation::EQUAL;
                break;
            case BinOp::NOT_EQUAL:
                operation = CompareInst::Operation::NOT_EQUAL;
                break;
            case BinOp::LESS_THAN:
                operation = CompareInst::Operation::LESS_THAN;
                break;
            case BinOp::GREATER_THAN:
                operation = CompareInst::Operation::GREATER_THAN;
                break;
            case BinOp::LESS_EQUAL:
                operation = CompareInst::Operation::LESS_THAN_EQUAL;
                break;
            case BinOp::GREATER_EQUAL:
                operation = CompareInst::Operation::GREATER_THAN_EQUAL;
                break;
            default:
                throw std::runtime_error("Unexpected binary operation");
        }
        current_block->add_instruction(CompareInst::create(destReg, source1.to_argument(), source2.to_argument(), operation));
        return RegOrImmediate(destReg);
    }

    ArithInst::Operation operation;
    switch (astOp) {
        case BinOp::LOGIC_OR:
        case BinOp::LOGIC_AND:
            throw std::runtime_error("Not implemented");
        case BinOp::LEFT_SHIFT:
            operation = ArithInst::Operation::SHIFT_LEFT;
            break;
        case BinOp::RIGHT_SHIFT:
            operation = ArithInst::Operation::SHIFT_RIGHT;
            break;
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
        default:
            throw std::runtime_error("Unexpected binary operation");
    }
    current_block->add_instruction(ArithInst::create(destReg, source1.to_argument(), source2.to_argument(), operation));
    return RegOrImmediate(destReg);
}

IR::RegOrImmediate IR::IRContext::lower_unary_op(const UnaryOp *unary_op) {
    if (unary_op->get_operation() == UnaryOp::ADDRESS_OF) {
        return RegOrImmediate(lower_lvalue_ptr(unary_op->get_rhs()));
    }
    RegOrImmediate expr = lower_expression(unary_op->get_rhs());
    switch (auto operation = unary_op->get_operation()) {
        case UnaryOp::Op::INCREMENT:
        case UnaryOp::DECREMENT: {
            auto exprReg = expr.get_as_register();
            auto assignToPtr = lower_lvalue_ptr(unary_op->get_rhs());
            current_block->add_instruction(ArithInst::create(exprReg, expr.to_argument(), ImmediateArgument::create(1, exprReg.size), operation == UnaryOp::INCREMENT ? ArithInst::Operation::ADD : ArithInst::Operation::SUBTRACT));
            current_block->add_instruction(StoreInst::create(assignToPtr, expr.to_argument()));
            return expr;
        }
        case UnaryOp::NEGATE: {
            Register output = get_next_temp_reg();
            current_block->add_instruction(UnaryInst::create(output, expr.to_argument(), UnaryInst::Operation::NEGATE));
            return RegOrImmediate(output);
        }
        case UnaryOp::POSITIVE:
            return expr;
        case UnaryOp::NEGATIVE: {
            Register output = get_next_temp_reg();
            current_block->add_instruction(
                ArithInst::create(output,
                    ImmediateArgument::create(Immediate(0, expr.get_size())),
                    expr.to_argument(),
                    ArithInst::Operation::SUBTRACT)
                    );
            return RegOrImmediate(output);
        }
        case UnaryOp::INVERT: {
            Register output = get_next_temp_reg();
            current_block->add_instruction(UnaryInst::create(output, expr.to_argument(), UnaryInst::Operation::INVERT));
            return RegOrImmediate(output);
        }
        case UnaryOp::DEREFERENCE:
        case UnaryOp::SIZEOF:
        default:
            throw std::runtime_error("Not implemented");
    }
}

IR::RegOrImmediate IR::IRContext::lower_post_assignment(const PostAssignment *assignment) {
    RegOrImmediate expr = lower_expression(assignment->get_lhs());
    Register temp = get_next_temp_reg();
    auto destReg = lower_lvalue_ptr(assignment->get_lhs());

    auto operation = assignment->get_operation() == PostAssignment::INCREMENT ? ArithInst::Operation::ADD : ArithInst::Operation::SUBTRACT;
    current_block->add_instruction(ArithInst::create(temp, expr.to_argument(), ImmediateArgument::create(1, temp.size), operation));
    current_block->add_instruction(StoreInst::create(destReg, RegisterArgument::create(temp)));

    return expr;
}

IR::RegOrImmediate IR::IRContext::lower_identifier(const Identifier *idNode) {
    const std::string& id = idNode->get_value();
    auto type = idNode->get_type_info();

    if (Sema::is_scalar_type(type.type)) {
        if (id_reg_map.contains(idNode->get_symbol_entry())) {
            const Register ptr = id_reg_map.at(idNode->get_symbol_entry()).back();
            const Register value = get_next_temp_reg();
            current_block->add_instruction(LoadInst::create(value, RegisterArgument::create(ptr)));
            return RegOrImmediate(value);
        }

        if (globals.contains(id)) {
            return RegOrImmediate(load_global_value(id));
        }

        throw std::runtime_error("Register for identifier does not exist");
    }

    throw std::runtime_error("Not implemented");
}

IR::RegOrImmediate IR::IRContext::lower_constant(const Constant *constantNode) {
    long value = constantNode->get_parsed_value();
    return RegOrImmediate(Immediate(value, constantNode->get_type_info().get_size()));
}

IR::RegOrImmediate IR::IRContext::lower_function_call(const FunctionCall *functionCall) {
    auto functionExpr = functionCall->get_lhs();
    Register destReg = get_next_temp_reg();
    CallInst::CallArgs args;
    if (functionCall->has_argument_list()) {
        auto *argumentList = functionCall->get_argument_list();
        for (int i = 0; i < argumentList->get_size(); ++i) {
            auto *expr = (*argumentList)[i];
            args.push_back(lower_expression(expr).to_argument());
        }
    }

    if (functionExpr->get_type_info().is_function()) {
        auto *functionId = cast<Identifier>(functionExpr);
        current_block->add_instruction(CallInst::create(destReg, FunctionPtrArgument::create(functionId->get_value()), std::move(args)));
    } else {
        auto functionPointer = lower_expression(functionExpr);
        current_block->add_instruction(CallInst::create(destReg, functionPointer.to_argument(), std::move(args)));
    }

    return RegOrImmediate(destReg);
}

IR::Register IR::IRContext::lower_lvalue_ptr(const Expression *expr) {
    assert(Sema::is_lvalue(expr));
    if (auto *idNode = dyn_cast<Identifier>(expr)) {
        if (id_reg_map.contains(idNode->get_symbol_entry())) {
            return id_reg_map.at(idNode->get_symbol_entry()).back();
        }
        throw std::runtime_error("Register for identifier does not exist");
    }
    if (auto *unaryOp = dyn_cast<UnaryOp>(expr)) {
        assert(unaryOp->get_operation() == UnaryOp::DEREFERENCE);
        Register output = get_next_temp_reg(Register::Type::Ptr, 4);
        Register dereferencedPtr = lower_lvalue_ptr(unaryOp->get_rhs());
        current_block->add_instruction(LoadInst::create(output, RegisterArgument::create(dereferencedPtr)));
        return output;
    }

    throw std::runtime_error("Not implemented");
}

IR::Global * IR::IRContext::add_global(const std::string& identifier, MemSize size) {
    globals[identifier] = std::make_unique<Global>(identifier,global_counter ++, size);
    return globals[identifier].get();
}

IR::Register IR::IRContext::load_global_value(const std::string &identifier) {
    const auto global = globals.at(identifier).get();
    const auto valueReg = get_next_temp_reg();
    current_block->add_instruction(LoadInst::create(valueReg, GlobalArgument::create(global->identifier)));
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
