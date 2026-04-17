#include "parse.hpp"
#include "ast.hpp"
#include "lexer.hpp"
#include <istream>
#include <memory>
#include <stdexcept>
#include <vector>

using std::make_shared;
using std::shared_ptr;

Parser::Parser() = default;

shared_ptr<AST::Node> Parser::parse(std::istream &stream) {
    lexer.tokenize(stream);
    return parse_translation_unit();
}

shared_ptr<AST::TranslationUnit> Parser::parse_translation_unit() {
    std::vector<shared_ptr<AST::Node> > nodes;
    do {
        if (lexer.peek().type() == TokenType::FunctionDeclaration) {
            nodes.push_back(parse_function_decl());
        } else {
            nodes.push_back(parse_decl());
        }
    } while (lexer.peek().type() != TokenType::EndOfFile);
    lexer.eat(TokenType::EndOfFile);

    return AST::TranslationUnit::create(nodes);
}

shared_ptr<AST::FunctionDecl> Parser::parse_function_decl() {
}

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

shared_ptr<AST::Decl> Parser::parse_decl() {
    std::vector<shared_ptr<AST::Node> > specQuals;
    std::vector<shared_ptr<AST::Node> > initDeclarators;
    Token token = lexer.peek();
    while (std::ranges::find(SpecQualTypes, token.type()) != SpecQualTypes.end()) {
        if (token.type() == TokenType::Const) {
            specQuals.push_back(parse_type_qualifier());
        } else if (token.type() == TokenType::Static) {
            specQuals.push_back(parse_storage_class());
        } else {
            specQuals.push_back(parse_type_specifier());
        }
        token = lexer.peek();
    }

    if (specQuals.empty()) return nullptr;

    if (lexer.peek().type() != TokenType::Semicolon) {
        initDeclarators.push_back(parse_init_declarator());
        while (lexer.peek().type() == TokenType::Comma) {
            lexer.eat(TokenType::Comma);
            initDeclarators.push_back(parse_init_declarator());
        }
    }

    return AST::Decl::create(specQuals, initDeclarators);
}

shared_ptr<AST::StorageClass> Parser::parse_storage_class() {
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

shared_ptr<AST::TypeSpecifier> Parser::parse_type_specifier() {
    switch (const Token token = lexer.peek(); token.type()) {
        case TokenType::Struct: {
            return parse_struct_specifier();
        }
        default: {
            for (const auto &[tokenType, typeSpecifierType]: typeSpecifierMap) {
                if (token.type() == tokenType) {
                    return AST::TypeSpecifier::create(typeSpecifierType);
                }
            }
            throw std::runtime_error("Parsing error in type_specifier");
        }
    }
}

shared_ptr<AST::StructSpecifier> Parser::parse_struct_specifier() {
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
        const auto structDeclaration = parse_struct_decl_list();
        lexer.eat(TokenType::RBracket);

        return AST::StructSpecifier::create(structName, structDeclaration);
    }

    return AST::StructSpecifier::create(structName);
}

shared_ptr<AST::StructDeclList> Parser::parse_struct_decl_list() {
    std::vector<shared_ptr<AST::Node> > structDeclarations;
    while (lexer.peek().type() != TokenType::RBracket) {
        structDeclarations.push_back(parse_struct_decl());
    }

    return AST::StructDeclList::create(structDeclarations);
}

shared_ptr<AST::StructDecl> Parser::parse_struct_decl() {
    std::vector<shared_ptr<AST::Node> > specifierQualifiers;
    Token token = lexer.peek();
    while (std::ranges::find(SpecQualTypes, token.type()) != SpecQualTypes.end() && token.type() != TokenType::Static) {
        if (token.type() == TokenType::Const) {
            specifierQualifiers.push_back(parse_type_qualifier());
        } else {
            specifierQualifiers.push_back(parse_type_specifier());
        }
        token = lexer.peek();
    }
    const shared_ptr<AST::Node> declarator = parse_declarator();
    lexer.eat(TokenType::Semicolon);

    return AST::StructDecl::create(specifierQualifiers, declarator);
}

