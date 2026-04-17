#ifndef AST_H
#define AST_H

#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "ast.hpp"

using std::shared_ptr;

namespace AST {
    class Node;
    class Statement;
    class Decl;
    class Expression;
    class CompoundStatement;

    using const_iterator = shared_ptr<Node> *;

    std::string to_string(const std::shared_ptr<Node> &node, int indent);

    inline std::string to_string(const std::shared_ptr<Node> &node) { return AST::to_string(node, 0); }

    class Node {
    public:
        virtual ~Node() = default;

        virtual shared_ptr<Node> *begin() {
            return nullptr;
        }

        [[nodiscard]] virtual const shared_ptr<Node> *begin() const {
            return nullptr;
        }

        virtual shared_ptr<Node> *end() {
            return nullptr;
        }

        [[nodiscard]] virtual const shared_ptr<Node> *end() const {
            return nullptr;
        }

        friend std::ostream &operator<<(std::ostream &os, const std::shared_ptr<Node> &obj) {
            return os << AST::to_string(obj);
        }

        [[nodiscard]] std::string get_name() const {
            const std::string nodeName = _get_name();
            if (nodeName.empty()) {
                return "UnnamedNode";
            }
            return "AST::" + nodeName;
        }

    protected:
        [[nodiscard]] virtual std::string _get_name() const {
            return "";
        }
    };

    class TranslationUnit : public Node {
        shared_ptr<Node> *nodes = nullptr;
        size_t size = 0;

    public:
        ~TranslationUnit() override {
            delete[] nodes;
        }

        [[nodiscard]] size_t get_size() const {
            return size;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return nodes + size;
        }

        static shared_ptr<TranslationUnit> create(const std::vector<shared_ptr<Node> > &nodes) {
            auto base = std::make_shared<TranslationUnit>();
            base->nodes = new shared_ptr<Node>[nodes.size()];
            base->size = nodes.size();
            std::ranges::copy(nodes, base->nodes);
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "TranslationUnit";
        }
    };

