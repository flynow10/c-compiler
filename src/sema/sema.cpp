//
// Created by Natalie Wagner on 4/18/26.
//

#include "sema.hpp"

#include <sstream>

#include "exceptions.hpp"
#include "../casting.hpp"
#include "../parsing/ast.hpp"

using namespace AST;

void Sema::accept_ast(Node *ast) {
    try {
        if (const auto tu = dyn_cast<TranslationUnit>(ast)) {
            tu->set_symbol_table(std::make_shared<SymbolTable>(nullptr, SymbolTable::File));
            global_table = tu->get_symbol_table_raw();
            local_table = global_table;
            initialize_builtin_functions();
            int i = 0;
            for (const auto &node: *tu) {
                try {
                    if (isa<Decl>(node)) {
                        accept_decl(cast<Decl>(node.get()));
                    } else if (isa<FunctionDecl>(node)) {
                        accept_function_decl(cast<FunctionDecl>(node.get()));
                    } else {
                        throw std::runtime_error(
                            "Translation units must only contain declarations and function declarations.");
                    }
                } catch (...) {
                    std::stringstream s;
                    s << "Declaration #" << i << " of translation unit:";
                    std::throw_with_nested(std::runtime_error(s.str()));
                }
                i++;
            }
        } else {
            throw std::runtime_error("AST must begin with a translation unit.");
        }
    } catch (const std::exception &e) {
        SemaAnalysis::print_exception(e);
        throw std::runtime_error("exception occurred in semantic analysis");
    }
}

void Sema::initialize_builtin_functions() {
    FunctionType::ArgList argList;
    argList.emplace_back(FunctionArg({IntegerType::get(*this, PrimitiveType::Char)}));
    auto putsFunction = FunctionType::get(*this, {&void_type, true}, argList);
    QualType putsType = {putsFunction, true};
    global_table->addSymbol("puts", putsType);
}

const std::unordered_map<TypeSpecifier::TypeSpecifierType, PrimitiveType> TypeSpecMap = {
    {TypeSpecifier::LONG, PrimitiveType::Long},
    {TypeSpecifier::INT, PrimitiveType::Int},
    {TypeSpecifier::SHORT, PrimitiveType::Short},
    {TypeSpecifier::CHAR, PrimitiveType::SignedChar},
    {TypeSpecifier::STRUCT, PrimitiveType::Struct},
    {TypeSpecifier::VOID, PrimitiveType::Void},
};

void Sema::accept_decl(Decl *decl) {
    auto *declaratorList = cast<InitDeclaratorList>(decl->get_declarators());
    bool couldBeForwardDecl = declaratorList->get_size() == 0;
    auto declSpecs = cast<DeclSpecifiers>(decl->get_decl_specs());
    auto type = accept_decl_specifiers(cast<DeclSpecifiers>(declSpecs), couldBeForwardDecl);

    for (int i = 0; i < declaratorList->get_size(); ++i) {
        auto *initDeclarator = (*declaratorList)[i];
        assert(isa<InitDeclarator>(initDeclarator));
        accept_init_declarator(cast<InitDeclarator>(initDeclarator), type);
    }
}

PrimitiveType convertFromTypeSpec(const TypeSpecifier::TypeSpecifierType type, const bool isSigned,
                                  const bool signMarked) {
    switch (type) {
        case TypeSpecifier::VOID:
            return PrimitiveType::Void;
        case TypeSpecifier::CHAR:
            if (isSigned)
                return signMarked ? PrimitiveType::SignedChar : PrimitiveType::Char;
            return PrimitiveType::UnsignedChar;
        case TypeSpecifier::SHORT:
            if (isSigned)
                return PrimitiveType::Short;
            return PrimitiveType::UnsignedShort;
        case TypeSpecifier::INT:
            if (isSigned)
                return PrimitiveType::Int;
            return PrimitiveType::UnsignedInt;
        case TypeSpecifier::LONG:
            if (isSigned)
                return PrimitiveType::Long;
            return PrimitiveType::UnsignedLong;
        case TypeSpecifier::STRUCT:
            return PrimitiveType::Struct;
        default:
            throw std::runtime_error("Unexpected type specifier type");
    }
}

void throwDeclareMultipleTypes() noexcept(false) {
    throw std::runtime_error("Cannot currently specify multiple types in one declaration");
}

