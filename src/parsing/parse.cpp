#include "parse.hpp"
#include "ast.hpp"
#include "../lexer.hpp"
#include <istream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

using std::make_shared;
using std::unique_ptr;

const std::vector SpecQualTypes = {
    TokenType::Static,
    TokenType::Void,
    TokenType::Char,
    TokenType::Short,
    TokenType::Int,
    TokenType::Long,
    TokenType::Unsigned,
    TokenType::Signed,
    TokenType::Struct,
    TokenType::Const,
};

Parser::Parser() = default;

unique_ptr<AST::Node> Parser::parse(std::istream &stream) {
    lexer.tokenize(stream);
    return parse_translation_unit();
}

unique_ptr<AST::TranslationUnit> Parser::parse_translation_unit() {
    std::vector<unique_ptr<AST::Node>> nodes;
    do {
        lexer.save_cursor();
        auto decl = parse_decl();
        if (decl == nullptr) {
            lexer.back_track();
            nodes.push_back(parse_function_decl());
        } else {
            lexer.succeed();
            nodes.push_back(std::move(decl));
        }
    } while (lexer.peek().type() != TokenType::EndOfFile);
    lexer.eat(TokenType::EndOfFile);

    return AST::TranslationUnit::create(nodes);
}

unique_ptr<AST::FunctionDecl> Parser::parse_function_decl() {
    auto specQuals = parse_decl_specifiers();
    auto declarator = parse_declarator();
    auto body = parse_compound_statement();
    return AST::FunctionDecl::create(std::move(specQuals), std::move(declarator), std::move(body));
}


unique_ptr<AST::Decl> Parser::parse_decl() {
    std::vector<unique_ptr<AST::Node> > initDeclarators;
    Token token = lexer.peek();

    auto specQuals = parse_decl_specifiers();

    if (lexer.peek().type() != TokenType::Semicolon) {
        initDeclarators.push_back(parse_init_declarator());
        while (lexer.peek().type() == TokenType::Comma) {
            lexer.eat(TokenType::Comma);
            initDeclarators.push_back(parse_init_declarator());
        }
    }
    if (lexer.peek().type() == TokenType::Semicolon) {
        lexer.eat(TokenType::Semicolon);
    } else {
        return nullptr;
    }

    return AST::Decl::create(std::move(specQuals), AST::InitDeclaratorList::create(initDeclarators));
}

unique_ptr<AST::DeclSpecifiers> Parser::parse_decl_specifiers(std::initializer_list<TokenType> excludedTypes) {
    std::vector<unique_ptr<AST::Node> > specQuals;
    Token token = lexer.peek();
    TokenType type = token.type();
    while (std::ranges::find(SpecQualTypes, type) != SpecQualTypes.end()
        && std::ranges::find(excludedTypes, type) == excludedTypes.end()) {
        if (type == TokenType::Const) {
            specQuals.push_back(parse_type_qualifier());
        } else if (type == TokenType::Static) {
            specQuals.push_back(parse_storage_class());
        } else {
            specQuals.push_back(parse_type_specifier());
        }
        token = lexer.peek();
        type = token.type();
    }

    return AST::DeclSpecifiers::create(specQuals);
}

unique_ptr<AST::StorageClass> Parser::parse_storage_class() {
    switch (lexer.peek().type()) {
        case TokenType::Static: {
            lexer.eat(TokenType::Static);
            return AST::StorageClass::create(AST::StorageClass::STATIC);
        }
        default: {
            throw std::runtime_error("Parsing error in storage_class");
        }
    }
}

std::unordered_map<TokenType, AST::TypeSpecifier::TypeSpecifierType> typeSpecifierMap = {
    {TokenType::Void, AST::TypeSpecifier::VOID},
    {TokenType::Char, AST::TypeSpecifier::CHAR},
    {TokenType::Short, AST::TypeSpecifier::SHORT},
    {TokenType::Int, AST::TypeSpecifier::INT},
    {TokenType::Long, AST::TypeSpecifier::LONG},
    {TokenType::Unsigned, AST::TypeSpecifier::UNSIGNED},
    {TokenType::Signed, AST::TypeSpecifier::SIGNED},
};

