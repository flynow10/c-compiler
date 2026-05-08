//
// Created by Natalie Wagner on 4/18/26.
//

#ifndef SEMA_HPP
#define SEMA_HPP
#include <iostream>
#include <list>
#include <map>

#include "../parsing/ast.hpp"
#include "./types.hpp"

using namespace AST;

// ----------------------------
// Symbol Table
// ----------------------------

struct Entry {
    std::string identifier;
    bool is_complete = true;
    QualType type;

    static Entry create(const std::string &identifier, QualType type);
};

class SymbolTable {
public:

    enum TableType {
        File,
        Function,
        Loop,
        Block
    };

    SymbolTable(SymbolTable &) = delete;
    SymbolTable & operator=(const SymbolTable &) = delete;

    std::map<std::string, Entry> structs;
    std::map<std::string, Entry> symbols;
    SymbolTable *parent_scope = nullptr;
    TableType table_type = File;

    explicit SymbolTable(SymbolTable * parent, TableType type) : parent_scope(parent), table_type(type) {
    }

    Entry *addStruct(const std::string &identifier);

    Entry *addSymbol(const Entry &entry);
    Entry *addSymbol(const std::string &identifier, QualType type);

    Entry *tryFindStruct(const std::string &identifier);

    Entry *tryFindSymbol(const std::string &identifier);

    Entry *findStruct(const std::string &identifier);

    Entry *findSymbol(const std::string &identifier);

    [[nodiscard]] bool isLocallyDefinedSymbol(const std::string &identifier) const;

    [[nodiscard]] bool isLocallyDefinedStruct(const std::string &identifier) const;

    [[nodiscard]] bool isInLoop() const;
    [[nodiscard]] bool isInFunction() const;
};

// ----------------------------
// Semantic Analysis
// ----------------------------

class Sema {
public:
    using HashValue = std::size_t;
    Type void_type = Type(PrimitiveType::Void);
    std::map<HashValue, IntegerType> integer_types;
    std::map<HashValue, PointerType> pointer_types;
    std::map<HashValue, StructType> struct_types;
    std::map<HashValue, StructRefType> struct_ref_types;
    std::map<HashValue, ArrayType> array_types;
    std::map<HashValue, FunctionType> function_types;
    SymbolTable *global_table = nullptr;
    SymbolTable *local_table = nullptr;
    Entry *function_declaration_ptr = nullptr;

    // ---------
    // Acceptors
    // ---------
    void accept_ast(Node *ast);
private:
    void accept_decl(Decl *decl);

    /// NOTE: If couldBeForwardDecl is true, this function still checks semantics, but returns
    /// a void type instead
    QualType accept_decl_specifiers(DeclSpecifiers *specifiers, bool couldBeForwardDecl);

    Type *accept_struct_specifier(StructSpecifier *specifier, bool couldBeForwardDecl);

    StructType *accept_struct_decl(StructDeclList *declList);

    Entry *accept_init_declarator(InitDeclarator *initDeclarator, QualType partialType);
    void accept_initializer_list(InitializerList *initializerList, Type *structOrRef);
    Entry accept_declarator(Declarator *declarator, QualType type, bool couldBeAbstract);
    QualType accept_index_declarator(IndexDeclarator *indexDeclarator, QualType type, bool inFunctionDef);
    QualType accept_parameterized_declarator(ParameterizedDeclarator *parameterizedDeclarator, QualType returnType);

    void accept_function_decl(FunctionDecl *functionDecl);
    void accept_compound_statement(CompoundStatement *compoundStatement);
    void accept_compound_statement(CompoundStatement *compoundStatement, const std::shared_ptr<SymbolTable> &symbolTable);
    void accept_statement(Statement *statement);
    void accept_selection_statement(SelectionStatement *selectionStatement);
    void accept_while_statement(WhileStatement *whileStatement);
    void accept_do_statement(DoStatement *doStatement);
    void accept_for_statement(ForStatement *forStatement);
    void accept_control_statement(ControlStatement *controlStatement);

    QualType accept_expression(Expression *expression);
    QualType accept_expression_list(ExpressionList *expressionList);
    QualType accept_assignment(Assignment *assignment);
    QualType accept_bin_op(BinOp *binOp);
    QualType accept_cast(Cast *cast);
    QualType accept_unary_op(UnaryOp *unaryOp);
    QualType accept_sizeof(SizeofType *type);
    QualType accept_index_expression(IndexExpression *indexExpression);
    QualType accept_function_call(FunctionCall *functionCall);
    QualType accept_post_assignment(PostAssignment *postAssignment);
    QualType accept_member_access(MemberAccess *memberAccess);
    QualType accept_identifier(Identifier *identifier);
    QualType accept_constant(Constant *constant);
    QualType accept_string_literal(StringLiteral *stringLiteral);

    // -------
    // Helpers
    // -------
public:
    static bool is_lvalue(const Expression *expression);
    static bool is_rvalue(const Expression *expression);

    static bool is_constant_expression(const Expression *expression);

    // Type Comparisons
    static bool are_implicitly_convertable(const Type *left, const Type *right);
    static bool is_scalar_type(const Type *type);

    static bool is_valid_bin_op(const Type *lType, const Type *rType, BinOp::Op operation);
    static Type *integer_promotion(Sema& ctx, Type *type);
    static Type *usual_arithmetic_conversions(Sema& ctx, Type *lType, Type *rType);
    static QualType dereference_pointer(Type *type, const Expression *parentExpression);
    static bool is_pointer_to_function(const Type *type);

    QualType convert_array_to_pointer(QualType type, const Expression *parentExpression);


    // Debug
    void print_debug_info();
private:
    template <typename T>
    void print_table(const std::map<HashValue, T> &table);
};

template <typename T>
inline void Sema::print_table(const std::map<HashValue, T> &table) {
    std::cout << "Table ------" << std::endl;
    for (auto value : table) {
        std::cout << value.first << ": " << value.second.to_string() << std::endl;
    }
}


#endif //SEMA_HPP
