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
            delete nodes;
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
        enum TypeSpecifierType { VOID, CHAR, SHORT, INT, LONG, UNSIGNED, SIGNED, STRUCT, ENUM};

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
        // TODO: Replace with node list
        std::vector<shared_ptr<Node>> structDeclarations;
    public:
        static shared_ptr<StructDeclList> create(std::vector<shared_ptr<Node>> structDeclarations) {
            auto base = std::make_shared<StructDeclList>();
            base->structDeclarations = std::move(structDeclarations);
            return base;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "StructDeclList";
        }
    };

    class StructDecl : public Node {
        // TODO: Replace with node list
        std::vector<shared_ptr<Node>> specifier_quals;
        shared_ptr<Node> declarator;

    public:
        static shared_ptr<StructDecl> create(std::vector<shared_ptr<Node>> specifier_quals, shared_ptr<Node> declarator) {
            auto base = std::make_shared<StructDecl>();
            base->specifier_quals = std::move(specifier_quals);
            base->declarator = std::move(declarator);
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
        enum {DIRECT_DECLARATOR, POINTER, SUFFIX};
        shared_ptr<Node> nodes[3];
        bool hasPointer = false;
        bool hasSuffix = false;
    public:
        static shared_ptr<Declarator> create(shared_ptr<Node> declarator) {
            return create(std::move(declarator), nullptr);
        }

        static shared_ptr<Declarator> create(shared_ptr<Node> directDeclarator, shared_ptr<Node> pointer) {
            return create(std::move(directDeclarator), std::move(pointer), nullptr);
        }

        static shared_ptr<Declarator> create(shared_ptr<Node> directDeclarator, shared_ptr<Node> pointer, shared_ptr<Node> suffix) {
            auto base = std::make_shared<Declarator>();
            base->nodes[DIRECT_DECLARATOR] = std::move(directDeclarator);
            base->nodes[POINTER] = std::move(pointer);
            base->nodes[SUFFIX] = std::move(suffix);
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
        shared_ptr<Pointer> nextPointer = nullptr;
        bool isConst = false;
    public:
        void set_next_pointer(shared_ptr<Pointer> nextPointer) {
            this->nextPointer = std::move(nextPointer);
        }

        shared_ptr<Pointer> get_next_pointer() {
            return nextPointer;
        }

        shared_ptr<Node> *begin() override {
            if (nextPointer == nullptr) {
                return nullptr;
            }
            return reinterpret_cast<shared_ptr<Node> *>(&nextPointer);
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            if (nextPointer == nullptr) {
                return nullptr;
            }
            return reinterpret_cast<const shared_ptr<Node> *>(&nextPointer);
        }

        shared_ptr<Node> *end() override {
            if (nextPointer == nullptr) {
                return nullptr;
            }
            return reinterpret_cast<shared_ptr<Node> *>(&nextPointer + 1);
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            if (nextPointer == nullptr) {
                return nullptr;
            }
            return reinterpret_cast<const shared_ptr<Node> *>(&nextPointer + 1);
        }

        static shared_ptr<Pointer> create(const bool isConst) {
            auto base = std::make_shared<Pointer>();
            base->isConst = isConst;
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
        shared_ptr<Node> * begin() override {
            return &constantExpression;
        }

        [[nodiscard]] const shared_ptr<Node> * begin() const override {
            return &constantExpression;
        }

        shared_ptr<Node> * end() override {
            return &constantExpression + 1;
        }

        [[nodiscard]] const shared_ptr<Node> * end() const override {
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
        shared_ptr<Node> * begin() override {
            return &parameterList;
        }

        [[nodiscard]] const shared_ptr<Node> * begin() const override {
            return &parameterList;
        }

        shared_ptr<Node> * end() override {
            return &parameterList + 1;
        }

        [[nodiscard]] const shared_ptr<Node> * end() const override {
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

    class InitDeclarator : public Node {
        enum {DECLARATOR, INITIALIZER};
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

        static shared_ptr<InitializerList> create(std::vector<shared_ptr<Node>> initializers) {
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

    class FunctionDecl : public Node {
        shared_ptr<Node> body;

    public:
        [[nodiscard]] shared_ptr<CompoundStatement> get_body() const {
            return std::dynamic_pointer_cast<CompoundStatement>(body);
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return &body;
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return &body;
        }

        static shared_ptr<FunctionDecl> create(shared_ptr<Node> body) {
            auto base = std::make_shared<FunctionDecl>();
            base->body = std::move(body);
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

        shared_ptr<Node> * begin() override {
            return statements;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return statements;
        }

        shared_ptr<Node> * end() override {
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

    class Expression : public Statement {
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Statement::_get_name() + "::Expression";
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

        [[nodiscard]] shared_ptr<Node> * begin() override {
            return reinterpret_cast<shared_ptr<Node> *>(expressions);
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return reinterpret_cast<const shared_ptr<Node> *>(expressions);
        }

        [[nodiscard]] shared_ptr<Node> * end() override {
            return reinterpret_cast<shared_ptr<Node> *>(expressions + size);
        }

        [[nodiscard]] const shared_ptr<Node> *end() const override {
            return reinterpret_cast<const shared_ptr<Node> *> (expressions + size);
        }

        static shared_ptr<ExpressionList> create(std::vector<shared_ptr<Expression>> expressions) {
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

    class

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

        shared_ptr<Node> * begin() override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        shared_ptr<Node> * end() override {
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

        shared_ptr<Node> * begin() override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        shared_ptr<Node> * end() override {
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

        shared_ptr<Node> * begin() override {
            return nodes;
        }

        [[nodiscard]] const shared_ptr<Node> *begin() const override {
            return nodes;
        }

        shared_ptr<Node> * end() override {
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