unique_ptr<AST::TypeSpecifier> Parser::parse_type_specifier() {
    switch (const Token token = lexer.peek(); token.type()) {
        case TokenType::Struct: {
            return parse_struct_specifier();
        }
        default: {
            for (const auto &[tokenType, typeSpecifierType]: typeSpecifierMap) {
                if (token.type() == tokenType) {
                    lexer.pop();
                    return AST::TypeSpecifier::create(typeSpecifierType);
                }
            }
            throw std::runtime_error("Parsing error in type_specifier");
        }
    }
}

unique_ptr<AST::StructSpecifier> Parser::parse_struct_specifier() {
    lexer.eat(TokenType::Struct);
    std::string structName;
    if (lexer.peek().type() == TokenType::Identifier) {
        const Token idToken = lexer.eat(TokenType::Identifier);
        structName = idToken.value();
    } else if (lexer.peek().type() != TokenType::LBracket) {
        throw std::runtime_error("Parsing error in struct_specifier");
    }

    if (lexer.peek().type() == TokenType::LBracket) {
        lexer.eat(TokenType::LBracket);
        auto structDeclaration = parse_struct_decl_list();
        lexer.eat(TokenType::RBracket);

        return AST::StructSpecifier::create(structName, std::move(structDeclaration));
    }

    return AST::StructSpecifier::create(structName);
}

unique_ptr<AST::StructDeclList> Parser::parse_struct_decl_list() {
    std::vector<unique_ptr<AST::Node> > structDeclarations;
    while (lexer.peek().type() != TokenType::RBracket) {
        structDeclarations.push_back(parse_struct_decl());
    }

    return AST::StructDeclList::create(structDeclarations);
}

unique_ptr<AST::StructDecl> Parser::parse_struct_decl() {
    unique_ptr<AST::Node> spec_quals = parse_decl_specifiers({TokenType::Static});
    unique_ptr<AST::Node> declarator = parse_declarator();
    lexer.eat(TokenType::Semicolon);

    return AST::StructDecl::create(std::move(spec_quals), std::move(declarator));
}

unique_ptr<AST::TypeQualifier> Parser::parse_type_qualifier() {
    lexer.eat(TokenType::Const);
    return AST::TypeQualifier::create();
}

/*
 * Unused ghost parser left for possible refactoring later to better ast structure
void Parser::parse_ghost_declarator() {
    while (lexer.peek().type() == TokenType::Star || lexer.peek().type() == TokenType::Const) {
        lexer.pop();
    }
    if (lexer.peek().type() == TokenType::Identifier) {
        lexer.pop();
    } else if (lexer.peek().type() == TokenType::LParen) {
        lexer.eat(TokenType::LParen);
        parse_ghost_declarator();
        lexer.eat(TokenType::RParen);
    }

    if (lexer.peek().type() == TokenType::LSquare) {
        lexer.eat(TokenType::LSquare);
        while (lexer.peek().type() != TokenType::RSquare) {
            lexer.pop();
        }
        lexer.eat(TokenType::RSquare);
    }

    if (lexer.peek().type() == TokenType::LParen) {
        lexer.eat(TokenType::LParen);
        while (lexer.peek().type() != TokenType::RParen) {
            lexer.pop();
        }
        lexer.eat(TokenType::RParen);
    }
}
*/

