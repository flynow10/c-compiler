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
    };

    class TranslationUnit : public Node {
        shared_ptr<Node> *nodes = nullptr;
        size_t size = 0;

    public:
        ~TranslationUnit() override {
            delete nodes;
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
    };

    class Statement : public Node {
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
    };

    class TypeSpecifier : public Node {
    public:
        enum TypeSpecifierType { VOID, CHAR, SHORT, INT, LONG, UNSIGNED, SIGNED, STRUCT, ENUM, TYPENAME };

        static shared_ptr<TypeSpecifier> create(const TypeSpecifierType type) {
            if (type == STRUCT || type == ENUM || type == TYPENAME) {
                throw std::runtime_error("Cannot create specific type specifier with general factor method");
            }
            auto base = std::make_shared<TypeSpecifier>();
            base->type = type;
            return base;
        }

    protected:
        TypeSpecifierType type = VOID;
    };

    class StructSpecifier : public TypeSpecifier {
        std::string structName;
        bool hasStructName = false;
    };

    class EnumSpecifier : public TypeSpecifier {};

    class TypeQualifier : public Node {
    public:
        static shared_ptr<TypeQualifier> create() {
            auto base = std::make_shared<TypeQualifier>();
            return base;
        }
    };

    class TypenameSpecifier : public TypeSpecifier {
        std::string typeName;
    public:
        [[nodiscard]] std::string get_name() const {
            return typeName;
        }

        static shared_ptr<TypenameSpecifier> create(std::string name) {
            auto base = std::make_shared<TypenameSpecifier>();
            base->type = TYPENAME;
            base->typeName = std::move(name);
            return base;
        }
    };

    class Declarator : public Node {

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
    };

    class CompoundStatement : public Statement {
        shared_ptr<Node> *statements = nullptr;
        size_t size = 0;

    public:
        ~CompoundStatement() override {
            delete statements;
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
    };

    class Expression : public Statement {
    };

    class ExpressionList : public Expression {

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
    };
} // namespace AST
#endif // !AST_H
