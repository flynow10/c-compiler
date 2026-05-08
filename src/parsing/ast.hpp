#ifndef AST_H
#define AST_H

#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include <span>

#include "ast.hpp"
#include "../casting.hpp"
#include "../sema/types.hpp"

using std::unique_ptr;
class SymbolTable;
struct Entry;

namespace AST {
    class Node;
    class Statement;
    class Expression;
    class CompoundStatement;
    class InitDeclaratorList;

    using const_iterator = unique_ptr<Node> *;

    std::string to_string(const Node *node, int indent);

    inline std::string to_string(const Node *node) { return to_string(node, 0); }

    class Node {
    public:
        enum NodeKind {
            NK_TranslationUnit,
            NK_DeclSpecifiers,
            NK_StorageClass,
            NK_TypeSpecifier,
            NK_StructSpecifier,
            NK_StructDeclList,
            NK_StructDecl,
            NK_TypeQualifier,
            NK_Declarator,
            NK_Pointer,
            NK_DirectDeclarator,
            NK_IndexDeclarator,
            NK_ParameterizedDeclarator,
            NK_ParameterList,
            NK_Parameter,
            NK_InitDeclaratorList,
            NK_InitDeclarator,
            NK_InitializerList,
            NK_TypeName,
            NK_FunctionDecl,
            NK_Statement,
            NK_Decl,
            NK_CompoundStatement,
            NK_ExpressionStatement,
            NK_SelectionStatement,
            NK_WhileStatement,
            NK_DoStatement,
            NK_ForStatement,
            NK_ControlStatement,
            NK_LastStatement,
            NK_Expression,
            NK_ExpressionList,
            NK_Assignment,
            NK_BinOp,
            NK_Cast,
            NK_UnaryOp,
            NK_SizeofType,
            NK_Postfix,
            NK_IndexExpression,
            NK_FunctionCall,
            NK_PostAssignment,
            NK_MemberAccess,
            NK_LastPostfix,
            NK_Identifier,
            NK_Constant,
            NK_StringLiteral,
            NK_LastExpression
        };
    private:
        const NodeKind Kind;
    public:
        [[nodiscard]] NodeKind getKind() const { return Kind; }

        explicit Node(const NodeKind K) : Kind(K) {}
        virtual ~Node() = default;
        // Node(const Node &node) = delete;


        virtual unique_ptr<Node> *begin() {
            return nullptr;
        }

        [[nodiscard]] virtual const unique_ptr<Node> *begin() const {
            return nullptr;
        }

        virtual unique_ptr<Node> *end() {
            return nullptr;
        }

        [[nodiscard]] virtual const unique_ptr<Node> *end() const {
            return nullptr;
        }

        friend std::ostream &operator<<(std::ostream &os, const std::unique_ptr<Node> &obj) {
            return os << to_string(obj.get());
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
    public:
        TranslationUnit() : Node(NK_TranslationUnit) {}
    private:
        unique_ptr<Node> *nodes = nullptr;
        size_t size = 0;
#ifndef NDEBUG
        std::span<unique_ptr<Node>> _nodes = {nodes, 0};
#endif

        // Semantic analysis
        std::shared_ptr<SymbolTable> symbolTable;

    public:
        ~TranslationUnit() override {
            delete[] nodes;
        }

        [[nodiscard]] size_t get_size() const {
            return size;
        }

        // Semantic analysis

        void set_symbol_table(std::shared_ptr<SymbolTable> newTable) {
            this->symbolTable = std::move(newTable);
        }

        std::shared_ptr<SymbolTable> &get_symbol_table() {
            return symbolTable;
        }

        [[nodiscard]] SymbolTable *get_symbol_table_raw() const {
            return symbolTable.get();
        }

        unique_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return nodes;
        }

        unique_ptr<Node> *end() override {
            return &nodes[size];
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return &nodes[size];
        }

        static unique_ptr<TranslationUnit> create(std::vector<unique_ptr<Node>> &nodes) {
            auto base = std::make_unique<TranslationUnit>();
            base->nodes = new unique_ptr<Node>[nodes.size()];
            base->size = nodes.size();
            for (int i = 0; i < nodes.size(); ++i) {
                base->nodes[i] = std::move(nodes[i]);
            }
#ifndef NDEBUG
            base->_nodes = {base->nodes, base->size};
#endif

            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_TranslationUnit;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "TranslationUnit";
        }
    };

    class Statement : public Node {
    public:
        explicit Statement(const NodeKind K) : Node(K) {}

        static bool classof(const Node *node) {
            return node->getKind() >= NK_Statement && node->getKind() <= NK_LastStatement;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "Statement";
        }
    };

    class Decl : public Statement {
        enum {DECL_SPECS, DECLARATOR_LIST};
        unique_ptr<Node> nodes[2];
    public:
        Decl() : Statement(NK_Decl) {}

        [[nodiscard]] Node *get_decl_specs() const {
            return nodes[DECL_SPECS].get();
        }

        [[nodiscard]] InitDeclaratorList *get_declarators() const {
            return cast<InitDeclaratorList>(nodes[DECLARATOR_LIST].get());
        }

        [[nodiscard]] unique_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return nodes;
        }

        [[nodiscard]] unique_ptr<Node> *end() override {
            return nodes + 2;
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return nodes + 2;
        }