QualType Sema::accept_decl_specifiers(DeclSpecifiers *specifiers, bool couldBeForwardDecl = false) {
    Type *type = nullptr;
    std::optional<TypeSpecifier::TypeSpecifierType> sType;
    bool signMarked = false;
    bool isSigned = true;
    bool isConst = false;

    for (int i = 0; i < specifiers->get_size(); ++i) {
        auto *node = specifiers->operator[](i);
        if (auto *structSpec = dyn_cast<StructSpecifier>(node)) {
            if (sType.has_value()) {
                throwDeclareMultipleTypes();
            }
            type = accept_struct_specifier(structSpec, couldBeForwardDecl);
            sType = TypeSpecifier::STRUCT;
            continue;
        }
        if (const auto *typeSpec = dyn_cast<TypeSpecifier>(node)) {
            const auto typeSpecType = typeSpec->get_type();
            if (typeSpecType == TypeSpecifier::UNSIGNED || typeSpecType == TypeSpecifier::SIGNED) {
                if (signMarked) {
                    throw std::runtime_error("Declarations cannot contain multiple signedness specifiers");
                }
                isSigned = typeSpecType == TypeSpecifier::SIGNED;
                signMarked = true;
                continue;
            }

            if (sType.has_value()) {
                throwDeclareMultipleTypes();
            }
            sType = typeSpecType;
            continue;
        }
        if (isa<TypeQualifier>(node)) {
            if (isConst) {
                throw std::runtime_error("Declarations cannot contain multiple const qualifiers");
            }
            isConst = true;
        }
    }

    if (!sType.has_value()) {
        throw std::runtime_error("At least one type specifier must be included in a declaration");
    }

    if (signMarked && (sType == TypeSpecifier::STRUCT || sType == TypeSpecifier::VOID)) {
        throw std::runtime_error("Only integer types can be marked as signed or unsigned");
    }

    if (couldBeForwardDecl) {
        return {&void_type, true};
    }

    if (type != nullptr) {
        return {type, isConst};
    }
    return {Type::get(*this, convertFromTypeSpec(sType.value(), isSigned, signMarked)), isConst};
}

Type *Sema::accept_struct_specifier(StructSpecifier *specifier, bool couldBeForwardDecl) {
    if (specifier->has_declaration()) {
        // Create an incomplete struct which can be used in the declaration
        if (specifier->has_struct_name()) {
            const auto &identifier = specifier->get_identifier();
            Entry *entry = local_table->tryFindStruct(identifier);
            if (!entry) {
                local_table->addStruct(identifier);
            }
        }

        auto *structDecl = cast<StructDeclList>(specifier->get_declaration());
        auto *structType = accept_struct_decl(cast<StructDeclList>(structDecl));

        // Add the full struct to the symbol table if needed
        if (specifier->has_struct_name()) {
            const auto &identifier = specifier->get_identifier();
            Entry *entry = local_table->findStruct(identifier);
            if (entry->is_complete) {
                throw std::runtime_error("Cannot redefine struct with identifier \"" + identifier + "\"");
            }

            entry->type.type = structType;
            entry->is_complete = true;
        }
        return structType;
    }

    const auto &identifier = specifier->get_identifier();
    if (couldBeForwardDecl) {
        local_table->addStruct(identifier);
        return nullptr;
    }
    auto *entry = local_table->findStruct(identifier);
    if (!entry) {
        throw std::runtime_error("Could not find struct with identifier \"" + identifier + "\"");
    }
    if (!entry->is_complete) {
        return StructRefType::get(*this, entry->identifier);
    }
    assert(isa<StructType>(entry->type.type));
    return cast<StructType>(entry->type.type);
}

StructType *Sema::accept_struct_decl(StructDeclList *declList) {
    StructType::MemberMap members;
    for (const auto &node: *declList) {
        const auto *structDecl = cast<StructDecl>(node.get());

        auto entryParams = accept_decl_specifiers(cast<DeclSpecifiers>(structDecl->get_spec_qual()));
        auto *declarator = cast<Declarator>(structDecl->get_declarator());
        Entry member = accept_declarator(declarator, entryParams, false);

        if (members.contains(member.identifier)) {
            throw std::runtime_error("Redefinition of struct member \"" + member.identifier + "\"");
        }

        members[member.identifier] = member.type;
    }

    return StructType::get(*this, members);
}