unique_ptr<AST::Declarator> Parser::parse_declarator(bool isAbstract) {
    unique_ptr<AST::Node> pointer = nullptr;
    if (lexer.peek().type() == TokenType::Star) {
        pointer = parse_pointer();
    }
    unique_ptr<AST::Node> directDeclarator = parse_direct_declarator(isAbstract);
    unique_ptr<AST::Node> suffix = nullptr;

    TokenType type = lexer.peek().type();
    while (type == TokenType::LSquare || type == TokenType::LParen) {
        if (type == TokenType::LSquare) {
            suffix = parse_index_declarator(std::move(suffix));
        } else {
            suffix = parse_parameterized_declarator(std::move(suffix));
        }
        type = lexer.peek().type();
    }

    return AST::Declarator::create(std::move(directDeclarator), std::move(pointer), std::move(suffix));
}


unique_ptr<AST::Pointer> Parser::parse_pointer() {
    lexer.eat(TokenType::Star);
    unique_ptr<AST::Pointer> head = nullptr;
    if (lexer.peek().type() == TokenType::Const) {
        lexer.eat(TokenType::Const);
        head = AST::Pointer::create(true);
    } else {
        head = AST::Pointer::create(false);
    }
    AST::Pointer* current = head.get();

    while (lexer.peek().type() == TokenType::Star) {
        lexer.eat(TokenType::Star);
        unique_ptr<AST::Pointer> next = AST::Pointer::create(lexer.peek().type() == TokenType::Const);
        if (lexer.peek().type() == TokenType::Const) {
            lexer.eat(TokenType::Const);
        }
        current->set_next_pointer(std::move(next));
        current = current->get_next_pointer();
    }

    return head;
}

unique_ptr<AST::Node> Parser::parse_direct_declarator(const bool isAbstract) {
    const auto token = lexer.peek();
    if (token.type() == TokenType::Identifier) {
        lexer.eat(TokenType::Identifier);
        return AST::DirectDeclarator::create(token.value());
    }

    if (token.type() == TokenType::LParen) {
        lexer.eat(TokenType::LParen);
        auto declarator = parse_declarator(isAbstract);
        lexer.eat(TokenType::RParen);
        return declarator;
    }

    if (isAbstract) {
        return nullptr;
    }

    throw std::runtime_error("Parsing error in direct declarator");
}

unique_ptr<AST::IndexDeclarator> Parser::parse_index_declarator(unique_ptr<AST::Node> prevSuffix) {
    unique_ptr<AST::Node> constantExpression = nullptr;
    lexer.eat(TokenType::LSquare);
    if (lexer.peek().type() != TokenType::RSquare) {
        constantExpression = parse_logical_or_expression();
    }
    lexer.eat(TokenType::RSquare);
    return AST::IndexDeclarator::create(std::move(constantExpression), std::move(prevSuffix));
}

unique_ptr<AST::ParameterizedDeclarator> Parser::parse_parameterized_declarator(unique_ptr<AST::Node> prevSuffix) {
    unique_ptr<AST::Node> parameterList = nullptr;
    lexer.eat(TokenType::LParen);
    if (lexer.peek().type() != TokenType::RParen) {
        parameterList = parse_parameter_list();
    }
    lexer.eat(TokenType::RParen);
    return AST::ParameterizedDeclarator::create(std::move(parameterList), std::move(prevSuffix));
}

unique_ptr<AST::ParameterList> Parser::parse_parameter_list() {
    std::vector<unique_ptr<AST::Node>> parameters;
    parameters.push_back(parse_parameter());

    while (lexer.peek().type() == TokenType::Comma) {
        lexer.pop();
        parameters.push_back(parse_parameter());
    }

    return AST::ParameterList::create(parameters);
}

unique_ptr<AST::Parameter> Parser::parse_parameter() {
    unique_ptr<AST::Node> specQual = parse_decl_specifiers();
    unique_ptr<AST::Node> declarator = nullptr;
    if (lexer.peek().type() != TokenType::Comma) {
        declarator = parse_declarator(true);
    }
    return AST::Parameter::create(std::move(specQual), std::move(declarator));
}