shared_ptr<AST::TypeQualifier> Parser::parse_type_qualifier() {
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

shared_ptr<AST::Declarator> Parser::parse_declarator() {
    shared_ptr<AST::Node> pointer = nullptr;
    if (lexer.peek().type() == TokenType::Star) {
        pointer = parse_pointer();
    }

    const shared_ptr<AST::Node> directDeclarator = parse_direct_declarator();
    shared_ptr<AST::Node> suffix = nullptr;

    if (lexer.peek().type() == TokenType::LSquare) {
        suffix = parse_index_declarator();
    } else if (lexer.peek().type() == TokenType::LParen) {
        suffix = parse_parameterized_declarator();
    }

    return AST::Declarator::create(directDeclarator, pointer, suffix);
}


shared_ptr<AST::Pointer> Parser::parse_pointer() {
    lexer.eat(TokenType::Star);
    shared_ptr<AST::Pointer> head = nullptr;
    if (lexer.peek().type() == TokenType::Const) {
        lexer.eat(TokenType::Const);
        head = AST::Pointer::create(true);
    } else {
        head = AST::Pointer::create(false);
    }
    shared_ptr<AST::Pointer> current = head;

    while (lexer.peek().type() == TokenType::Star) {
        lexer.eat(TokenType::Star);
        const shared_ptr<AST::Pointer> next = AST::Pointer::create(lexer.peek().type() == TokenType::Const);
        if (lexer.peek().type() == TokenType::Const) {
            lexer.eat(TokenType::Const);
        }
        current->set_next_pointer(next);
        current = next;
    }

    return head;
}

shared_ptr<AST::Node> Parser::parse_direct_declarator() {
    const auto token = lexer.peek();
    if (token.type() == TokenType::Identifier) {
        lexer.eat(TokenType::Identifier);
        return AST::DirectDeclarator::create(token.value());
    }

    if (token.type() == TokenType::LParen) {
        lexer.eat(TokenType::LParen);
        const auto declarator = parse_declarator();
        lexer.eat(TokenType::RBracket);
        return declarator;
    }

    throw std::runtime_error("Parsing error in direct declarator");
}

shared_ptr<AST::IndexDeclarator> Parser::parse_index_declarator() {
    shared_ptr<AST::Node> constantExpression = nullptr;
    lexer.eat(TokenType::LSquare);
    if (lexer.peek().type() != TokenType::RSquare) {
        constantExpression = parse_logical_or_expression();
    }
    lexer.eat(TokenType::RSquare);
    return AST::IndexDeclarator::create(constantExpression);
}

shared_ptr<AST::ParameterizedDeclarator> Parser::parse_parameterized_declarator() {
    shared_ptr<AST::Node> parameterList = nullptr;
    lexer.eat(TokenType::LParen);
    if (lexer.peek().type() != TokenType::RParen) {
        // TODO: Implement parameter lists
        //parameterList = parse_parameter_list();
    }
    lexer.eat(TokenType::RParen);
    return AST::ParameterizedDeclarator::create(parameterList);
}

shared_ptr<AST::InitDeclarator> Parser::parse_init_declarator() {
    const shared_ptr<AST::Node> declarator = parse_declarator();
    if (lexer.peek().type() == TokenType::Equal) {
        lexer.eat(TokenType::Equal);
        shared_ptr<AST::Node> initializer;
        if (lexer.peek().type() == TokenType::LBracket) {
            lexer.eat(TokenType::LBracket);
            initializer = parse_initializer_list();
            lexer.eat(TokenType::RBracket);
        } else {
            initializer = parse_assignment_expression();
        }
        return AST::InitDeclarator::create(declarator, initializer);
    }
    return AST::InitDeclarator::create(declarator);
}

shared_ptr<AST::InitializerList> Parser::parse_initializer_list() {
    std::vector<shared_ptr<AST::Node> > initializers;
    while (lexer.peek().type() != TokenType::RBracket) {
        if (lexer.peek().type() == TokenType::LBracket) {
            lexer.eat(TokenType::LBracket);
            initializers.push_back(parse_initializer_list());
            lexer.eat(TokenType::RBracket);
        } else {
            initializers.push_back(parse_assignment_expression());
        }
    }

    return AST::InitializerList::create(initializers);
}

shared_ptr<AST::CompoundStatement> Parser::parse_compound_statement() {
    lexer.eat(TokenType::LBracket);
    std::vector<shared_ptr<AST::Node> > nodes;
    while (lexer.peek().type() != TokenType::RBracket) {
        lexer.save_cursor();
        if (auto declaration = parse_decl(); declaration != nullptr) {
            nodes.push_back(declaration);
            lexer.succeed();
            continue;
        }
        lexer.back_track();

        auto statement = parse_statement();
        if (statement == nullptr) {
            throw std::runtime_error("Parsing error in compound statement");
        }
        nodes.push_back(statement);
    }
    lexer.eat(TokenType::RBracket);
    return AST::CompoundStatement::create(nodes);
}

shared_ptr<AST::Statement> Parser::parse_statement() {
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
            return expression;
        }
    }
}