    class Statement : public Node {
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "Statement";
        }
    };

    class Decl : public Node {
        shared_ptr<Node> *nodes = nullptr;
        size_t num_spec_quals = 0;
        size_t num_declarators = 0;

    public:
        ~Decl() override {
            delete[] nodes;
        }

        [[nodiscard]] size_t get_num_spec_quals() const {
            return num_spec_quals;
        }

        [[nodiscard]] shared_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        [[nodiscard]] shared_ptr<Node> *end() override {
            return nodes + num_spec_quals + num_declarators;
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return nodes + num_spec_quals + num_declarators;
        }

        static shared_ptr<Decl> create(const std::vector<shared_ptr<Node> > &spec_quals,
                                       const std::vector<shared_ptr<Node> > &declarators) {
            auto base = std::make_shared<Decl>();
            base->nodes = new shared_ptr<Node>[spec_quals.size()];
            base->num_spec_quals = spec_quals.size();
            base->num_declarators = declarators.size();
            std::ranges::copy(spec_quals, base->nodes);
            std::ranges::copy(declarators, base->nodes + base->num_spec_quals);
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "Decl";
        }
    };

    class DeclSpecifiers : public Node {
        std::shared_ptr<Node> *nodes = nullptr;
        size_t num_spec_quals = 0;
    public:
        ~DeclSpecifiers() override {
            delete[] nodes;
        }

        shared_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        [[nodiscard]] shared_ptr<Node> *end() override {
            return nodes + num_spec_quals;
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return nodes + num_spec_quals;
        }

        static shared_ptr<DeclSpecifiers> create(const std::vector<shared_ptr<Node> > &nodes) {
            auto base = std::make_shared<DeclSpecifiers>();
            base->nodes = new shared_ptr<Node>[nodes.size()];
            base->num_spec_quals = nodes.size();
            std::ranges::copy(nodes, base->nodes);
            return base;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "DeclSpecifiers";
        }
    };

    class StorageClass : public Node {
    public:
        enum StorageClassType { TYPEDEF, STATIC, AUTO };

        [[nodiscard]] StorageClassType get_type() const {
            return type;
        }

        static shared_ptr<StorageClass> create(StorageClassType type) {
            auto base = std::make_shared<StorageClass>();
            base->type = type;
            return base;
        }

    private:
        StorageClassType type = AUTO;

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "StorageClass";
        }
    };

    class TypeSpecifier : public Node {
    public:
        enum TypeSpecifierType { VOID, CHAR, SHORT, INT, LONG, UNSIGNED, SIGNED, STRUCT, ENUM };

        [[nodiscard]] TypeSpecifierType get_type() const {
            return type;
        }

        static shared_ptr<TypeSpecifier> create(const TypeSpecifierType type) {
            if (type == STRUCT || type == ENUM) {
                throw std::runtime_error("Cannot create specific type specifier with general factor method");
            }
            auto base = std::make_shared<TypeSpecifier>();
            base->type = type;
            return base;
        }

    protected:
        TypeSpecifierType type = VOID;

        [[nodiscard]] std::string _get_name() const override {
            return "TypeSpecifier";
        }
    };

    class StructSpecifier : public TypeSpecifier {
        std::string structName;
        shared_ptr<Node> structDeclaration = nullptr;
        bool hasDeclaration = false;
        bool hasStructName = false;

    public:
        static shared_ptr<StructSpecifier> create(const std::string &structName) {
            auto base = std::make_shared<StructSpecifier>();
            base->structName = structName;
            base->hasStructName = true;
            base->hasDeclaration = false;
            return base;
        }

        static shared_ptr<StructSpecifier> create(const std::string &structName, shared_ptr<Node> structDeclaration) {
            auto base = std::make_shared<StructSpecifier>();
            if (structName.empty()) {
                base->hasStructName = false;
            } else {
                base->structName = structName;
                base->hasStructName = true;
            }
            base->structDeclaration = std::move(structDeclaration);
            base->hasDeclaration = true;
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->TypeSpecifier::_get_name() + "::StructSpecifier";
        }
    };

    class StructDeclList : public Node {
        shared_ptr<Node> *structDeclarations = nullptr;
        size_t size = 0;
    public:
        ~StructDeclList() override {
            delete[] structDeclarations;
        }

        [[nodiscard]] size_t get_size() const {
            return size;
        }

        shared_ptr<Node> *begin() override {
            return structDeclarations;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return structDeclarations;
        }

        shared_ptr<Node> *end() override {
            return structDeclarations + size;
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return structDeclarations + size;
        }

        static shared_ptr<StructDeclList> create(std::vector<shared_ptr<Node> > structDeclarations) {
            auto base = std::make_shared<StructDeclList>();
            base->structDeclarations = new shared_ptr<Node>[structDeclarations.size()];
            base->size = structDeclarations.size();
            std::ranges::copy(structDeclarations, base->structDeclarations);
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "StructDeclList";
        }
    };

    class StructDecl : public Node {
        shared_ptr<Node> *nodes = nullptr;
        size_t size = 0;
    public:
        [[nodiscard]] size_t get_size() const {
            return size;
        }

        [[nodiscard]] shared_ptr<Node> get_declarator() const {
            return nodes[0];
        }

        shared_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        shared_ptr<Node> *end() override {
            return nodes + size;
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return nodes + size;
        }

        static shared_ptr<StructDecl> create(std::vector<shared_ptr<Node> > specifier_quals,
                                             shared_ptr<Node> declarator) {
            auto base = std::make_shared<StructDecl>();
            base->nodes = new shared_ptr<Node>[base->get_size() + 1];
            base->nodes[0] = std::move(declarator);
            base->size = base->get_size();
            std::ranges::copy(specifier_quals, base->nodes + 1);
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "StructDecl";
        }
    };

    class TypeQualifier : public Node {
        // Empty because TypeQualifier is implied to represent a "const" in slimmed down language spec
    public:
        static shared_ptr<TypeQualifier> create() {
            return std::make_shared<TypeQualifier>();
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "TypeQualifier";
        }
    };

    class Declarator : public Node {
        enum { DIRECT_DECLARATOR, POINTER, SUFFIX };

        shared_ptr<Node> nodes[3];
        bool isAbstract = false;
        bool hasPointer = false;
        bool hasSuffix = false;

    public:
        shared_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        shared_ptr<Node> *end() override {
            return &nodes[3];
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &nodes[3];
        }

        static shared_ptr<Declarator> create_abstract(shared_ptr<Node> pointer, shared_ptr<Node> suffix) {
            return create(nullptr, std::move(pointer), std::move(suffix));
        }

        static shared_ptr<Declarator> create(shared_ptr<Node> directDeclarator) {
            return create(std::move(directDeclarator), nullptr);
        }

        static shared_ptr<Declarator> create(shared_ptr<Node> directDeclarator, shared_ptr<Node> pointer) {
            return create(std::move(directDeclarator), std::move(pointer), nullptr);
        }

        static shared_ptr<Declarator> create(shared_ptr<Node> directDeclarator, shared_ptr<Node> pointer,
                                             shared_ptr<Node> suffix) {
            auto base = std::make_shared<Declarator>();
            base->nodes[DIRECT_DECLARATOR] = std::move(directDeclarator);
            base->nodes[POINTER] = std::move(pointer);
            base->nodes[SUFFIX] = std::move(suffix);
            base->isAbstract = directDeclarator == nullptr;
            base->hasPointer = pointer != nullptr;
            base->hasSuffix = suffix != nullptr;
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "Declarator";
        }
    };

    class Pointer : public Node {
        shared_ptr<Pointer> next_pointer = nullptr;
        bool is_const = false;

    public:
        void set_next_pointer(shared_ptr<Pointer> nextPointer) {
            this->next_pointer = std::move(nextPointer);
        }

        shared_ptr<Pointer> get_next_pointer() {
            return next_pointer;
        }

        shared_ptr<Node> *begin() override {
            if (next_pointer == nullptr) {
                return nullptr;
            }
            return reinterpret_cast<shared_ptr<Node> *>(&next_pointer);
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            if (next_pointer == nullptr) {
                return nullptr;
            }
            return reinterpret_cast<const shared_ptr<Node> *>(&next_pointer);
        }

        shared_ptr<Node> *end() override {
            if (next_pointer == nullptr) {
                return nullptr;
            }
            return reinterpret_cast<shared_ptr<Node> *>(&next_pointer + 1);
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            if (next_pointer == nullptr) {
                return nullptr;
            }
            return reinterpret_cast<const shared_ptr<Node> *>(&next_pointer + 1);
        }

        static shared_ptr<Pointer> create(const bool isConst) {
            auto base = std::make_shared<Pointer>();
            base->is_const = isConst;
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "Pointer";
        }
    };

    class DirectDeclarator : public Node {
        std::string identifier;

    public:
        static shared_ptr<DirectDeclarator> create(std::string identifier) {
            auto base = std::make_shared<DirectDeclarator>();
            base->identifier = std::move(identifier);
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "DirectDeclarator";
        }
    };

    class IndexDeclarator : public Node {
        shared_ptr<Node> constantExpression;

    public:
        shared_ptr<Node> *begin() override {
            return &constantExpression;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return &constantExpression;
        }

        shared_ptr<Node> *end() override {
            return &constantExpression + 1;
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &constantExpression + 1;
        }

        static shared_ptr<IndexDeclarator> create(shared_ptr<Node> constantExpression) {
            auto base = std::make_shared<IndexDeclarator>();
            base->constantExpression = std::move(constantExpression);
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "IndexDeclarator";
        }
    };

    class ParameterizedDeclarator : public Node {
        shared_ptr<Node> parameterList;

    public:
        shared_ptr<Node> *begin() override {
            return &parameterList;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return &parameterList;
        }

        shared_ptr<Node> *end() override {
            return &parameterList + 1;
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &parameterList + 1;
        }

        static shared_ptr<ParameterizedDeclarator> create(shared_ptr<Node> parameterList) {
            auto base = std::make_shared<ParameterizedDeclarator>();
            base->parameterList = std::move(parameterList);
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "ParameterizedDeclarator";
        }
    };

    class ParameterList : public Node {
        shared_ptr<Node> *parameters = nullptr;
        size_t size = 0;

    public:
        ~ParameterList() override {
            delete[] parameters;
        }

        shared_ptr<Node> *begin() override {
            return parameters;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return parameters;
        }

        shared_ptr<Node> *end() override {
            return parameters + size;
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return parameters + size;
        }

        static shared_ptr<ParameterList> create(std::vector<shared_ptr<Node>> parameters) {
            auto base = std::make_shared<ParameterList>();
            base->parameters = new shared_ptr<Node>[parameters.size()];
            base->size = parameters.size();
            std::ranges::copy(parameters, base->parameters);
            return base;
        }
    };

    class Parameter : public Node {
        enum {SPEC_QUALS, DECLARATOR};
        shared_ptr<Node> nodes[2];
        bool has_declarator = false;
    public:
        shared_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        shared_ptr<Node> *end() override {
            return &nodes[2];
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &nodes[2];
        }

        static shared_ptr<Parameter> create(shared_ptr<Node> specQuals) {
            return create(std::move(specQuals), nullptr);
        }

        static shared_ptr<Parameter> create(shared_ptr<Node> specQuals, shared_ptr<Node> declarator) {
            auto base = std::make_shared<Parameter>();
            base->has_declarator = declarator != nullptr;
            base->nodes[SPEC_QUALS] = std::move(specQuals);
            base->nodes[DECLARATOR] = std::move(declarator);
            return base;
        }
    };

    class InitDeclarator : public Node {
        enum { DECLARATOR, INITIALIZER };

        shared_ptr<Node> nodes[2];
        bool hasInitializer = true;

    public:
        [[nodiscard]] shared_ptr<Declarator> get_declarator() const {
            return std::dynamic_pointer_cast<Declarator>(nodes[DECLARATOR]);
        }

        [[nodiscard]] shared_ptr<Node> *begin() override {
            return &nodes[0];
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return &nodes[0];
        }

        [[nodiscard]] shared_ptr<Node> *end() override {
            return &nodes[INITIALIZER];
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &nodes[INITIALIZER];
        }

        static shared_ptr<InitDeclarator> create(shared_ptr<Node> declarator) {
            return create(std::move(declarator), nullptr);
        }

        static shared_ptr<InitDeclarator> create(shared_ptr<Node> declarator, const shared_ptr<Node> &initializer) {
            auto base = std::make_shared<InitDeclarator>();
            base->nodes[DECLARATOR] = std::move(declarator);
            base->nodes[INITIALIZER] = initializer;
            base->hasInitializer = initializer != nullptr;
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "InitDeclarator";
        }
    };

    class InitializerList : public Node {
        shared_ptr<Node> *initializers = nullptr;
        size_t initializerCount = 0;

    public:
        ~InitializerList() override {
            delete[] initializers;
        }

        [[nodiscard]] shared_ptr<Node> *begin() override {
            return &initializers[0];
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return &initializers[0];
        }

        [[nodiscard]] shared_ptr<Node> *end() override {
            return &initializers[initializerCount];
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &initializers[initializerCount];
        }

        static shared_ptr<InitializerList> create(std::vector<shared_ptr<Node> > initializers) {
            auto base = std::make_shared<InitializerList>();
            base->initializers = new shared_ptr<Node>[initializers.size()];
            base->initializerCount = initializers.size();
            std::ranges::copy(initializers.begin(), initializers.end(), base->initializers);
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "InitializerList";
        }
    };

    // TODO: Implement abstract declarator handling
    class TypeName : public Node {
        shared_ptr<Node> *nodes = nullptr;
        size_t size = 0;
    public:
        ~TypeName() override {
            delete[] nodes;
        }

        [[nodiscard]] shared_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        [[nodiscard]] shared_ptr<Node> *end() override {
            return &nodes[size];
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &nodes[size];
        }

        static shared_ptr<TypeName> create(std::vector<shared_ptr<Node>> specQualList) {
            auto base = std::make_shared<TypeName>();
            base->nodes = new shared_ptr<Node>[specQualList.size()];
            std::ranges::copy(specQualList.begin(), specQualList.end(), base->nodes);
            base->size = specQualList.size();
            return base;
        }

    };

    class FunctionDecl : public Node {
        enum { SPEC_QUALS, DECLARATOR, BODY };
        shared_ptr<Node> nodes[3];

    public:
        [[nodiscard]] shared_ptr<DeclSpecifiers> get_spec_quals() const {
            return std::dynamic_pointer_cast<DeclSpecifiers>(nodes[SPEC_QUALS]);
        }

        [[nodiscard]] shared_ptr<Declarator> get_declarator() const {
            return std::dynamic_pointer_cast<Declarator>(nodes[DECLARATOR]);
        }

        [[nodiscard]] shared_ptr<CompoundStatement> get_body() const {
            return std::dynamic_pointer_cast<CompoundStatement>(nodes[BODY]);
        }

        shared_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        shared_ptr<Node> *end() override {
            return &nodes[3];
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &nodes[3];
        }

        static shared_ptr<FunctionDecl> create(shared_ptr<Node> specQuals, shared_ptr<Node> declarator, shared_ptr<Node> body) {
            auto base = std::make_shared<FunctionDecl>();
            base->nodes[SPEC_QUALS] = std::move(specQuals);
            base->nodes[DECLARATOR] = std::move(declarator);
            base->nodes[BODY] = std::move(body);
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "FunctionDecl";
        }
    };

    class CompoundStatement : public Statement {
        shared_ptr<Node> *statements = nullptr;
        size_t size = 0;

    public:
        ~CompoundStatement() override {
            delete[] statements;
        }

        [[nodiscard]] size_t get_size() const {
            return size;
        }

        shared_ptr<Node> *begin() override {
            return statements;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return statements;
        }

        shared_ptr<Node> *end() override {
            return statements + size;
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return statements + size;
        }

        static shared_ptr<CompoundStatement> create(const std::vector<shared_ptr<Node> > &statements) {
            auto base = std::make_shared<CompoundStatement>();
            base->statements = new shared_ptr<Node>[statements.size()];
            base->size = statements.size();
            std::ranges::copy(statements, base->statements);
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "CompoundStatement";
        }
    };

    class ExpressionStatement : public Statement {
        shared_ptr<Node> expression;
    public:
        [[nodiscard]] shared_ptr<Node> *begin() override {
            return &expression;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return &expression;
        }

        [[nodiscard]] shared_ptr<Node> *end() override {
            return &expression + 1;
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &expression + 1;
        }

        static shared_ptr<ExpressionStatement> create(shared_ptr<Node> expression) {
            auto base = std::make_shared<ExpressionStatement>();
            base->expression = std::move(expression);
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Statement::_get_name() + "::ExpressionStatement";
        }
    };

    class Expression : public Node {
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "Expression";
        }
    };

    class ExpressionList : public Expression {
        shared_ptr<Expression> *expressions = nullptr;
        size_t size = 0;

    public:
        ~ExpressionList() override {
            delete[] expressions;
        }

        [[nodiscard]] size_t get_size() const {
            return size;
        }

        [[nodiscard]] shared_ptr<Node> *begin() override {
            return reinterpret_cast<shared_ptr<Node> *>(expressions);
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return reinterpret_cast<const shared_ptr<Node> *>(expressions);
        }

        [[nodiscard]] shared_ptr<Node> *end() override {
            return reinterpret_cast<shared_ptr<Node> *>(expressions + size);
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return reinterpret_cast<const shared_ptr<Node> *>(expressions + size);
        }

        static shared_ptr<ExpressionList> create(std::vector<shared_ptr<Expression> > expressions) {
            auto base = std::make_shared<ExpressionList>();
            base->expressions = new shared_ptr<Expression>[expressions.size()];
            base->size = expressions.size();
            std::ranges::copy(expressions.begin(), expressions.end(), base->expressions);
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Expression::_get_name() + "::ExpressionList";
        }
    };

    class Assignment : public Expression {
    public:
        enum Op { EQUAL, ADD, SUB, MUL, DIV, LEFT, RIGHT, AND, OR, XOR, MOD };

    private:
        enum { LVALUE, RVALUE };

        shared_ptr<Expression> nodes[2];
        Op operation = EQUAL;

    public:
        shared_ptr<Node> *begin() override {
            return reinterpret_cast<shared_ptr<Node> *>(nodes);
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return reinterpret_cast<const shared_ptr<Node> *>(nodes);
        }

        shared_ptr<Node> *end() override {
            return reinterpret_cast<shared_ptr<Node> *>(&nodes[2]);
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return reinterpret_cast<const shared_ptr<Node> *>(&nodes[2]);
        }

        static shared_ptr<Assignment> create(shared_ptr<Expression> lhs, shared_ptr<Expression> rhs,
                                                       Op operation) {
            auto base = std::make_shared<Assignment>();
            base->nodes[LVALUE] = std::move(lhs);
            base->nodes[RVALUE] = std::move(rhs);
            base->operation = operation;
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Expression::_get_name() + "::Assignment";
        }
    };

    class BinOp : public Expression {
    public:
        enum Op {
            LOGIC_OR,
            LOGIC_AND,
            INCLUSIVE_OR,
            EXCLUSIVE_OR,
            AND,
            EQUAL,
            NOT_EQUAL,
            LESS_THAN,
            GREATER_THAN,
            LESS_EQUAL,
            GREATER_EQUAL,
            LEFT_SHIFT,
            RIGHT_SHIFT,
            ADD,
            SUB,
            MUL,
            DIV,
            MOD
        };

    private:
        enum { LVALUE, RVALUE };
        shared_ptr<Expression> nodes[2];
        Op operation = LOGIC_OR;
    public:
        shared_ptr<Node> *begin() override {
            return reinterpret_cast<shared_ptr<Node> *>(nodes);
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return reinterpret_cast<const shared_ptr<Node> *>(nodes);
        }

        shared_ptr<Node> *end() override {
            return reinterpret_cast<shared_ptr<Node> *>(&nodes[2]);
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return reinterpret_cast<const shared_ptr<Node> *>(&nodes[2]);
        }

        static shared_ptr<BinOp> create(shared_ptr<Expression> lhs, shared_ptr<Expression> rhs, const Op operation) {
            auto base = std::make_shared<BinOp>();
            base->nodes[LVALUE] = std::move(lhs);
            base->nodes[RVALUE] = std::move(rhs);
            base->operation = operation;
            return base;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Expression::_get_name() + "::BinOp";
        }
    };

    class Cast : public Expression {
        enum { TYPE_NAME, EXPRESSION };
        shared_ptr<Node> nodes[2];

    public:
        shared_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        shared_ptr<Node> *end() override {
            return &nodes[2];
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &nodes[2];
        }

        static shared_ptr<Cast> create(shared_ptr<Node> typeName, shared_ptr<Expression> expr) {
            auto base = std::make_shared<Cast>();
            base->nodes[TYPE_NAME] = std::move(typeName);
            base->nodes[EXPRESSION] = std::move(expr);
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Expression::_get_name() + "::Cast";
        }
    };

    class UnaryOp : public Expression {
    public:
        enum Op {
            INCREMENT,
            DECREMENT,
            ADDRESS_OF,
            DEREFERENCE,
            POSITIVE,
            NEGATIVE,
            NEGATE,
            INVERT,
            SIZEOF
        };
    private:
        Op operation = INCREMENT;
        shared_ptr<Expression> rhs;
    public:
        static shared_ptr<UnaryOp> create(shared_ptr<Expression> expr, const Op operation) {
            auto base = std::make_shared<UnaryOp>();
            base->rhs = std::move(expr);
            base->operation = operation;
            return base;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Expression::_get_name() + "::UnaryOp";
        }
    };

    class SizeofType : public Expression {
        shared_ptr<Node> typeName;

    public:
        shared_ptr<Node> *begin() override {
            return &typeName;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return &typeName;
        }

        shared_ptr<Node> *end() override {
            return &typeName + 1;
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &typeName + 1;
        }

        static shared_ptr<SizeofType> create(shared_ptr<Node> typeName) {
            auto base = std::make_shared<SizeofType>();
            base->typeName = std::move(typeName);
            return base;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Expression::_get_name() + "::SizeofType";
        }
    };

    class Postfix : public Expression {
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Expression::_get_name() + "::Postfix";
        }
    };

    class IndexExpression : public Postfix {
        enum {EXPRESSION, INDEX};
        shared_ptr<Node> nodes[2];
    public:
        shared_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        shared_ptr<Node> *end() override {
            return &nodes[2];
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &nodes[2];
        }

        static shared_ptr<IndexExpression> create(shared_ptr<Node> expression, shared_ptr<Node> index) {
            auto base = std::make_shared<IndexExpression>();
            base->nodes[EXPRESSION] = std::move(expression);
            base->nodes[INDEX] = std::move(index);
            return base;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Postfix::_get_name() + "::IndexExpression";
        }
    };

    class FunctionCall : public Postfix {
        enum {EXPRESSION, ARGUMENT_LIST};
        shared_ptr<Node> nodes[2];
    public:
        shared_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        shared_ptr<Node> *end() override {
            return &nodes[2];
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &nodes[2];
        }

        static shared_ptr<FunctionCall> create(shared_ptr<Node> expression) {
            return create(std::move(expression), nullptr);
        }

        static shared_ptr<FunctionCall> create(shared_ptr<Node> expression, shared_ptr<Node> argumentList) {
            auto base = std::make_shared<FunctionCall>();
            base->nodes[EXPRESSION] = std::move(expression);
            base->nodes[ARGUMENT_LIST] = std::move(argumentList);
            return base;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Postfix::_get_name() + "::FunctionCall";
        }
    };

    class PostAssignment : public Postfix {
    public:
        enum AssignmentType {INCREMENT, DECREMENT};
    private:
        shared_ptr<Node> expression;
        AssignmentType operation = INCREMENT;
    public:
        shared_ptr<Node> *begin() override {
            return &expression;
        }
        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return &expression;
        }

        shared_ptr<Node> *end() override {
            return &expression + 1;
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &expression + 1;
        }

        static shared_ptr<PostAssignment> create(shared_ptr<Node> expression, const AssignmentType operation) {
            auto base = std::make_shared<PostAssignment>();
            base->expression = std::move(expression);
            base->operation = operation;
            return base;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Postfix::_get_name() + "::PostAssignment";
        }
    };

    class MemberAccess : public Postfix {
    public:
        enum MemberAccessType {
            POINTER,
            MEMBER,
        };
    private:
        shared_ptr<Node> expression;
        MemberAccessType operation = MEMBER;
        std::string identifier;
    public:
        shared_ptr<Node> *begin() override {
            return &expression;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return &expression;
        }

        shared_ptr<Node> *end() override {
            return &expression + 1;
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &expression + 1;
        }

        static shared_ptr<MemberAccess> create(shared_ptr<Node> expression, const MemberAccessType operation, std::string identifier) {
            auto base = std::make_shared<MemberAccess>();
            base->expression = std::move(expression);
            base->operation = operation;
            base->identifier = std::move(identifier);
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Postfix::_get_name() + "::MemberAccess";
        }
    };

    class Identifier: public Expression {
        std::string identifier;
    public:
        [[nodiscard]] const std::string &get_value() const {
            return identifier;
        }

        static shared_ptr<Identifier> create(std::string identifier) {
            auto base = std::make_shared<Identifier>();
            base->identifier = std::move(identifier);
            return base;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Expression::_get_name() + "::Identifier";
        }
    };

    class Constant : public Expression {
        std::string constant;
    public:
        [[nodiscard]] const std::string &get_value() const {
            return constant;
        }

        static shared_ptr<Constant> create(std::string constant) {
            auto base = std::make_shared<Constant>();
            base->constant = std::move(constant);
            return base;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Expression::_get_name() + "::Constant";
        }
    };

    class StringLiteral : public Expression {
        std::string literal;
    public:
        [[nodiscard]] const std::string &get_value() const {
            return literal;
        }

        static shared_ptr<StringLiteral> create(std::string literal) {
            auto base = std::make_shared<StringLiteral>();
            base->literal = std::move(literal);
            return base;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override{
            return this->Expression::_get_name() + "::StringLiteral";
        }
    };

    class SelectionStatement : public Statement {
        enum { CONDITION, IF_BODY, ELSE_BODY };

        shared_ptr<Node> nodes[3];
        bool hasElse = false;

    public:
        [[nodiscard]] shared_ptr<Expression> get_condition() const {
            return std::dynamic_pointer_cast<Expression>(nodes[CONDITION]);
        }

        [[nodiscard]] shared_ptr<CompoundStatement> get_then() const {
            return std::dynamic_pointer_cast<CompoundStatement>(nodes[IF_BODY]);
        }

        [[nodiscard]] shared_ptr<CompoundStatement> get_else() const {
            if (!hasElse) return nullptr;
            return std::dynamic_pointer_cast<CompoundStatement>(nodes[ELSE_BODY]);
        }

        shared_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        shared_ptr<Node> *end() override {
            if (hasElse) return &nodes[ELSE_BODY];
            return &nodes[IF_BODY];
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            if (hasElse) return &nodes[ELSE_BODY];
            return &nodes[IF_BODY];
        }

        static shared_ptr<SelectionStatement> create(shared_ptr<Node> condition, shared_ptr<Node> trueBody) {
            auto base = std::make_shared<SelectionStatement>();
            base->nodes[0] = std::move(condition);
            base->nodes[1] = std::move(trueBody);
            base->nodes[2] = nullptr;
            return base;
        }

        static shared_ptr<SelectionStatement> create(const shared_ptr<Node> &condition,
                                                     const shared_ptr<Node> &trueBody, shared_ptr<Node> falseBody) {
            auto base = create(condition, trueBody);
            base->nodes[2] = std::move(falseBody);
            base->hasElse = true;
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Statement::_get_name() + "::SelectionStatement";
        }
    };

    class WhileStatement : public Statement {
        enum { CONDITION, BODY };

        shared_ptr<Node> nodes[2];

    public:
        [[nodiscard]] shared_ptr<Expression> get_condition() const {
            return std::dynamic_pointer_cast<Expression>(nodes[CONDITION]);
        }

        [[nodiscard]] shared_ptr<CompoundStatement> get_body() const {
            return std::dynamic_pointer_cast<CompoundStatement>(nodes[BODY]);
        }

        shared_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        shared_ptr<Node> *end() override {
            return &nodes[1];
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &nodes[1];
        }

        static shared_ptr<WhileStatement> create(shared_ptr<Node> condition, shared_ptr<Node> body) {
            auto base = std::make_shared<WhileStatement>();
            base->nodes[0] = std::move(condition);
            base->nodes[1] = std::move(body);
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Statement::_get_name() + "::WhileStatement";
        }
    };

    class DoStatement : public Statement {
        enum { CONDITION, BODY };

        shared_ptr<Node> nodes[2];

    public:
        [[nodiscard]] shared_ptr<Expression> get_condition() const {
            return std::dynamic_pointer_cast<Expression>(nodes[CONDITION]);
        }

        [[nodiscard]] shared_ptr<CompoundStatement> get_body() const {
            return std::dynamic_pointer_cast<CompoundStatement>(nodes[BODY]);
        }

        shared_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        shared_ptr<Node> *end() override {
            return &nodes[1];
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &nodes[1];
        }

        static shared_ptr<DoStatement> create(shared_ptr<Node> condition, shared_ptr<Node> body) {
            auto base = std::make_shared<DoStatement>();
            base->nodes[CONDITION] = std::move(condition);
            base->nodes[BODY] = std::move(body);
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Statement::_get_name() + "::DoStatement";
        }
    };

    class ForStatement : public Statement {
        enum { INITIALIZATION, CONDITION, INCREMENT, BODY };

        shared_ptr<Node> nodes[4];
        bool hasInit = true;
        bool hasCondition = true;
        bool hasIncrement = true;

    public:
        [[nodiscard]] shared_ptr<Expression> get_initialization() const {
            if (!hasInit) return nullptr;
            return std::dynamic_pointer_cast<Expression>(nodes[INITIALIZATION]);
        }

        [[nodiscard]] shared_ptr<Expression> get_condition() const {
            if (!hasCondition) return nullptr;
            return std::dynamic_pointer_cast<Expression>(nodes[CONDITION]);
        }

        [[nodiscard]] shared_ptr<Expression> get_increment() const {
            if (!hasIncrement) return nullptr;
            return std::dynamic_pointer_cast<Expression>(nodes[INCREMENT]);
        }

        [[nodiscard]] shared_ptr<CompoundStatement> get_body() const {
            return std::dynamic_pointer_cast<CompoundStatement>(nodes[BODY]);
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &nodes[BODY];
        }

        static shared_ptr<ForStatement> create(const shared_ptr<Node> &body) {
            return create(body, nullptr, nullptr, nullptr);
        }

        static shared_ptr<ForStatement> create(shared_ptr<Node> body, shared_ptr<Node> initialization,
                                               shared_ptr<Node> condition, shared_ptr<Node> increment) {
            auto base = std::make_shared<ForStatement>();
            if (initialization == nullptr) {
                base->hasInit = false;
            }
            if (condition == nullptr) {
                base->hasCondition = false;
            }
            if (increment == nullptr) {
                base->hasIncrement = false;
            }
            base->nodes[INITIALIZATION] = std::move(initialization);
            base->nodes[CONDITION] = std::move(condition);
            base->nodes[INCREMENT] = std::move(increment);
            base->nodes[BODY] = std::move(body);
            return base;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Statement::_get_name() + "::ForStatement";
        }
    };

    class ControlStatement : public Statement {
        shared_ptr<Node> returnExpression = nullptr;

    public:
        enum State { BREAK, CONTINUE, RETURN };

        [[nodiscard]] bool is_break() const { return state == BREAK; }
        [[nodiscard]] bool is_continue() const { return state == CONTINUE; }
        [[nodiscard]] bool is_return() const { return state == RETURN; }

        [[nodiscard]] shared_ptr<Expression> get_return_expression() const {
            if (!is_return()) return nullptr;
            return std::dynamic_pointer_cast<Expression>(returnExpression);
        }

        shared_ptr<Node> *begin() override {
            return &returnExpression;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return &returnExpression;
        }

        shared_ptr<Node> *end() override {
            return &returnExpression;
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &returnExpression;
        }

        static shared_ptr<ControlStatement> create(const State state) {
            auto base = std::make_shared<ControlStatement>();
            base->state = state;
            return base;
        }

        static shared_ptr<ControlStatement> create(shared_ptr<Node> returnExpression) {
            auto base = std::make_shared<ControlStatement>();
            base->state = RETURN;
            base->returnExpression = std::move(returnExpression);
            return base;
        }

    private:
        State state = BREAK;

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Statement::_get_name() + "::ControlStatement";
        }
    };
} // namespace AST
#endif // !AST_H