        static unique_ptr<Decl> create(unique_ptr<Node> spec_quals,
                                       unique_ptr<Node> declarators) {
            auto base = std::make_unique<Decl>();
            base->nodes[DECL_SPECS] = std::move(spec_quals);
            base->nodes[DECLARATOR_LIST] = std::move(declarators);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_Decl;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "Decl";
        }
    };

    class DeclSpecifiers : public Node {
        std::unique_ptr<Node> *nodes = nullptr;
        size_t size = 0;
    public:
        DeclSpecifiers() : Node(NK_DeclSpecifiers) {}
        ~DeclSpecifiers() override {
            delete[] nodes;
        }

        unique_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return nodes;
        }

        [[nodiscard]] unique_ptr<Node> *end() override {
            return nodes + size;
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return nodes + size;
        }

        Node *operator[](const std::size_t index) const {
            return nodes[index].get();
        }

        [[nodiscard]] size_t get_size() const {
            return size;
        }

        static unique_ptr<DeclSpecifiers> create(std::vector<unique_ptr<Node> > &nodes) {
            auto base = std::make_unique<DeclSpecifiers>();
            base->nodes = new unique_ptr<Node>[nodes.size()];
            base->size = nodes.size();
            for (int i = 0; i < nodes.size(); ++i) {
                base->nodes[i] = std::move(nodes[i]);
            }
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_DeclSpecifiers;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "DeclSpecifiers";
        }
    };

    class StorageClass : public Node {
    public:
        StorageClass() : Node(NK_StorageClass) {}
        enum StorageClassType { TYPEDEF, STATIC, AUTO };

        [[nodiscard]] StorageClassType get_type() const {
            return type;
        }

        static unique_ptr<StorageClass> create(StorageClassType type) {
            auto base = std::make_unique<StorageClass>();
            base->type = type;
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_StorageClass;
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
        TypeSpecifier() : Node(NK_TypeSpecifier) {}
        explicit TypeSpecifier(const NodeKind K) : Node(K) {}
        enum TypeSpecifierType { VOID, CHAR, SHORT, INT, LONG, UNSIGNED, SIGNED, STRUCT, ENUM };

        [[nodiscard]] TypeSpecifierType get_type() const {
            return type;
        }

        static unique_ptr<TypeSpecifier> create(const TypeSpecifierType type) {
            if (type == STRUCT || type == ENUM) {
                throw std::runtime_error("Cannot create specific type specifier with general factor method");
            }
            auto base = std::make_unique<TypeSpecifier>();
            base->type = type;
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() >= NK_TypeSpecifier && node->getKind() <= NK_StructSpecifier;
        }

    protected:
        TypeSpecifierType type = VOID;

        [[nodiscard]] std::string _get_name() const override {
            return "TypeSpecifier";
        }
    };

    class StructSpecifier : public TypeSpecifier {
        std::string structName;
        unique_ptr<Node> structDeclaration = nullptr;
        bool hasDeclaration = false;
        bool hasStructName = false;

    public:
        StructSpecifier() : TypeSpecifier(NK_StructSpecifier) {}

        unique_ptr<Node> *begin() override {
            if (!hasDeclaration)
                return nullptr;
            return &structDeclaration;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            if (!hasDeclaration)
                return nullptr;
            return &structDeclaration;
        }

        unique_ptr<Node> *end() override {
            if (!hasDeclaration)
                return nullptr;
            return &structDeclaration + 1;
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            if (!hasDeclaration)
                return nullptr;
            return &structDeclaration + 1;
        }

        [[nodiscard]] bool has_struct_name() const {
            return hasStructName;
        }

        [[nodiscard]] bool has_declaration() const {
            return hasDeclaration;
        }

        [[nodiscard]] const std::string & get_identifier() const {
            return structName;
        }

        [[nodiscard]] Node *get_declaration() const {
            return structDeclaration.get();
        }

        static unique_ptr<StructSpecifier> create(const std::string &structName) {
            auto base = std::make_unique<StructSpecifier>();
            base->structName = structName;
            base->hasStructName = true;
            base->hasDeclaration = false;
            return base;
        }

        static unique_ptr<StructSpecifier> create(const std::string &structName, unique_ptr<Node> structDeclaration) {
            auto base = std::make_unique<StructSpecifier>();
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

        static bool classof(const Node *node) {
            return node->getKind() == NK_StructSpecifier;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->TypeSpecifier::_get_name() + "::StructSpecifier";
        }
    };

    class StructDeclList : public Node {
        unique_ptr<Node> *structDeclarations = nullptr;
        size_t size = 0;
    public:
        StructDeclList() : Node(NK_StructDeclList) {}
        ~StructDeclList() override {
            delete[] structDeclarations;
        }

        [[nodiscard]] size_t get_size() const {
            return size;
        }

        unique_ptr<Node> *begin() override {
            return structDeclarations;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return structDeclarations;
        }

        unique_ptr<Node> *end() override {
            return structDeclarations + size;
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return structDeclarations + size;
        }

        static unique_ptr<StructDeclList> create(std::vector<unique_ptr<Node>> &structDeclarations) {
            auto base = std::make_unique<StructDeclList>();
            base->structDeclarations = new unique_ptr<Node>[structDeclarations.size()];
            base->size = structDeclarations.size();
            for (size_t i = 0; i < structDeclarations.size(); i++) {
                base->structDeclarations[i] = std::move(structDeclarations[i]);
            }
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_StructDeclList;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "StructDeclList";
        }
    };

    class StructDecl : public Node {
        enum {DECL_SPECS, DECLARATOR};
        unique_ptr<Node> nodes[2];
    public:
        StructDecl() : Node(NK_StructDecl) {}

        [[nodiscard]] Node *get_declarator() const {
            return nodes[DECLARATOR].get();
        }

        [[nodiscard]] Node *get_spec_qual() const {
            return nodes[DECL_SPECS].get();
        }

        unique_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return nodes;
        }

        unique_ptr<Node> *end() override {
            return nodes + 2;
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return nodes + 2;
        }

        static unique_ptr<StructDecl> create(unique_ptr<Node> specifier_quals,
                                             unique_ptr<Node> declarator) {
            auto base = std::make_unique<StructDecl>();
            base->nodes[DECL_SPECS] = std::move(specifier_quals);
            base->nodes[DECLARATOR] = std::move(declarator);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_StructDecl;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "StructDecl";
        }
    };

    class TypeQualifier : public Node {
        // Empty because TypeQualifier is implied to represent a "const" in slimmed down language spec
    public:
        TypeQualifier() : Node(NK_TypeQualifier) {}
        static unique_ptr<TypeQualifier> create() {
            return std::make_unique<TypeQualifier>();
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_TypeQualifier;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "TypeQualifier";
        }
    };

    class Declarator : public Node {
        enum { DIRECT_DECLARATOR, POINTER, SUFFIX };

        unique_ptr<Node> nodes[3];
        bool isAbstract = false;
        bool hasPointer = false;
        bool hasSuffix = false;

    public:
        Declarator() : Node(NK_Declarator) {}
        [[nodiscard]] Node *get_direct_declarator() const {
            return nodes[DIRECT_DECLARATOR].get();
        }

        [[nodiscard]] Node *get_pointer() const {
            return nodes[POINTER].get();
        }

        [[nodiscard]] Node *get_suffix() const {
            return nodes[SUFFIX].get();
        }

        [[nodiscard]] bool is_abstract() const {
            return isAbstract;
        }

        [[nodiscard]] bool has_pointer() const {
            return hasPointer;
        }

        [[nodiscard]] bool has_suffix() const {
            return hasSuffix;
        }

        unique_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return nodes;
        }

        unique_ptr<Node> *end() override {
            return &nodes[3];
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return &nodes[3];
        }

        static unique_ptr<Declarator> create_abstract(unique_ptr<Node> pointer, unique_ptr<Node> suffix) {
            return create(nullptr, std::move(pointer), std::move(suffix));
        }

        static unique_ptr<Declarator> create(unique_ptr<Node> directDeclarator) {
            return create(std::move(directDeclarator), nullptr);
        }

        static unique_ptr<Declarator> create(unique_ptr<Node> directDeclarator, unique_ptr<Node> pointer) {
            return create(std::move(directDeclarator), std::move(pointer), nullptr);
        }

        static unique_ptr<Declarator> create(unique_ptr<Node> directDeclarator, unique_ptr<Node> pointer,
                                             unique_ptr<Node> suffix) {
            auto base = std::make_unique<Declarator>();
            base->isAbstract = directDeclarator == nullptr;
            base->hasPointer = pointer != nullptr;
            base->hasSuffix = suffix != nullptr;
            base->nodes[DIRECT_DECLARATOR] = std::move(directDeclarator);
            base->nodes[POINTER] = std::move(pointer);
            base->nodes[SUFFIX] = std::move(suffix);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_Declarator;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "Declarator";
        }
    };

    class Pointer : public Node {
        unique_ptr<Pointer> next_pointer = nullptr;
        bool isConst = false;

    public:
        Pointer() : Node(NK_Pointer) {}
        void set_next_pointer(unique_ptr<Pointer> nextPointer) {
            this->next_pointer = std::move(nextPointer);
        }

        [[nodiscard]] Pointer *get_next_pointer() const {
            if (next_pointer == nullptr) {
                return nullptr;
            }
            return next_pointer.get();
        }

        [[nodiscard]] bool has_next_pointer() const {
            return next_pointer != nullptr;
        }

        [[nodiscard]] bool is_const() const {
            return isConst;
        }

        unique_ptr<Node> *begin() override {
            if (next_pointer == nullptr) {
                return nullptr;
            }
            return reinterpret_cast<unique_ptr<Node> *>(&next_pointer);
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            if (next_pointer == nullptr) {
                return nullptr;
            }
            return reinterpret_cast<const unique_ptr<Node> *>(&next_pointer);
        }

        unique_ptr<Node> *end() override {
            if (next_pointer == nullptr) {
                return nullptr;
            }
            return reinterpret_cast<unique_ptr<Node> *>(&next_pointer + 1);
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            if (next_pointer == nullptr) {
                return nullptr;
            }
            return reinterpret_cast<const unique_ptr<Node> *>(&next_pointer + 1);
        }

        static unique_ptr<Pointer> create(const bool isConst) {
            auto base = std::make_unique<Pointer>();
            base->isConst = isConst;
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_Pointer;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "Pointer";
        }
    };

    class DirectDeclarator : public Node {
        std::string identifier;

    public:
        DirectDeclarator() : Node(NK_DirectDeclarator) {}

        [[nodiscard]] const std::string &get_identifier() const {
            return identifier;
        }

        static unique_ptr<DirectDeclarator> create(std::string identifier) {
            auto base = std::make_unique<DirectDeclarator>();
            base->identifier = std::move(identifier);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_DirectDeclarator;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "DirectDeclarator";
        }
    };

    class IndexDeclarator : public Node {
        enum {CONSTANT_EXPRESSION, NEXT_SUFFIX};
        unique_ptr<Node> nodes[2];
        bool hasNext = false;
        bool hasExpression = false;

    public:
        IndexDeclarator() : Node(NK_IndexDeclarator) {}

        [[nodiscard]] bool has_expression() const {
            return hasExpression;
        }

        [[nodiscard]] Expression *get_expression() const {
            return cast<Expression>(nodes[CONSTANT_EXPRESSION].get());
        }

        [[nodiscard]] bool has_next_suffix() const {
            return hasNext;
        }

        [[nodiscard]] Node *get_next_suffix() const {
            return nodes[NEXT_SUFFIX].get();
        }

        unique_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return nodes;
        }

        unique_ptr<Node> *end() override {
            return &nodes[2];
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return &nodes[2];
        }

        static unique_ptr<IndexDeclarator> create(unique_ptr<Node> constantExpression, unique_ptr<Node> suffix) {
            auto base = std::make_unique<IndexDeclarator>();
            base->hasNext = suffix != nullptr;
            base->hasExpression = constantExpression != nullptr;
            base->nodes[CONSTANT_EXPRESSION] = std::move(constantExpression);
            base->nodes[NEXT_SUFFIX] = std::move(suffix);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_IndexDeclarator;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "IndexDeclarator";
        }
    };

    class ParameterList;

    class ParameterizedDeclarator : public Node {
        enum {PARAMETER_LIST, NEXT_SUFFIX};
        unique_ptr<Node> nodes[2];
        bool hasParameters = false;
        bool hasNext = false;

    public:
        ParameterizedDeclarator() : Node(NK_ParameterizedDeclarator) {}

        [[nodiscard]] bool has_parameters() const {
            return hasParameters;
        }

        [[nodiscard]] ParameterList *get_parameter_list() const {
            return cast<ParameterList>(nodes[PARAMETER_LIST].get());
        }

        [[nodiscard]] bool has_next_suffix() const {
            return hasNext;
        }

        [[nodiscard]] Node *get_next_suffix() const {
            return nodes[NEXT_SUFFIX].get();
        }

        unique_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return nodes;
        }

        unique_ptr<Node> *end() override {
            return &nodes[2];
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return &nodes[2];
        }

        static unique_ptr<ParameterizedDeclarator> create(unique_ptr<Node> parameterList, unique_ptr<Node> suffix) {
            auto base = std::make_unique<ParameterizedDeclarator>();
            base->hasNext = suffix != nullptr;
            base->hasParameters = parameterList != nullptr;
            base->nodes[PARAMETER_LIST] = std::move(parameterList);
            base->nodes[NEXT_SUFFIX] = std::move(suffix);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_ParameterizedDeclarator;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "ParameterizedDeclarator";
        }
    };

    class ParameterList : public Node {
        unique_ptr<Node> *parameters = nullptr;
        size_t size = 0;

    public:
        ParameterList() : Node(NK_ParameterList) {}
        ~ParameterList() override {
            delete[] parameters;
        }

        unique_ptr<Node> *begin() override {
            return parameters;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return parameters;
        }

        unique_ptr<Node> *end() override {
            return parameters + size;
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return parameters + size;
        }

        static unique_ptr<ParameterList> create(std::vector<unique_ptr<Node>> &parameters) {
            auto base = std::make_unique<ParameterList>();
            base->parameters = new unique_ptr<Node>[parameters.size()];
            base->size = parameters.size();
            for (size_t i = 0; i < parameters.size(); i++) {
                base->parameters[i] = std::move(parameters[i]);
            }
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_ParameterList;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "ParameterList";
        }
    };

    class Parameter : public Node {
        enum {SPEC_QUALS, DECLARATOR};
        unique_ptr<Node> nodes[2];
        bool hasDeclarator = false;
    public:
        Parameter() : Node(NK_Parameter) {}

        [[nodiscard]] DeclSpecifiers* get_decl_specs() const {
            return cast<DeclSpecifiers>(nodes[SPEC_QUALS].get());
        }

        [[nodiscard]] bool has_declarator() const {
            return hasDeclarator;
        }

        [[nodiscard]] Declarator *get_declarator() const {
            return cast<Declarator>(nodes[DECLARATOR].get());
        }

        unique_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return nodes;
        }

        unique_ptr<Node> *end() override {
            return &nodes[2];
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return &nodes[2];
        }

        static unique_ptr<Parameter> create(unique_ptr<Node> specQuals) {
            return create(std::move(specQuals), nullptr);
        }

        static unique_ptr<Parameter> create(unique_ptr<Node> specQuals, unique_ptr<Node> declarator) {
            auto base = std::make_unique<Parameter>();
            base->hasDeclarator = declarator != nullptr;
            base->nodes[SPEC_QUALS] = std::move(specQuals);
            base->nodes[DECLARATOR] = std::move(declarator);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_Parameter;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "Parameter";
        }
    };

    class InitDeclaratorList: public Node {
        unique_ptr<Node> *nodes = nullptr;
        size_t size = 0;
    public:
        InitDeclaratorList() : Node(NK_InitDeclaratorList) {}
        ~InitDeclaratorList() override {
            delete[] nodes;
        }

        unique_ptr<Node> * begin() override {
            return nodes;
        }

        [[nodiscard]] const unique_ptr<Node> * begin() const override {
            return nodes;
        }

        unique_ptr<Node> * end() override {
            return nodes + size;
        }

        [[nodiscard]] const unique_ptr<Node> * end() const override {
            return nodes + size;
        }

        [[nodiscard]] size_t get_size() const {
            return size;
        }

        Node *operator[](const size_t index) const {
            return nodes[index].get();
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_InitDeclaratorList;
        }

        static unique_ptr<InitDeclaratorList> create(std::vector<unique_ptr<Node>> &initDeclarators) {
            auto base = std::make_unique<InitDeclaratorList>();
            base->nodes = new unique_ptr<Node>[initDeclarators.size()];
            base->size = initDeclarators.size();
            for (size_t i = 0; i < initDeclarators.size(); i++) {
                base->nodes[i] = std::move(initDeclarators[i]);
            }
            return base;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "InitDeclaratorList";
        }
    };

    class InitDeclarator : public Node {
        enum { DECLARATOR, INITIALIZER };

        unique_ptr<Node> nodes[2];
        bool hasInitializer = true;

        // Semantic Analysis
        Entry * symbol_entry = nullptr;

    public:
        InitDeclarator() : Node(NK_InitDeclarator) {}
        [[nodiscard]] Declarator *get_declarator() const {
            return cast<Declarator>(nodes[DECLARATOR].get());
        }

        [[nodiscard]] bool has_initializer() const {
            return hasInitializer;
        }

        [[nodiscard]] Node *get_initializer() const {
            return nodes[INITIALIZER].get();
        }

        // Semantic Analysis

        Entry *get_symbol_entry() const {
            if (symbol_entry == nullptr) {
                throw std::runtime_error("Semantic analysis has not been performed on AST");
            }
            return symbol_entry;
        }

        void set_symbol_entry(Entry *symbolEntry) {
            this->symbol_entry = symbolEntry;
        }

        [[nodiscard]] unique_ptr<Node> *begin() override {
            return &nodes[0];
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return &nodes[0];
        }

        [[nodiscard]] unique_ptr<Node> *end() override {
            return &nodes[INITIALIZER];
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return &nodes[INITIALIZER];
        }

        static unique_ptr<InitDeclarator> create(unique_ptr<Node> declarator) {
            return create(std::move(declarator), nullptr);
        }

        static unique_ptr<InitDeclarator> create(unique_ptr<Node> declarator, unique_ptr<Node> initializer) {
            auto base = std::make_unique<InitDeclarator>();
            base->hasInitializer = initializer != nullptr;
            base->nodes[DECLARATOR] = std::move(declarator);
            base->nodes[INITIALIZER] = std::move(initializer);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_InitDeclarator;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "InitDeclarator";
        }
    };

    class InitializerList : public Node {
        unique_ptr<Node> *initializers = nullptr;
        size_t initializerCount = 0;

    public:
        InitializerList() : Node(NK_InitializerList) {}
        ~InitializerList() override {
            delete[] initializers;
        }

        [[nodiscard]] unique_ptr<Node> *begin() override {
            return &initializers[0];
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return &initializers[0];
        }

        [[nodiscard]] unique_ptr<Node> *end() override {
            return &initializers[initializerCount];
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return &initializers[initializerCount];
        }

        static unique_ptr<InitializerList> create(std::vector<unique_ptr<Node> > &initializers) {
            auto base = std::make_unique<InitializerList>();
            base->initializers = new unique_ptr<Node>[initializers.size()];
            base->initializerCount = initializers.size();
            for (size_t i = 0; i < initializers.size(); i++) {
                base->initializers[i] = std::move(initializers[i]);
            }
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_InitializerList;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "InitializerList";
        }
    };

    // TODO: Implement abstract declarator handling
    class TypeName : public Node {
        enum {DECL_SPECS, ABSTRACT_DECLARATOR};
        unique_ptr<Node> nodes[2];
    public:
        TypeName() : Node(NK_TypeName) {}

        [[nodiscard]] Node *get_abstract_declarator() const {
            return nodes[ABSTRACT_DECLARATOR].get();
        }

        [[nodiscard]] Node *get_decl_specs() const {
            return nodes[DECL_SPECS].get();
        }

        [[nodiscard]] unique_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return nodes;
        }

        [[nodiscard]] unique_ptr<Node> *end() override {
            return nodes + 2;
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return nodes + 2;
        }

        static unique_ptr<TypeName> create(unique_ptr<Node> declSpecs, unique_ptr<Node> abstractDeclarator) {
            auto base = std::make_unique<TypeName>();
            base->nodes[DECL_SPECS] = std::move(declSpecs);
            base->nodes[ABSTRACT_DECLARATOR] = std::move(abstractDeclarator);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_TypeName;
        }

        protected:
        [[nodiscard]] std::string _get_name() const override {
            return "TypeName";
        }
    };

    class FunctionDecl : public Node {
        enum { SPEC_QUALS, DECLARATOR, BODY };
        unique_ptr<Node> nodes[3];

        // Semantic Analysis
        std::shared_ptr<SymbolTable> symbolTable = nullptr;
        Entry *functionEntry = nullptr;

    public:
        FunctionDecl() : Node(NK_FunctionDecl) {}
        [[nodiscard]] DeclSpecifiers *get_spec_quals() const {
            return cast<DeclSpecifiers>(nodes[SPEC_QUALS].get());
        }

        [[nodiscard]] Declarator *get_declarator() const {
            return cast<Declarator>(nodes[DECLARATOR].get());
        }

        [[nodiscard]] CompoundStatement *get_body() const {
            return cast<CompoundStatement>(nodes[BODY].get());
        }


        void set_symbol_table(std::shared_ptr<SymbolTable> newTable) {
            symbolTable = std::move(newTable);
        }

        std::shared_ptr<SymbolTable> &get_symbol_table() {
            return symbolTable;
        }

        [[nodiscard]] SymbolTable *get_symbol_table_raw() const {
            return symbolTable.get();
        }

        void set_function_entry(Entry *entry) {
            functionEntry = entry;
        }

        Entry *get_function_entry() const {
            if (functionEntry == nullptr) {
                throw std::runtime_error("Semantic analysis has not been performed on AST");
            }
            return functionEntry;
        }

        unique_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return nodes;
        }

        unique_ptr<Node> *end() override {
            return &nodes[3];
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return &nodes[3];
        }

        static unique_ptr<FunctionDecl> create(unique_ptr<Node> specQuals, unique_ptr<Node> declarator, unique_ptr<Node> body) {
            auto base = std::make_unique<FunctionDecl>();
            base->nodes[SPEC_QUALS] = std::move(specQuals);
            base->nodes[DECLARATOR] = std::move(declarator);
            base->nodes[BODY] = std::move(body);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_FunctionDecl;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "FunctionDecl";
        }
    };

    class CompoundStatement : public Statement {
        unique_ptr<Node> *statements = nullptr;
        size_t size = 0;
        std::shared_ptr<SymbolTable> symbolTable;

    public:
        CompoundStatement() : Statement(NK_CompoundStatement) {}
        ~CompoundStatement() override {
            delete[] statements;
        }

        [[nodiscard]] size_t get_size() const {
            return size;
        }

        void set_symbol_table(std::shared_ptr<SymbolTable> newTable) {
            symbolTable = std::move(newTable);
        }

        std::shared_ptr<SymbolTable> &get_symbol_table() {
            return symbolTable;
        }

        [[nodiscard]] SymbolTable *get_symbol_table_raw() const {
            return symbolTable.get();
        }

        unique_ptr<Node> *begin() override {
            return statements;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return statements;
        }

        unique_ptr<Node> *end() override {
            return statements + size;
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return statements + size;
        }

        static unique_ptr<CompoundStatement> create(std::vector<unique_ptr<Node>> &statements) {
            auto base = std::make_unique<CompoundStatement>();
            base->statements = new unique_ptr<Node>[statements.size()];
            base->size = statements.size();
            for (size_t i = 0; i < statements.size(); i++) {
                base->statements[i] = std::move(statements[i]);
            }
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_CompoundStatement;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "CompoundStatement";
        }
    };

    class ExpressionStatement : public Statement {
        unique_ptr<Node> expression;
    public:
        ExpressionStatement() : Statement(NK_ExpressionStatement) {}

        [[nodiscard]] Expression *get_expression() const {
            return cast<Expression>(expression.get());
        }

        [[nodiscard]] unique_ptr<Node> *begin() override {
            return &expression;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return &expression;
        }

        [[nodiscard]] unique_ptr<Node> *end() override {
            return &expression + 1;
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return &expression + 1;
        }

        static unique_ptr<ExpressionStatement> create(unique_ptr<Node> expression) {
            auto base = std::make_unique<ExpressionStatement>();
            base->expression = std::move(expression);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_ExpressionStatement;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Statement::_get_name() + "::ExpressionStatement";
        }
    };

    class Expression : public Node {
        QualType type_info = {nullptr, false};

        friend Sema;
        void set_type(QualType type) {
            type_info = type;
        }
    public:
        explicit Expression(const NodeKind K) : Node(K) {}

        [[nodiscard]] bool has_type_info() const {
            return type_info.type != nullptr;
        }

        [[nodiscard]] const QualType & get_type_info() const {
            return type_info;
        }

        static bool classof(const Node *node) {
            return node->getKind() >= NK_Expression && node->getKind() <= NK_LastExpression;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return "Expression";
        }
    };

    class ExpressionList : public Expression {
        unique_ptr<Expression> *expressions = nullptr;
        size_t size = 0;

    public:
        ExpressionList() : Expression(NK_ExpressionList) {}
        ~ExpressionList() override {
            delete[] expressions;
        }

        [[nodiscard]] size_t get_size() const {
            return size;
        }

        [[nodiscard]] unique_ptr<Node> *begin() override {
            return reinterpret_cast<unique_ptr<Node> *>(expressions);
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return reinterpret_cast<const unique_ptr<Node> *>(expressions);
        }

        [[nodiscard]] unique_ptr<Node> *end() override {
            return reinterpret_cast<unique_ptr<Node> *>(expressions + size);
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return reinterpret_cast<const unique_ptr<Node> *>(expressions + size);
        }

        [[nodiscard]] Expression * operator[](const size_t index) const {
            return expressions[index].get();
        }

        static unique_ptr<ExpressionList> create(std::vector<unique_ptr<Expression>> &expressions) {
            auto base = std::make_unique<ExpressionList>();
            base->expressions = new unique_ptr<Expression>[expressions.size()];
            base->size = expressions.size();
            for (size_t i = 0; i < expressions.size(); i++) {
                base->expressions[i] = std::move(expressions[i]);
            }
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_ExpressionList;
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

        unique_ptr<Expression> nodes[2];
        Op operation = EQUAL;

    public:
        Assignment() : Expression(NK_Assignment) {}

        [[nodiscard]] Op get_operation() const {
            return operation;
        }

        [[nodiscard]] Expression *get_lhs() const {
            return nodes[LVALUE].get();
        }

        [[nodiscard]] Expression *get_rhs() const {
            return nodes[RVALUE].get();
        }

        unique_ptr<Node> *begin() override {
            return reinterpret_cast<unique_ptr<Node> *>(nodes);
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return reinterpret_cast<const unique_ptr<Node> *>(nodes);
        }

        unique_ptr<Node> *end() override {
            return reinterpret_cast<unique_ptr<Node> *>(&nodes[2]);
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return reinterpret_cast<const unique_ptr<Node> *>(&nodes[2]);
        }

        static unique_ptr<Assignment> create(unique_ptr<Expression> lhs, unique_ptr<Expression> rhs,
                                                       Op operation) {
            auto base = std::make_unique<Assignment>();
            base->nodes[LVALUE] = std::move(lhs);
            base->nodes[RVALUE] = std::move(rhs);
            base->operation = operation;
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_Assignment;
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
        unique_ptr<Expression> nodes[2];
        Op operation = LOGIC_OR;
    public:
        BinOp() : Expression(NK_BinOp) {}

        [[nodiscard]] Op get_operation() const {
            return operation;
        }

        [[nodiscard]] Expression *get_lhs() const {
            return nodes[LVALUE].get();
        }

        [[nodiscard]] Expression *get_rhs() const {
            return nodes[RVALUE].get();
        }

        unique_ptr<Node> *begin() override {
            return reinterpret_cast<unique_ptr<Node> *>(nodes);
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return reinterpret_cast<const unique_ptr<Node> *>(nodes);
        }

        unique_ptr<Node> *end() override {
            return reinterpret_cast<unique_ptr<Node> *>(&nodes[2]);
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return reinterpret_cast<const unique_ptr<Node> *>(&nodes[2]);
        }

        static unique_ptr<BinOp> create(unique_ptr<Expression> lhs, unique_ptr<Expression> rhs, const Op operation) {
            auto base = std::make_unique<BinOp>();
            base->nodes[LVALUE] = std::move(lhs);
            base->nodes[RVALUE] = std::move(rhs);
            base->operation = operation;
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_BinOp;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Expression::_get_name() + "::BinOp";
        }
    };

    class Cast : public Expression {
        enum { TYPE_NAME, EXPRESSION };
        unique_ptr<Node> nodes[2];

    public:
        Cast() : Expression(NK_Cast) {}

        [[nodiscard]] TypeName *get_type_name() const {
            return cast<TypeName>(nodes[TYPE_NAME].get());
        }

        [[nodiscard]] Expression *get_expression() const {
            return cast<Expression>(nodes[EXPRESSION].get());
        }

        unique_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return nodes;
        }

        unique_ptr<Node> *end() override {
            return &nodes[2];
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return &nodes[2];
        }

        static unique_ptr<Cast> create(unique_ptr<Node> typeName, unique_ptr<Expression> expr) {
            auto base = std::make_unique<Cast>();
            base->nodes[TYPE_NAME] = std::move(typeName);
            base->nodes[EXPRESSION] = std::move(expr);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_Cast;
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
        unique_ptr<Expression> rhs;
    public:
        UnaryOp() : Expression(NK_UnaryOp) {}

        [[nodiscard]] Op get_operation() const {
            return operation;
        }

        [[nodiscard]] Expression *get_rhs() const {
            return rhs.get();
        }

        unique_ptr<Node> * begin() override {
            return reinterpret_cast<unique_ptr<Node> *>(&rhs);
        }

        [[nodiscard]] const unique_ptr<Node> * begin() const override {
            return reinterpret_cast<const unique_ptr<Node> *>(&rhs);
        }

        unique_ptr<Node> * end() override {
            return reinterpret_cast<unique_ptr<Node> *>(&rhs + 1);
        }

        [[nodiscard]] const unique_ptr<Node> * end() const override {
            return reinterpret_cast<const unique_ptr<Node> *>(&rhs + 1);
        }

        static unique_ptr<UnaryOp> create(unique_ptr<Expression> expr, const Op operation) {
            auto base = std::make_unique<UnaryOp>();
            base->rhs = std::move(expr);
            base->operation = operation;
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_UnaryOp;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Expression::_get_name() + "::UnaryOp";
        }
    };

    class SizeofType : public Expression {
        unique_ptr<Node> typeName;

    public:
        SizeofType() : Expression(NK_SizeofType) {}

        [[nodiscard]] Node *get_type_name() const {
            return typeName.get();
        }

        unique_ptr<Node> *begin() override {
            return &typeName;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return &typeName;
        }

        unique_ptr<Node> *end() override {
            return &typeName + 1;
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return &typeName + 1;
        }

        static unique_ptr<SizeofType> create(unique_ptr<Node> typeName) {
            auto base = std::make_unique<SizeofType>();
            base->typeName = std::move(typeName);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_SizeofType;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Expression::_get_name() + "::SizeofType";
        }
    };

    class Postfix : public Expression {
    public:
        explicit Postfix(const NodeKind K) : Expression(K) {}

        static bool classof(const Node *node) {
            return node->getKind() >= NK_Postfix && node->getKind() <= NK_LastPostfix;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Expression::_get_name() + "::Postfix";
        }
    };

    class IndexExpression : public Postfix {
        enum {EXPRESSION, INDEX};
        unique_ptr<Expression> nodes[2];
    public:
        IndexExpression() : Postfix(NK_IndexExpression) {}

        [[nodiscard]] Expression *get_lhs() const {
            return nodes[EXPRESSION].get();
        }

        [[nodiscard]] Expression *get_index() const {
            return nodes[INDEX].get();
        }

        unique_ptr<Node> *begin() override {
            return reinterpret_cast<unique_ptr<Node> *>(nodes);
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return reinterpret_cast<const unique_ptr<Node> *>(nodes);
        }

        unique_ptr<Node> *end() override {
            return reinterpret_cast<unique_ptr<Node> *>(nodes + 2);
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return reinterpret_cast<const unique_ptr<Node> *>(nodes + 2);
        }

        static unique_ptr<IndexExpression> create(unique_ptr<Expression> expression, unique_ptr<Expression> index) {
            auto base = std::make_unique<IndexExpression>();
            base->nodes[EXPRESSION] = std::move(expression);
            base->nodes[INDEX] = std::move(index);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_IndexExpression;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Postfix::_get_name() + "::IndexExpression";
        }
    };

    class FunctionCall : public Postfix {
        enum {EXPRESSION, ARGUMENT_LIST};
        unique_ptr<Node> nodes[2];
        bool hasArgumentList = false;
    public:
        FunctionCall() : Postfix(NK_FunctionCall) {}

        [[nodiscard]] Expression *get_lhs() const {
            return cast<Expression>(nodes[EXPRESSION].get());
        }

        [[nodiscard]] bool has_argument_list() const {
            return hasArgumentList;
        }

        [[nodiscard]] ExpressionList *get_argument_list() const {
            return cast<ExpressionList>(nodes[ARGUMENT_LIST].get());
        }

        unique_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return nodes;
        }

        unique_ptr<Node> *end() override {
            return &nodes[2];
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return &nodes[2];
        }

        static unique_ptr<FunctionCall> create(unique_ptr<Node> expression) {
            return create(std::move(expression), nullptr);
        }

        static unique_ptr<FunctionCall> create(unique_ptr<Node> expression, unique_ptr<Node> argumentList) {
            auto base = std::make_unique<FunctionCall>();
            base->hasArgumentList = argumentList != nullptr;
            base->nodes[EXPRESSION] = std::move(expression);
            base->nodes[ARGUMENT_LIST] = std::move(argumentList);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_FunctionCall;
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
        unique_ptr<Node> expression;
        AssignmentType operation = INCREMENT;
    public:
        PostAssignment() : Postfix(NK_PostAssignment) {}

        [[nodiscard]] Expression *get_lhs() const {
            return cast<Expression>(expression.get());
        }

        unique_ptr<Node> *begin() override {
            return &expression;
        }
        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return &expression;
        }

        unique_ptr<Node> *end() override {
            return &expression + 1;
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return &expression + 1;
        }

        static unique_ptr<PostAssignment> create(unique_ptr<Node> expression, const AssignmentType operation) {
            auto base = std::make_unique<PostAssignment>();
            base->expression = std::move(expression);
            base->operation = operation;
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_PostAssignment;
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
        unique_ptr<Expression> expression;
        MemberAccessType operation = MEMBER;
        std::string identifier;
    public:
        MemberAccess() : Postfix(NK_MemberAccess) {}

        [[nodiscard]] Expression *get_lhs() const {
            return expression.get();
        }

        [[nodiscard]] const std::string &get_rhs() const {
            return identifier;
        }

        [[nodiscard]] MemberAccessType get_access_type() const {
            return this->operation;
        }

        unique_ptr<Node> *begin() override {
            return reinterpret_cast<unique_ptr<Node> *>(&expression);
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return reinterpret_cast<const unique_ptr<Node> *>(&expression);
        }

        unique_ptr<Node> *end() override {
            return reinterpret_cast<unique_ptr<Node> *>(&expression + 1);
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return reinterpret_cast<const unique_ptr<Node> *>(&expression + 1);
        }

        static unique_ptr<MemberAccess> create(unique_ptr<Expression> expression, const MemberAccessType operation, std::string identifier) {
            auto base = std::make_unique<MemberAccess>();
            base->expression = std::move(expression);
            base->operation = operation;
            base->identifier = std::move(identifier);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_MemberAccess;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Postfix::_get_name() + "::MemberAccess";
        }
    };

    class Identifier: public Expression {
        std::string identifier;
    public:
        Identifier() : Expression(NK_Identifier) {}
        [[nodiscard]] const std::string &get_value() const {
            return identifier;
        }

        static unique_ptr<Identifier> create(std::string identifier) {
            auto base = std::make_unique<Identifier>();
            base->identifier = std::move(identifier);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_Identifier;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Expression::_get_name() + "::Identifier";
        }
    };

    class Constant : public Expression {
        std::string constant;
    public:
        Constant() : Expression(NK_Constant) {}
        [[nodiscard]] const std::string &get_value() const {
            return constant;
        }

        static unique_ptr<Constant> create(std::string constant) {
            auto base = std::make_unique<Constant>();
            base->constant = std::move(constant);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_Constant;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Expression::_get_name() + "::Constant";
        }
    };

    class StringLiteral : public Expression {
        std::string literal;
    public:
        StringLiteral() : Expression(NK_StringLiteral) {}
        [[nodiscard]] const std::string &get_value() const {
            return literal;
        }

        static unique_ptr<StringLiteral> create(std::string literal) {
            auto base = std::make_unique<StringLiteral>();
            base->literal = std::move(literal);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_StringLiteral;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override{
            return this->Expression::_get_name() + "::StringLiteral";
        }
    };

    class SelectionStatement : public Statement {
        enum { CONDITION, IF_BODY, ELSE_BODY };

        unique_ptr<Node> nodes[3];
        bool hasElse = false;

    public:
        SelectionStatement() : Statement(NK_SelectionStatement) {}
        [[nodiscard]] Expression *get_condition() const {
            return cast<Expression>(nodes[CONDITION].get());
        }

        [[nodiscard]] CompoundStatement *get_then() const {
            return cast<CompoundStatement>(nodes[IF_BODY].get());
        }

        [[nodiscard]] bool has_else() const {
            return hasElse;
        }

        [[nodiscard]] CompoundStatement *get_else() const {
            if (!hasElse) return nullptr;
            return cast<CompoundStatement>(nodes[ELSE_BODY].get());
        }

        unique_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return nodes;
        }

        unique_ptr<Node> *end() override {
            if (hasElse) return &nodes[ELSE_BODY];
            return &nodes[IF_BODY];
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            if (hasElse) return &nodes[ELSE_BODY];
            return &nodes[IF_BODY];
        }

        static unique_ptr<SelectionStatement> create(unique_ptr<Node> condition, unique_ptr<Node> trueBody) {
            return create(std::move(condition), std::move(trueBody), nullptr);
        }

        static unique_ptr<SelectionStatement> create(unique_ptr<Node> condition,
                                                     unique_ptr<Node> trueBody, unique_ptr<Node> falseBody) {
            auto base = std::make_unique<SelectionStatement>();
            base->hasElse = falseBody != nullptr;
            base->nodes[CONDITION] = std::move(condition);
            base->nodes[IF_BODY] = std::move(trueBody);
            base->nodes[ELSE_BODY] = std::move(falseBody);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_SelectionStatement;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Statement::_get_name() + "::SelectionStatement";
        }
    };

    class WhileStatement : public Statement {
        enum { CONDITION, BODY };

        unique_ptr<Node> nodes[2];

    public:
        WhileStatement() : Statement(NK_WhileStatement) {}
        [[nodiscard]] Expression *get_condition() const {
            return cast<Expression>(nodes[CONDITION].get());
        }

        [[nodiscard]] CompoundStatement *get_body() const {
            return cast<CompoundStatement>(nodes[BODY].get());
        }

        unique_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return nodes;
        }

        unique_ptr<Node> *end() override {
            return &nodes[1];
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return &nodes[1];
        }

        static unique_ptr<WhileStatement> create(unique_ptr<Node> condition, unique_ptr<Node> body) {
            auto base = std::make_unique<WhileStatement>();
            base->nodes[0] = std::move(condition);
            base->nodes[1] = std::move(body);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_WhileStatement;
        }
    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Statement::_get_name() + "::WhileStatement";
        }
    };

    class DoStatement : public Statement {
        enum { CONDITION, BODY };

        unique_ptr<Node> nodes[2];

    public:
        DoStatement() : Statement(NK_DoStatement) {}
        [[nodiscard]] Expression *get_condition() const {
            return cast<Expression>(nodes[CONDITION].get());
        }

        [[nodiscard]] CompoundStatement *get_body() const {
            return cast<CompoundStatement>(nodes[BODY].get());
        }

        unique_ptr<Node> *begin() override {
            return nodes;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return nodes;
        }

        unique_ptr<Node> *end() override {
            return &nodes[1];
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return &nodes[1];
        }

        static unique_ptr<DoStatement> create(unique_ptr<Node> condition, unique_ptr<Node> body) {
            auto base = std::make_unique<DoStatement>();
            base->nodes[CONDITION] = std::move(condition);
            base->nodes[BODY] = std::move(body);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_DoStatement;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Statement::_get_name() + "::DoStatement";
        }
    };

    class ForStatement : public Statement {
        enum { INITIALIZATION, CONDITION, INCREMENT, BODY };

        unique_ptr<Node> nodes[4];
        bool hasInit = true;
        bool hasCondition = true;
        bool hasIncrement = true;

    public:
        ForStatement() : Statement(NK_ForStatement) {}
        [[nodiscard]] Node *get_initialization() const {
            if (!hasInit) return nullptr;
            return nodes[INITIALIZATION].get();
        }

        [[nodiscard]] Expression *get_condition() const {
            if (!hasCondition) return nullptr;
            return cast<Expression>(nodes[CONDITION].get());
        }

        [[nodiscard]] Expression *get_increment() const {
            if (!hasIncrement) return nullptr;
            return cast<Expression>(nodes[INCREMENT].get());
        }

        [[nodiscard]] CompoundStatement *get_body() const {
            return cast<CompoundStatement>(nodes[BODY].get());
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return nodes;
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return &nodes[BODY];
        }

        static unique_ptr<ForStatement> create(unique_ptr<Node> body) {
            return create(std::move(body), nullptr, nullptr, nullptr);
        }

        static unique_ptr<ForStatement> create(unique_ptr<Node> body, unique_ptr<Node> initialization,
                                               unique_ptr<Node> condition, unique_ptr<Node> increment) {
            auto base = std::make_unique<ForStatement>();
            base->hasInit = initialization != nullptr;
            base->hasCondition = condition != nullptr;
            base->hasIncrement = increment != nullptr;
            base->nodes[INITIALIZATION] = std::move(initialization);
            base->nodes[CONDITION] = std::move(condition);
            base->nodes[INCREMENT] = std::move(increment);
            base->nodes[BODY] = std::move(body);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_ForStatement;
        }

    protected:
        [[nodiscard]] std::string _get_name() const override {
            return this->Statement::_get_name() + "::ForStatement";
        }
    };

    class ControlStatement : public Statement {
        unique_ptr<Node> returnExpression = nullptr;

    public:
        ControlStatement() : Statement(NK_ControlStatement) {}
        enum State { BREAK, CONTINUE, RETURN };

        [[nodiscard]] bool is_break() const { return state == BREAK; }
        [[nodiscard]] bool is_continue() const { return state == CONTINUE; }
        [[nodiscard]] bool is_return() const { return state == RETURN; }

        [[nodiscard]] bool has_return_expr() const {return returnExpression != nullptr; }

        [[nodiscard]] Expression *get_return_expression() const {
            if (!is_return()) return nullptr;
            return cast<Expression>(returnExpression.get());
        }

        unique_ptr<Node> *begin() override {
            return &returnExpression;
        }

        [[nodiscard]] const unique_ptr<Node> *begin() const override {
            return &returnExpression;
        }

        unique_ptr<Node> *end() override {
            return &returnExpression + 1;
        }

        [[nodiscard]] const unique_ptr<Node> *end() const override {
            return &returnExpression + 1;
        }

        static unique_ptr<ControlStatement> create(const State state) {
            auto base = std::make_unique<ControlStatement>();
            base->state = state;
            return base;
        }

        static unique_ptr<ControlStatement> create(unique_ptr<Node> returnExpression) {
            auto base = std::make_unique<ControlStatement>();
            base->state = RETURN;
            base->returnExpression = std::move(returnExpression);
            return base;
        }

        static bool classof(const Node *node) {
            return node->getKind() == NK_ControlStatement;
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