shared_ptr<AST::SelectionStatement> Parser::parse_selection_statement() {
    lexer.eat(TokenType::If);
    lexer.eat(TokenType::LParen);
    const auto condition = parse_expression();
    lexer.eat(TokenType::RParen);
    const auto thenBody = parse_compound_statement();
    if (lexer.peek().type() == TokenType::Else) {
        lexer.eat(TokenType::Else);
        const auto elseBody = parse_compound_statement();
        return AST::SelectionStatement::create(condition, thenBody, elseBody);
    }
    return AST::SelectionStatement::create(condition, thenBody);
}

shared_ptr<AST::WhileStatement> Parser::parse_while_statement() {
    lexer.eat(TokenType::While);
    lexer.eat(TokenType::LParen);
    const auto condition = parse_expression();
    lexer.eat(TokenType::RParen);
    const auto body = parse_compound_statement();
    return AST::WhileStatement::create(condition, body);
}

shared_ptr<AST::DoStatement> Parser::parse_do_statement() {
    lexer.eat(TokenType::Do);
    auto body = parse_compound_statement();
    lexer.eat(TokenType::While);
    lexer.eat(TokenType::LParen);
    const auto condition = parse_expression();
    lexer.eat(TokenType::RParen);
    lexer.eat(TokenType::Semicolon);
    return AST::DoStatement::create(condition, body);
}

shared_ptr<AST::ForStatement> Parser::parse_for_statement() {
    lexer.eat(TokenType::For);
    lexer.eat(TokenType::LParen);
    shared_ptr<AST::Expression> initialization = nullptr, condition = nullptr, increment = nullptr;
    if (lexer.peek().type() != TokenType::Semicolon) {
        initialization = parse_expression();
    }
    lexer.eat(TokenType::Semicolon);
    if (lexer.peek().type() != TokenType::Semicolon) {
        condition = parse_expression();
    }
    lexer.eat(TokenType::Semicolon);
    if (lexer.peek().type() != TokenType::RParen) {
        increment = parse_expression();
    }
    lexer.eat(TokenType::RParen);
    const auto body = parse_compound_statement();
    return AST::ForStatement::create(body, initialization, condition, increment);
}

shared_ptr<AST::ControlStatement> Parser::parse_control_statement() {
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
                return AST::ControlStatement::create(parse_expression());
            }
            lexer.eat(TokenType::Semicolon);
            return AST::ControlStatement::create(AST::ControlStatement::RETURN);
        }
        default: {
            throw std::runtime_error("Parsing failed in control statement");
        }
    }
}

shared_ptr<AST::Expression> Parser::parse_expression() {
    return parse_expression_list();
}

shared_ptr<AST::Expression> Parser::parse_expression_list() {
    std::vector<shared_ptr<AST::Expression>> expressions;
    expressions.push_back(parse_assignment_expression());
    while (lexer.peek().type() == TokenType::Comma) {
        lexer.eat(TokenType::Comma);
        expressions.push_back(parse_assignment_expression());
    }
    if (expressions.size() == 1) {
        return expressions[0];
    }
    return AST::ExpressionList::create(expressions);
}

shared_ptr<AST::Expression> Parser::parse_assignment_expression() {

}
