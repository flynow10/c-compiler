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
    std::vector<shared_ptr<AST::Node>> nodes;
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
    TokenType::Typedef,
    TokenType::Static,
    TokenType::Void,
    TokenType::Char,
    TokenType::Short,
    TokenType::Int,
    TokenType::Long,
    TokenType::Unsigned,
    TokenType::Signed,
    TokenType::Struct,
    TokenType::Enum,
    TokenType::Typename,
    TokenType::Const,
};

shared_ptr<AST::Decl> Parser::parse_decl() {
    std::vector<shared_ptr<AST::Node>> specQuals;
    std::vector<shared_ptr<AST::Node>> initDeclarators;
    Token token = lexer.peek();
    while (std::ranges::find(SpecQualTypes, token.type()) != SpecQualTypes.end()) {
        if (token.type() == TokenType::Const) {
            specQuals.push_back(parse_type_qualifier());
        } else if (token.type() == TokenType::Typedef || token.type() == TokenType::Static) {
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

    if (is_type_def) {
        for (const auto & init_declarator: initDeclarators) {
            // TODO: Handle storing type name
        }
    }

    is_type_def = false;

    return AST::Decl::create(specQuals, initDeclarators);
}

shared_ptr<AST::StorageClass> Parser::parse_storage_class() {
    switch (lexer.peek().type()) {
        case TokenType::Typedef: {
            lexer.eat(TokenType::Typedef);
            is_type_def = true;
            return AST::StorageClass::create(AST::StorageClass::TYPEDEF);
        }
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
        case TokenType::Enum: {
            return parse_enum_specifier();
        }
        case TokenType::Typename: {
            lexer.eat(TokenType::Typename);
        }
        default: {
            for (const auto & typeMapping: typeSpecifierMap) {
                if (token.type() == typeMapping.first) {
                    return AST::TypeSpecifier::create(typeMapping.second);
                }
            }
            throw std::runtime_error("Parsing error in type_specifier");
        }
    }
}

shared_ptr<AST::StructSpecifier> Parser::parse_struct_specifier() {

}

shared_ptr<AST::EnumSpecifier> Parser::parse_enum_specifier() {

}

shared_ptr<AST::TypeQualifier> Parser::parse_type_qualifier() {
    return AST::TypeQualifier::create();
}

shared_ptr<AST::InitDeclarator> Parser::parse_init_declarator() {

}

shared_ptr<AST::Declarator> Parser::parse_declarator() {
    
}

shared_ptr<AST::CompoundStatement> Parser::parse_compound_statement() {
    lexer.eat(TokenType::LBracket);
    std::vector<shared_ptr<AST::Node>> nodes;
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
}

