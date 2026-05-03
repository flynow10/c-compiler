//
// Created by Natalie Wagner on 4/25/26.
//
#include "exceptions.hpp"
#include "sema.hpp"
#include "types.hpp"

QualType Sema::accept_expression(Expression *expression) {
    QualType type;
    if (auto *list = dyn_cast<ExpressionList>(expression)) {
        type = accept_expression_list(list);
    } else if (auto *assignment = dyn_cast<Assignment>(expression)) {
        type = accept_assignment(assignment);
    } else if (auto *binOp = dyn_cast<BinOp>(expression)) {
        type = accept_bin_op(binOp);
    } else if (auto *cast = dyn_cast<Cast>(expression)) {
        type = accept_cast(cast);
    } else if (auto *unaryOp = dyn_cast<UnaryOp>(expression)) {
        type = accept_unary_op(unaryOp);
    } else if (auto *sizeofType = dyn_cast<SizeofType>(expression)) {
        type = accept_sizeof(sizeofType);
    } else if (auto *indexExpression = dyn_cast<IndexExpression>(expression)) {
        type = accept_index_expression(indexExpression);
    } else if (auto *functionCall = dyn_cast<FunctionCall>(expression)) {
        type = accept_function_call(functionCall);
    } else if (auto *postAssignment = dyn_cast<PostAssignment>(expression)) {
        type = accept_post_assignment(postAssignment);
    } else if (auto *memberAccess = dyn_cast<MemberAccess>(expression)) {
        type = accept_member_access(memberAccess);
    } else if (auto *identifier = dyn_cast<Identifier>(expression)) {
        type = accept_identifier(identifier);
    } else if (auto *constant = dyn_cast<Constant>(expression)) {
        type = accept_constant(constant);
    } else if (auto *stringLiteral = dyn_cast<StringLiteral>(expression)) {
        type = accept_string_literal(stringLiteral);
    } else {
        throw SemaAnalysis::ExprException(expression, "Unknown expression type");
    }
    if (!expression->has_type_info()) {
        expression->set_type(type);
    }
    return type;
}

QualType Sema::accept_expression_list(ExpressionList *expressionList) {
    for (size_t i = 0; i < expressionList->get_size() - 1; i++) {
        auto *node = (*expressionList)[i];
        accept_expression(node);
    }
    return accept_expression((*expressionList)[expressionList->get_size() - 1]);
}

QualType Sema::accept_assignment(Assignment *assignment) {
    auto *lhs = assignment->get_lhs();
    auto *rhs = assignment->get_rhs();
    if (!is_lvalue(lhs)) {
        throw SemaAnalysis::ExprException(assignment, "Assignment not allowed to non lvalue expression");
    }
    auto lType = accept_expression(lhs);
    if (lType.is_const) {
        throw SemaAnalysis::ExprException(assignment, "Cannot assign to constant expression");
    }
    auto rType = accept_expression(rhs);
    if (!are_implicitly_convertable(lType.type, rType.type)) {
        throw SemaAnalysis::ExprException(assignment, "Assignment of incompatible types is forbidden");
    }
    return lType;
}

QualType Sema::accept_bin_op(BinOp *binOp) {
    auto *lhs = binOp->get_lhs();
    auto *rhs = binOp->get_rhs();
    auto lType = accept_expression(lhs);
    auto rType = accept_expression(rhs);
    if (!is_valid_bin_op(lType.type, rType.type, binOp->get_operation())) {
        throw SemaAnalysis::ExprException(binOp, "Cannot perform binary operation between incompatible types");
    }

    if (lType.is_integer() && rType.is_integer()) {
        return {usual_arithmetic_conversions(*this, lType.type, rType.type), false};
    }

    // TODO: Implement other operations
    throw std::runtime_error("Other operations have not been implemented!");
}

QualType Sema::accept_cast(Cast *cast) {
    auto type = cast->get_type_name();
    auto expr = cast->get_expression();
    auto exprType = accept_expression(expr);
    // TODO: Implement casting rules
    return exprType;
}