Entry *Sema::accept_init_declarator(InitDeclarator *initDeclarator, const QualType partialType) {
    auto *declarator = initDeclarator->get_declarator();
    Entry entry = accept_declarator(declarator, partialType, false);
    if (initDeclarator->has_initializer()) {
        if (isa<Expression>(initDeclarator->get_initializer())) {
            auto *initExpr = cast<Expression>(initDeclarator->get_initializer());
            auto assignmentType = accept_expression(initExpr);
            if (!are_implicitly_convertable(entry.type.type, assignmentType.type)) {
                throw std::runtime_error("Assignment of incompatible types is forbidden");
            }
        } else {
            auto *initializerList = cast<InitializerList>(initDeclarator->get_initializer());
            accept_initializer_list(initializerList, entry.type.type);
        }
    } else {
        if (entry.type.is_function()) {
            entry.is_complete = false;
        }
    }
    auto *entryPtr = local_table->addSymbol(entry);
    initDeclarator->set_symbol_entry(entryPtr);
    return entryPtr;
}

void Sema::accept_initializer_list(InitializerList *initializerList, Type *structOrRef) {
    if (!structOrRef->is_struct()) {
        throw std::runtime_error("Cannot use initializer list for non struct type");
    }
    StructType *type = StructType::convert(*this, structOrRef);
    auto it = type->members.begin();
    for (auto & node: *initializerList) {
        if (isa<Expression>(node.get())) {
            auto exprType = accept_expression(cast<Expression>(node.get()));
            if (!are_implicitly_convertable(it->second.type, exprType.type)) {
                throw std::runtime_error("Invalid type in initializer list");
            }
        } else {
            assert(isa<InitializerList>(node.get()));
            auto *nestedList = cast<InitializerList>(node.get());
            Type *nestedType = it->second.type;
            accept_initializer_list(nestedList, nestedType);
        }
        it = std::next(it);
    }
}

Entry Sema::accept_declarator(Declarator *declarator, QualType type, bool couldBeAbstract) {
    Entry entry = {.type = type};

    if (declarator->has_pointer()) {
        auto *pointer = cast<AST::Pointer>(declarator->get_pointer());
        do {
            entry.type = {PointerType::get(*this, entry.type), pointer->is_const()};
            pointer = pointer->get_next_pointer();
        } while (pointer != nullptr);
    }

    if (declarator->has_suffix()) {
        auto *suffix = declarator->get_suffix();
        do {
            if (auto *indexDeclarator = dyn_cast<IndexDeclarator>(suffix)) {
                entry.type = accept_index_declarator(indexDeclarator, entry.type, couldBeAbstract);
                suffix = indexDeclarator->get_next_suffix();
            } else if (auto *parameterizedDeclarator = dyn_cast<ParameterizedDeclarator>(suffix)) {
                entry.type = accept_parameterized_declarator(parameterizedDeclarator, entry.type);
                suffix = parameterizedDeclarator->get_next_suffix();
            } else {
                throw std::runtime_error("Unknown suffix type");
            }
        } while (suffix != nullptr);
    }

    if (declarator->is_abstract()) {
        if (!couldBeAbstract) {
            throw std::runtime_error("Non abstract declarator must have an identifier");
        }
    } else {
        auto *directDeclarator = declarator->get_direct_declarator();
        if (isa<DirectDeclarator>(directDeclarator)) {
            entry.identifier = cast<DirectDeclarator>(directDeclarator)->get_identifier();
        } else {
            assert(isa<Declarator>(directDeclarator));
            entry = accept_declarator(cast<Declarator>(directDeclarator), entry.type, couldBeAbstract);
        }
    }

    if (isa<StructRefType>(entry.type.type)) {
        throw std::runtime_error("Declarator base type must be complete");
    }

    return entry;
}

QualType Sema::accept_index_declarator(IndexDeclarator *indexDeclarator, QualType type, bool inFunctionDef) {
    if (!indexDeclarator->has_expression()) {
        if (!inFunctionDef) {
            throw std::runtime_error("Array declaration must provide a size");
        }
        return {ArrayType::get(*this, type, ArrayType::UNKNOWN_SIZE), type.is_const};
    }
    auto *constExpr = indexDeclarator->get_expression();
    QualType constExprType = accept_expression(constExpr);
    if (!is_constant_expression(constExpr)) {
        throw std::runtime_error("Cannot declare array of variable size");
    }
    // TODO: Handle constant size check
    return {ArrayType::get(*this, type, 0), type.is_const};
}