unique_ptr<AST::InitDeclarator> Parser::parse_init_declarator() {
    unique_ptr<AST::Node> declarator = parse_declarator();
    if (lexer.peek().type() == TokenType::Equal) {
        lexer.eat(TokenType::Equal);
        unique_ptr<AST::Node> initializer;
        if (lexer.peek().type() == TokenType::LBracket) {
            lexer.eat(TokenType::LBracket);
            initializer = parse_initializer_list();
            lexer.eat(TokenType::RBracket);
        } else {
            initializer = parse_assignment_expression();
        }
        return AST::InitDeclarator::create(std::move(declarator), std::move(initializer));
    }
    return AST::InitDeclarator::create(std::move(declarator));
}

unique_ptr<AST::TypeName> Parser::parse_type_name() {
    auto specifierQualifiers = parse_decl_specifiers({TokenType::Static});
    auto abstractDeclarator = parse_declarator(true);

    return AST::TypeName::create(std::move(specifierQualifiers), std::move(abstractDeclarator));
}

unique_ptr<AST::InitializerList> Parser::parse_initializer_list() {
    std::vector<unique_ptr<AST::Node> > initializers;
    while (lexer.peek().type() != TokenType::RBracket) {
        if (lexer.peek().type() == TokenType::LBracket) {
            lexer.eat(TokenType::LBracket);
            initializers.push_back(parse_initializer_list());
            lexer.eat(TokenType::RBracket);
        } else {
            initializers.push_back(parse_assignment_expression());
        }
        if (lexer.peek().type() == TokenType::Comma) {
            lexer.eat(TokenType::Comma);
        }
    }

    return AST::InitializerList::create(initializers);
}

unique_ptr<AST::CompoundStatement> Parser::parse_compound_statement() {
    lexer.eat(TokenType::LBracket);
    std::vector<unique_ptr<AST::Node> > nodes;
    while (lexer.peek().type() != TokenType::RBracket) {
        //lexer.save_cursor();
        // if (auto declaration = parse_decl(); declaration != nullptr) {
        if (std::ranges::find(SpecQualTypes, lexer.peek().type()) != SpecQualTypes.end()) {
            nodes.push_back(parse_decl());
            //lexer.succeed();
            continue;
        }
        //lexer.back_track();

        auto statement = parse_statement();
        if (statement == nullptr) {
            throw std::runtime_error("Parsing error in compound statement");
        }
        nodes.push_back(std::move(statement));
    }
    lexer.eat(TokenType::RBracket);
    return AST::CompoundStatement::create(nodes);
}

unique_ptr<AST::Statement> Parser::parse_statement() {
    switch (lexer.peek().type()) {
        case TokenType::If: {
            return parse_selection_statement();
        }
        case TokenType::While: {
            return parse_while_statement();
        }
        case TokenType::For: {
            return parse_for_statement();
        }
        case TokenType::Do: {
            return parse_do_statement();
        }
        case TokenType::Break:
        case TokenType::Continue:
        case TokenType::Return: {
            return parse_control_statement();
        }
        case TokenType::LBracket: {
            return parse_compound_statement();
        }
        default: {
            auto expression = parse_expression();
            lexer.eat(TokenType::Semicolon);
            return AST::ExpressionStatement::create(std::move(expression));
        }
    }
}

unique_ptr<AST::SelectionStatement> Parser::parse_selection_statement() {
    lexer.eat(TokenType::If);
    lexer.eat(TokenType::LParen);
    auto condition = parse_expression();
    lexer.eat(TokenType::RParen);
    auto thenBody = parse_compound_statement();
    if (lexer.peek().type() == TokenType::Else) {
        lexer.eat(TokenType::Else);
        auto elseBody = parse_compound_statement();
        return AST::SelectionStatement::create(std::move(condition), std::move(thenBody), std::move(elseBody));
    }
    return AST::SelectionStatement::create(std::move(condition), std::move(thenBody));
}