QualType Sema::accept_unary_op(UnaryOp *unaryOp) {
    auto *rhs = unaryOp->get_rhs();
    auto rType = accept_expression(rhs);
    auto operation = unaryOp->get_operation();

    if (operation == UnaryOp::ADDRESS_OF) {
        if (!is_lvalue(rhs)) {
            throw SemaAnalysis::ExprException(unaryOp, "Address of operation is not allowed on non lvalue types");
        }
        return {PointerType::get(*this, rType), true};
    }

    if (operation == UnaryOp::DEREFERENCE) {
        return dereference_pointer(rType.type, unaryOp);
    }

    if (operation == UnaryOp::SIZEOF) {
        return {IntegerType::get(*this, UnsignedInt), true};
    }

    if (operation == UnaryOp::INCREMENT || operation == UnaryOp::DECREMENT) {
        if (!is_lvalue(rhs)) {
            throw SemaAnalysis::ExprException(unaryOp, "Cannot increment or decrement non lvalue types");
        }
        if (rType.is_const) {
            throw SemaAnalysis::ExprException(unaryOp, "Cannot increment or decrement to constant expression");
        }
    }

    return rType;
}

QualType Sema::accept_sizeof(SizeofType *type) {
    return {IntegerType::get(*this, UnsignedInt), true};
}

QualType Sema::accept_index_expression(IndexExpression *indexExpression) {
    auto *lhs = indexExpression->get_lhs();
    auto *index = indexExpression->get_index();
    auto lType = accept_expression(lhs);
    auto indexType = accept_expression(index);
    if (!(lType.is_pointer() && indexType.is_integer()) && !(lType.is_integer() && indexType.is_pointer())) {
        throw SemaAnalysis::ExprException(indexExpression, "Cannot index expressions of non-pointer type");
    }
    auto &pointerType = lType.is_pointer() ? lType : indexType;
    auto &integerType = lType.is_integer() ? lType : indexType;
    return dereference_pointer(pointerType.type, indexExpression);
}

QualType Sema::accept_function_call(FunctionCall *functionCall) {
    auto *lhs = functionCall->get_lhs();
    auto lType = accept_expression(lhs);
    throw SemaAnalysis::ExprException(functionCall, "Function calls have not been implemented yet");
}

QualType Sema::accept_post_assignment(PostAssignment *postAssignment) {
    auto *lhs = postAssignment->get_lhs();
    if (!is_lvalue(lhs)) {
        throw SemaAnalysis::ExprException(postAssignment, "Post assignment not allowed to non lvalue expression");
    }
    return accept_expression(lhs);
}

QualType Sema::accept_member_access(MemberAccess *memberAccess) {
    QualType rhsType = accept_expression(memberAccess->get_lhs());

    if (memberAccess->get_access_type() == MemberAccess::POINTER) {
        rhsType = dereference_pointer(rhsType.type, memberAccess);
    }

    const auto &memberId = memberAccess->get_rhs();
    if (!isa<StructType>(rhsType.type)) {
        throw SemaAnalysis::ExprException(memberAccess, "Cannot access member of non-struct type");
    }
    auto *structType = cast<StructType>(rhsType.type);
    if (!structType->members.contains(memberId)) {
        throw SemaAnalysis::ExprException(memberAccess, "Member does not exist on struct");
    }
    auto &[memberType, memberConst] = structType->members.at(memberId);
    return {memberType, rhsType.is_const || memberConst};
}

QualType Sema::accept_identifier(Identifier *identifier) {
    auto &idValue = identifier->get_value();
    const auto *entry = local_table->findSymbol(idValue);
    return entry->type;
}

QualType Sema::accept_constant(Constant *constant) {
    auto &constantValue = constant->get_value();
    return {IntegerType::get(*this, Int), true};
}

QualType Sema::accept_string_literal(StringLiteral *stringLiteral) {
    auto &stringValue = stringLiteral->get_value();
    auto *charType = IntegerType::get(*this, Char);
    auto *arrayType = ArrayType::get(*this, {charType, true}, stringValue.size());
    return {arrayType, true};
}

// ----------------------------
// Integer Rank
// ----------------------------

typedef unsigned short Rank;
constexpr Rank INTEGER_RANK = 3;

Rank get_integer_rank(const PrimitiveType type) {
    switch (type) {
        case Char:
        case SignedChar:
        case UnsignedChar:
            return 1;
        case Short:
        case UnsignedShort:
            return 2;
        case Int:
        case UnsignedInt:
            return 3;
        case Long:
        case UnsignedLong:
            return 4;
        default:
            throw std::runtime_error("Cannot find the rank of non integer types");
    }
}

// ----------------------------
// Helper Functions
// ----------------------------