QualType Sema::accept_parameterized_declarator(ParameterizedDeclarator *parameterizedDeclarator, QualType returnType) {
    if (!parameterizedDeclarator->has_parameters()) {
        return {FunctionType::get(*this, returnType, FunctionType::ArgList()), returnType.is_const};
    }
    auto *parameterList = parameterizedDeclarator->get_parameter_list();
    FunctionType::ArgList parameterTypes;
    for (auto &node: *parameterList) {
        auto *parameter = cast<Parameter>(node.get());
        QualType parameterType = accept_decl_specifiers(parameter->get_decl_specs());
        if (parameter->has_declarator()) {
            Entry entry = accept_declarator(parameter->get_declarator(), parameterType, true);
            parameterTypes.emplace_back(entry.type, entry.identifier);
        } else {
            parameterTypes.emplace_back(parameterType);
        }
    }
    return {FunctionType::get(*this, returnType, parameterTypes), returnType.is_const};
}

void Sema::accept_function_decl(FunctionDecl *functionDecl) {
    auto *declSpecs = functionDecl->get_spec_quals();
    auto *declarator = functionDecl->get_declarator();

    const auto entryPrototype = accept_decl_specifiers(declSpecs);
    const auto entry = accept_declarator(declarator, entryPrototype, false);

    Entry * functionEntry = global_table->tryFindSymbol(entry.identifier);
    if (functionEntry != nullptr) {
        if (functionEntry->is_complete) {
            throw std::runtime_error("Redefinition of function \"" + functionEntry->identifier + "\"");
        }
        // TODO: This still isn't working because named arguments are being included in the function hash
        // TODO: We can't easily remove then because that breaks the function named argument parsing below
        if (functionEntry->type != entry.type) {
            throw std::runtime_error("Function declaration must match forward declaration");
        }
        functionEntry->is_complete = true;
    } else {
        functionEntry = global_table->addSymbol(entry);
    }

    function_declaration_ptr = functionEntry;
    functionDecl->set_function_entry(function_declaration_ptr);

    // Add intermediate symbol table to contain function arguments
    // This is functionally the same as adding the arguments to the compound statement table
    functionDecl->set_symbol_table(std::make_shared<SymbolTable>(local_table, SymbolTable::Function));
    local_table = functionDecl->get_symbol_table_raw();

    if (!isa<FunctionType>(entry.type.type)) {
        throw std::runtime_error("Function declaration must declare a function type");
    }

    if (functionDecl->_has_parameter_list()) {
        const auto *parameters = functionDecl->_get_parameter_list();
        const auto *functionType = cast<FunctionType>(function_declaration_ptr->type.type);

        for (int i = 0; i < parameters->get_size(); ++i) {
            const auto &parameterType = functionType->argument_types.at(i);
            auto parameter = (*parameters)[i];

            if (parameter->get_declarator()->is_abstract()) {
                throw std::runtime_error("Function declaration must declare a named argument");
            }

            auto &argumentName =  parameter->get_declarator()->get_identifier();
            local_table->addSymbol(argumentName, parameterType.type);
        }
    }

    auto body = functionDecl->get_body();
    accept_compound_statement(body, functionDecl->get_symbol_table());
    function_declaration_ptr = nullptr;
}

void Sema::accept_compound_statement(CompoundStatement *compoundStatement) {
    accept_compound_statement(compoundStatement, std::make_shared<SymbolTable>(local_table, SymbolTable::Block));
}

void Sema::accept_compound_statement(CompoundStatement *compoundStatement, const std::shared_ptr<SymbolTable> &symbolTable) {
    compoundStatement->set_symbol_table(symbolTable);
    local_table = compoundStatement->get_symbol_table_raw();

    int i = 0;
    for (auto &node: *compoundStatement) {
        assert(isa<Statement>(node.get()));
        const auto statement = cast<Statement>(node.get());
        try {
            accept_statement(statement);
        } catch (...) {
            std::stringstream s;
            s << "Statement #" << i << " of compound statement:";
            std::throw_with_nested(std::runtime_error(s.str()));
        }
        i++;
    }

    local_table = local_table->parent_scope;
}