unique_ptr<AST::WhileStatement> Parser::parse_while_statement() {
    lexer.eat(TokenType::While);
    lexer.eat(TokenType::LParen);
    auto condition = parse_expression();
    lexer.eat(TokenType::RParen);
    auto body = parse_compound_statement();
    return AST::WhileStatement::create(std::move(condition), std::move(body));
}

unique_ptr<AST::DoStatement> Parser::parse_do_statement() {
    lexer.eat(TokenType::Do);
    auto body = parse_compound_statement();
    lexer.eat(TokenType::While);
    lexer.eat(TokenType::LParen);
    auto condition = parse_expression();
    lexer.eat(TokenType::RParen);
    lexer.eat(TokenType::Semicolon);
    return AST::DoStatement::create(std::move(condition), std::move(body));
}

unique_ptr<AST::ForStatement> Parser::parse_for_statement() {
    lexer.eat(TokenType::For);
    lexer.eat(TokenType::LParen);
    unique_ptr<AST::Node> initialization = nullptr, condition = nullptr, increment = nullptr;
    if (lexer.peek().type() != TokenType::Semicolon) {
        if (std::ranges::find(SpecQualTypes, lexer.peek().type()) == SpecQualTypes.end()) {
            initialization = parse_expression();
            lexer.eat(TokenType::Semicolon);
        } else {
            initialization = parse_decl();
        }
    } else {
        lexer.eat(TokenType::Semicolon);
    }
    if (lexer.peek().type() != TokenType::Semicolon) {
        condition = parse_expression();
    }
    lexer.eat(TokenType::Semicolon);
    if (lexer.peek().type() != TokenType::RParen) {
        increment = parse_expression();
    }
    lexer.eat(TokenType::RParen);
    auto body = parse_compound_statement();
    return AST::ForStatement::create(std::move(body), std::move(initialization), std::move(condition), std::move(increment));
}

unique_ptr<AST::ControlStatement> Parser::parse_control_statement() {
    switch (lexer.peek().type()) {
        case TokenType::Break: {
            lexer.eat(TokenType::Break);
            lexer.eat(TokenType::Semicolon);
            return AST::ControlStatement::create(AST::ControlStatement::BREAK);
        }
        case TokenType::Continue: {
            lexer.eat(TokenType::Continue);
            lexer.eat(TokenType::Semicolon);
            return AST::ControlStatement::create(AST::ControlStatement::CONTINUE);
        }
        case TokenType::Return: {
            lexer.eat(TokenType::Return);
            if (lexer.peek().type() != TokenType::Semicolon) {
                auto expression = parse_expression();
                lexer.eat(TokenType::Semicolon);
                return AST::ControlStatement::create(std::move(expression));
            }
            lexer.eat(TokenType::Semicolon);
            return AST::ControlStatement::create(AST::ControlStatement::RETURN);
        }
        default: {
            throw std::runtime_error("Parsing failed in control statement");
        }
    }
}

unique_ptr<AST::Expression> Parser::parse_expression() {
    return parse_expression_list(false);
}

unique_ptr<AST::Expression> Parser::parse_expression_list(bool requireSurround) {
    std::vector<unique_ptr<AST::Expression>> expressions;
    expressions.push_back(parse_assignment_expression());
    while (lexer.peek().type() == TokenType::Comma) {
        lexer.eat(TokenType::Comma);
        expressions.push_back(parse_assignment_expression());
    }
    if (!requireSurround && expressions.size() == 1) {
        return std::move(expressions[0]);
    }
    return AST::ExpressionList::create(expressions);
}