bool Sema::is_lvalue(const Expression *expression) {
    if (isa<Identifier, StringLiteral, IndexExpression>(expression)) {
        return true;
    }

    if (const auto *memberAccess = dyn_cast<MemberAccess>(expression)) {
        if (memberAccess->get_access_type() == MemberAccess::POINTER || is_lvalue(memberAccess->get_lhs())) {
            return true;
        }
    }

    if (const auto *unaryOp = dyn_cast<UnaryOp>(expression)) {
        if (unaryOp->get_operation() == UnaryOp::DEREFERENCE) {
            return is_lvalue(unaryOp->get_rhs());
        }
    }

    return false;
}

bool Sema::is_rvalue(const Expression *expression) {
    if (isa<
        Constant,
        Cast,
        BinOp,
        Assignment,
        PostAssignment,
        ExpressionList,
        FunctionCall,
        SizeofType
    >(expression)) {
        return true;
    }

    if (const auto *memberAccess = dyn_cast<MemberAccess>(expression)) {
        if (memberAccess->get_access_type() == MemberAccess::MEMBER && is_rvalue(memberAccess->get_lhs())) {
            return true;
        }
    }

    if (const auto *unaryOp = dyn_cast<UnaryOp>(expression)) {
        if (unaryOp->get_operation() == UnaryOp::DEREFERENCE) {
            return is_rvalue(unaryOp->get_rhs());
        }
        return true;
    }

    return false;
}

bool Sema::are_implicitly_convertable(const Type *left, const Type *right) {
    // TODO: Make better compatibility test
    if (left->type == right->type) {
        return true;
    }

    if (left->is_integer() && right->is_integer()) {
        return true;
    }

    return false;
}

bool Sema::is_scalar_type(const Type *type) {
    return type->is_integer() || type->is_pointer();
}

bool Sema::is_valid_bin_op(const Type *lType, const Type *rType, BinOp::Op operation) {
    // TODO: Still need to implement pointers
    switch (operation) {
        case BinOp::ADD:
        case BinOp::SUB: {
            if (lType->is_integer() && rType->is_integer()) {
                return true;
            }
            return false;
        }
        case BinOp::MUL:
        case BinOp::DIV:
        case BinOp::MOD: {
            if (lType->is_integer() && rType->is_integer()) {
                return true;
            }
            return false;
        }
        case BinOp::EQUAL:
        case BinOp::NOT_EQUAL: {
            if (lType->is_integer() && rType->is_integer()) {
                return true;
            }
            return false;
        }
        case BinOp::AND:
        case BinOp::INCLUSIVE_OR:
        case BinOp::EXCLUSIVE_OR:
        case BinOp::LEFT_SHIFT:
        case BinOp::RIGHT_SHIFT:
        case BinOp::GREATER_EQUAL:
        case BinOp::LESS_EQUAL:
        case BinOp::GREATER_THAN:
        case BinOp::LESS_THAN: {
            return lType->is_integer() && rType->is_integer();
        }
        case BinOp::LOGIC_AND:
        case BinOp::LOGIC_OR: {
            return is_scalar_type(lType) && is_scalar_type(rType);
        }
    }
    return false;
}


Type *Sema::integer_promotion(Sema &ctx, Type *type) {
    if (Rank rank = get_integer_rank(type->type); rank < INTEGER_RANK) {
        return IntegerType::get(ctx, Int);
    }
    return type;
}

Type *Sema::usual_arithmetic_conversions(Sema &ctx, Type *lType, Type *rType) {
    lType = integer_promotion(ctx, lType);
    rType = integer_promotion(ctx, rType);

    if (lType == rType) {
        return lType;
    }

    if (lType->is_signed() == rType->is_signed()) {
        if (get_integer_rank(lType->type) < get_integer_rank(rType->type)) {
            return lType;
        }
        return rType;
    }

    Type *unsignedType = lType->is_signed() ? rType : lType;
    Type *signedType = lType->is_signed() ? lType : rType;

    if (get_integer_rank(unsignedType->type) >= get_integer_rank(signedType->type)) {
        return unsignedType;
    }
    return signedType;
}

QualType Sema::dereference_pointer(Type *type, const Expression *parentExpression) {
    if (!isa<PointerType>(type)) {
        throw SemaAnalysis::ExprException(parentExpression, "Cannot dereference type which is not a PointerType");
    }
    return cast<PointerType>(type)->pointed_type;
}