void Sema::accept_statement(Statement *statement) {
    if (auto *compoundStmt = dyn_cast<CompoundStatement>(statement)) {
        accept_compound_statement(compoundStmt);
    } else if (auto *exprStmt = dyn_cast<ExpressionStatement>(statement)) {
        accept_expression(exprStmt->get_expression());
    } else if (auto *declStmt = dyn_cast<Decl>(statement)) {
        accept_decl(declStmt);
    } else if (auto *selectionStmt = dyn_cast<SelectionStatement>(statement)) {
        accept_selection_statement(selectionStmt);
    } else if (auto *whileStmt = dyn_cast<WhileStatement>(statement)) {
        accept_while_statement(whileStmt);
    } else if (auto *doStmt = dyn_cast<DoStatement>(statement)) {
        accept_do_statement(doStmt);
    } else if (auto *forStmt = dyn_cast<ForStatement>(statement)) {
        accept_for_statement(forStmt);
    } else if (auto *controlStmt = dyn_cast<ControlStatement>(statement)) {
        accept_control_statement(controlStmt);
    } else {
        throw std::runtime_error("Unexpected statement type");
    }
}

void Sema::accept_selection_statement(SelectionStatement *selectionStatement) {
    auto *condition = selectionStatement->get_condition();
    auto conditionType = accept_expression(condition);
    if (!is_scalar_type(conditionType.type)) {
        throw std::runtime_error("If statement conditional must be a scalar type");
    }
    accept_compound_statement(selectionStatement->get_then());
    if (selectionStatement->has_else()) {
        accept_compound_statement(selectionStatement->get_else());
    }
}

void Sema::accept_while_statement(WhileStatement *whileStatement) {
    auto *condition = whileStatement->get_condition();
    auto conditionType = accept_expression(condition);
    if (!is_scalar_type(conditionType.type)) {
        throw std::runtime_error("While statement condition must be a scalar type");
    }
    std::shared_ptr<SymbolTable> table = std::make_shared<SymbolTable>(local_table, SymbolTable::Loop);
    accept_compound_statement(whileStatement->get_body(), table);
}

void Sema::accept_do_statement(DoStatement *doStatement) {
    std::shared_ptr<SymbolTable> table = std::make_shared<SymbolTable>(local_table, SymbolTable::Loop);
    accept_compound_statement(doStatement->get_body(), table);
    auto *condition = doStatement->get_condition();
    auto conditionType = accept_expression(condition);
    if (!is_scalar_type(conditionType.type)) {
        throw std::runtime_error("While statement condition must be a scalar type");
    }
}

void Sema::accept_for_statement(ForStatement *forStatement) {
    std::shared_ptr<SymbolTable> forLoopScope = std::make_shared<SymbolTable>(local_table, SymbolTable::Loop);
    local_table = forLoopScope.get();
    if (forStatement->has_init()) {
        auto *initializer = forStatement->get_initialization();
        if (auto *expr = dyn_cast<Expression>(initializer)) {
            accept_expression(expr);
        } else if (auto *decl = dyn_cast<Decl>(initializer)) {
            accept_decl(decl);
        } else {
            throw std::runtime_error("Unexpected initializer type");
        }
    }

    if (forStatement->has_condition()) {
        auto *condition = forStatement->get_condition();
        auto conditionType = accept_expression(condition);
        if (!is_scalar_type(conditionType.type)) {
            throw std::runtime_error("For statement condition must be a scalar type");
        }
    }

    if (forStatement->has_increment()) {
        auto *increment = forStatement->get_increment();
        accept_expression(increment);
    }

    accept_compound_statement(forStatement->get_body(), forLoopScope);
}

void Sema::accept_control_statement(ControlStatement *controlStatement) {
    if (controlStatement->is_break() || controlStatement->is_continue()) {
        if (!local_table->isInLoop()) {
            throw std::runtime_error("Cannot break or continue when not in a loop body");
        }
    } else {
        if (!local_table->isInFunction()) {
            throw std::runtime_error("Cannot return outside of function body");
        }
        QualType returnedType = {&void_type, true};
        if (controlStatement->has_return_expr()) {
            returnedType = accept_expression(controlStatement->get_return_expression());
        }
        QualType returnType = cast<FunctionType>(function_declaration_ptr->type.type)->return_type;
        if (!are_implicitly_convertable(returnType.type, returnedType.type)) {
            throw std::runtime_error("Return expression does not match the functions declared return type");
        }
    }
}

void Sema::print_debug_info() {
    print_table(integer_types);
    print_table(pointer_types);
    print_table(struct_types);
    print_table(struct_ref_types);
    print_table(array_types);
    print_table(function_types);
}