const std::unordered_map<TokenType, AST::Assignment::Op> AssignmentOperatorMap = {
    {TokenType::Equal, AST::Assignment::Op::EQUAL},
    {TokenType::AddAssign, AST::Assignment::Op::ADD},
    {TokenType::SubAssign, AST::Assignment::Op::SUB},
    {TokenType::MultAssign, AST::Assignment::Op::MUL},
    {TokenType::DivideAssign, AST::Assignment::Op::DIV},
    {TokenType::LeftAssign, AST::Assignment::Op::LEFT},
    {TokenType::RightAssign, AST::Assignment::Op::RIGHT},
    {TokenType::AndAssign, AST::Assignment::Op::AND},
    {TokenType::OrAssign, AST::Assignment::Op::OR},
    {TokenType::XorAssign, AST::Assignment::Op::XOR},
    {TokenType::ModAssign, AST::Assignment::Op::MOD},
};

unique_ptr<AST::Expression> Parser::parse_assignment_expression() {
    unique_ptr<AST::Expression> lhs = parse_logical_or_expression();
    if (AssignmentOperatorMap.contains(lexer.peek().type())) {
        const AST::Assignment::Op op = AssignmentOperatorMap.at(lexer.pop().type());
        unique_ptr<AST::Expression> rhs = parse_assignment_expression();
        return AST::Assignment::create(std::move(lhs), std::move(rhs), op);
    }
    return lhs;
}

unique_ptr<AST::Expression> Parser::parse_logical_or_expression() {
    unique_ptr<AST::Expression> lhs = parse_logical_and_expression();
    if (lexer.peek().type() == TokenType::OrOp) {
        lexer.pop();
        auto rhs = parse_logical_or_expression();
        return AST::BinOp::create(std::move(lhs), std::move(rhs), AST::BinOp::LOGIC_OR);
    }
    return lhs;
}

unique_ptr<AST::Expression> Parser::parse_logical_and_expression() {
    unique_ptr<AST::Expression> lhs = parse_inclusive_or_expression();
    if (lexer.peek().type() == TokenType::AndOp) {
        lexer.pop();
        auto rhs = parse_logical_and_expression();
        return AST::BinOp::create(std::move(lhs), std::move(rhs), AST::BinOp::LOGIC_AND);
    }
    return lhs;
}

unique_ptr<AST::Expression> Parser::parse_inclusive_or_expression() {
    unique_ptr<AST::Expression> lhs = parse_exclusive_or_expression();
    if (lexer.peek().type() == TokenType::Pipe) {
        lexer.pop();
        auto rhs = parse_inclusive_or_expression();
        return AST::BinOp::create(std::move(lhs), std::move(rhs), AST::BinOp::INCLUSIVE_OR);
    }
    return lhs;
}

unique_ptr<AST::Expression> Parser::parse_exclusive_or_expression() {
    unique_ptr<AST::Expression> lhs = parse_and_expression();
    if (lexer.peek().type() == TokenType::Caret) {
        lexer.pop();
        auto rhs = parse_exclusive_or_expression();
        return AST::BinOp::create(std::move(lhs), std::move(rhs), AST::BinOp::EXCLUSIVE_OR);
    }
    return lhs;
}

unique_ptr<AST::Expression> Parser::parse_and_expression() {
    unique_ptr<AST::Expression> lhs = parse_equality_expression();
    if (lexer.peek().type() == TokenType::Ampersand) {
        lexer.pop();
        auto rhs = parse_and_expression();
        return AST::BinOp::create(std::move(lhs), std::move(rhs), AST::BinOp::AND);
    }
    return lhs;
}

unique_ptr<AST::Expression> Parser::parse_equality_expression() {
    unique_ptr<AST::Expression> lhs = parse_relation_expression();
    const TokenType type = lexer.peek().type();
    if (type == TokenType::EqOp || type == TokenType::NeOp) {
        lexer.pop();
        auto rhs = parse_equality_expression();
        return AST::BinOp::create(std::move(lhs), std::move(rhs), type == TokenType::EqOp ? AST::BinOp::EQUAL : AST::BinOp::NOT_EQUAL);
    }
    return lhs;
}

const std::unordered_map<TokenType, AST::BinOp::Op> RelationExpressionMap = {
    {TokenType::LessThan, AST::BinOp::Op::LESS_THAN},
    {TokenType::GreaterThan, AST::BinOp::Op::GREATER_THAN},
    {TokenType::LeOp, AST::BinOp::Op::LESS_EQUAL},
    {TokenType::GeOp, AST::BinOp::Op::GREATER_EQUAL},
};

unique_ptr<AST::Expression> Parser::parse_relation_expression() {
    unique_ptr<AST::Expression> lhs = parse_shift_expression();
    const TokenType type = lexer.peek().type();
    if (RelationExpressionMap.contains(type)) {
        lexer.pop();
        auto rhs = parse_relation_expression();
        return AST::BinOp::create(std::move(lhs), std::move(rhs), RelationExpressionMap.at(type));
    }
    return lhs;
}

unique_ptr<AST::Expression> Parser::parse_shift_expression() {
    unique_ptr<AST::Expression> lhs = parse_additive_expression();
    const TokenType type = lexer.peek().type();
    if (type == TokenType::LeftOp || type == TokenType::RightOp) {
        lexer.pop();
        auto rhs = parse_shift_expression();
        return AST::BinOp::create(std::move(lhs), std::move(rhs), type == TokenType::LeftOp ? AST::BinOp::LEFT_SHIFT : AST::BinOp::RIGHT_SHIFT);
    }
    return lhs;
}

unique_ptr<AST::Expression> Parser::parse_additive_expression() {
    unique_ptr<AST::Expression> lhs = parse_multiplicative_expression();
    const TokenType type = lexer.peek().type();
    if (type == TokenType::Plus || type == TokenType::Minus) {
        lexer.pop();
        auto rhs = parse_additive_expression();
        return AST::BinOp::create(std::move(lhs), std::move(rhs), type == TokenType::Plus ? AST::BinOp::ADD : AST::BinOp::SUB);
    }
    return lhs;
}

const std::unordered_map<TokenType, AST::BinOp::Op> MultiplicativeOperatorMap = {
    {TokenType::Star, AST::BinOp::Op::MUL},
    {TokenType::Slash, AST::BinOp::Op::DIV},
    {TokenType::Percent, AST::BinOp::Op::MOD},
};
unique_ptr<AST::Expression> Parser::parse_multiplicative_expression() {
    unique_ptr<AST::Expression> lhs = parse_cast_expression();
    const TokenType type = lexer.peek().type();
    if (MultiplicativeOperatorMap.contains(type)) {
        lexer.pop();
        auto rhs = parse_multiplicative_expression();
        return AST::BinOp::create(std::move(lhs), std::move(rhs), MultiplicativeOperatorMap.at(type));
    }
    return lhs;
}

unique_ptr<AST::Expression> Parser::parse_cast_expression() {
    unique_ptr<AST::Node> typeName = nullptr;
    if (lexer.peek().type() == TokenType::LParen && std::ranges::find(SpecQualTypes, lexer.peek(1).type()) != SpecQualTypes.end()) {
        lexer.pop();
        typeName = parse_type_name();
        lexer.eat(TokenType::RParen);
    }
    unique_ptr<AST::Expression> expr = parse_unary_expression();
    if (typeName != nullptr) {
        return AST::Cast::create(std::move(typeName), std::move(expr));
    }
    return expr;
}

const std::unordered_map<TokenType, AST::UnaryOp::Op> UnaryOperatorMap = {
    {TokenType::Ampersand, AST::UnaryOp::ADDRESS_OF},
    {TokenType::Star, AST::UnaryOp::DEREFERENCE},
    {TokenType::Plus, AST::UnaryOp::POSITIVE},
    {TokenType::Minus, AST::UnaryOp::NEGATIVE},
    {TokenType::Tilde, AST::UnaryOp::NEGATE},
    {TokenType::Bang, AST::UnaryOp::INVERT},
};

unique_ptr<AST::Expression> Parser::parse_unary_expression() {
    const TokenType type = lexer.peek().type();
    switch (type) {
        case TokenType::IncOp:
        case TokenType::DecOp: {
            lexer.pop();
            auto rhs = parse_unary_expression();
            return AST::UnaryOp::create(std::move(rhs), type == TokenType::IncOp ? AST::UnaryOp::INCREMENT : AST::UnaryOp::DECREMENT);
        }
        case TokenType::Sizeof: {
            lexer.pop();
            if (lexer.peek().type() == TokenType::LParen) {
                lexer.pop();
                auto typeName = parse_type_name();
                lexer.eat(TokenType::RParen);
                return AST::SizeofType::create(std::move(typeName));
            }
            auto rhs = parse_unary_expression();
            return AST::UnaryOp::create(std::move(rhs), AST::UnaryOp::SIZEOF);
        }
        default: {
            break;
        }
    }

    if (UnaryOperatorMap.contains(type)) {
        lexer.pop();
        auto rhs = parse_cast_expression();
        return AST::UnaryOp::create(std::move(rhs), UnaryOperatorMap.at(type));
    }

    return parse_postfix_expression();
}

constexpr TokenType PostfixTokens[] = {
    TokenType::LSquare,
    TokenType::LParen,
    TokenType::PtrOp,
    TokenType::Period,
    TokenType::IncOp,
    TokenType::DecOp,
};

unique_ptr<AST::Expression> Parser::parse_postfix_expression() {
    auto primaryExpression = parse_primary_expression();
    TokenType type = lexer.peek().type();
    while (std::ranges::find(PostfixTokens, type) != std::end(PostfixTokens)) {
        if (type == TokenType::LSquare) {
            lexer.pop();
            auto index = parse_expression();
            lexer.eat(TokenType::RSquare);
            primaryExpression = AST::IndexExpression::create(std::move(primaryExpression), std::move(index));
        } else if (type == TokenType::LParen) {
            lexer.pop();
            if (lexer.peek().type() == TokenType::RParen) {
                lexer.pop();
                primaryExpression = AST::FunctionCall::create(std::move(primaryExpression));
            } else {
                auto argumentList = parse_expression_list(true);
                lexer.eat(TokenType::RParen);
                primaryExpression = AST::FunctionCall::create(std::move(primaryExpression), std::move(argumentList));
            }
        } else if (type == TokenType::IncOp || type == TokenType::DecOp) {
            lexer.pop();
            primaryExpression = AST::PostAssignment::create(std::move(primaryExpression), type == TokenType::IncOp ? AST::PostAssignment::AssignmentType::INCREMENT : AST::PostAssignment::AssignmentType::DECREMENT);
        } else {
            lexer.pop();
            const Token identifier = lexer.eat(TokenType::Identifier);
            primaryExpression = AST::MemberAccess::create(std::move(primaryExpression), type == TokenType::PtrOp ? AST::MemberAccess::MemberAccessType::POINTER : AST::MemberAccess::MemberAccessType::MEMBER, identifier.value());
        }
        type = lexer.peek().type();
    }

    return primaryExpression;
}

unique_ptr<AST::Expression> Parser::parse_primary_expression() {
    switch (lexer.peek().type()) {
        case TokenType::Identifier: {
            return AST::Identifier::create(lexer.eat(TokenType::Identifier).value());
        }
        case TokenType::Constant: {
            return AST::Constant::create(lexer.eat(TokenType::Constant).value());
        }
        case TokenType::StringLiteral: {
            return AST::StringLiteral::create(lexer.eat(TokenType::StringLiteral).value());
        }
        case TokenType::LParen: {
            lexer.pop();
            auto expr = parse_expression();
            lexer.eat(TokenType::RParen);
            return expr;
        }
        default: {
            throw std::runtime_error("Parsing error in primary expression");
        }
    }
}
