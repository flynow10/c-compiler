//
// Created by Natalie Wagner on 2/17/26.
//

#ifndef C_COMPILER_TOKEN_HPP
#define C_COMPILER_TOKEN_HPP
#include <map>
#include <utility>


enum class TokenType {
    Directive,
    Identifier,
    Constant,
    StringLiteral,
    FunctionDeclaration,
    For,
    While,
    Do,
    If,
    Else,
    Continue,
    Break,
    Return,
    Static,
    Void,
    Char,
    Short,
    Int,
    Long,
    Unsigned,
    Signed,
    Struct,
    Const,
    Sizeof,
    AddAssign,
    SubAssign,
    MultAssign,
    DivideAssign,
    LeftAssign,
    RightAssign,
    AndAssign,
    OrAssign,
    XorAssign,
    OrOp,
    AndOp,
    EqOp,
    NeOp,
    LeOp,
    GeOp,
    LeftOp,
    RightOp,
    IncOp,
    DecOp,
    PtrOp,
    LParen,
    RParen,
    LBracket,
    RBracket,
    LSquare,
    RSquare,
    Semicolon,
    Comma,
    Colon,
    Equal,
    Star,
    Pipe,
    Caret,
    Ampersand,
    LessThan,
    GreaterThan,
    Plus,
    Minus,
    Slash,
    Percent,
    Tilde,
    Bang,
    Period,
    EndOfFile,
};

struct Token {
    Token(std::string value, const TokenType type) : _value(std::move(value)), _type(type) {}
    [[nodiscard]] std::string value() const {
        return _value;
    }
    [[nodiscard]] TokenType type() const {
        return _type;
    }
private:
    TokenType _type;
    std::string _value;
};

inline std::ostream &operator<<(std::ostream &out, const TokenType value) {
    static const auto strings = [] {
        // prior to C++17, replace std::string_view with
        // std::string or const char*
        std::map<TokenType, std::string_view> result;
#define INSERT_ELEMENT(p) result.emplace(p, #p)
        INSERT_ELEMENT(TokenType::Directive);
        INSERT_ELEMENT(TokenType::Identifier);
        INSERT_ELEMENT(TokenType::Constant);
        INSERT_ELEMENT(TokenType::StringLiteral);
        INSERT_ELEMENT(TokenType::FunctionDeclaration);
        INSERT_ELEMENT(TokenType::For);
        INSERT_ELEMENT(TokenType::While);
        INSERT_ELEMENT(TokenType::Do);
        INSERT_ELEMENT(TokenType::If);
        INSERT_ELEMENT(TokenType::Else);
        INSERT_ELEMENT(TokenType::Continue);
        INSERT_ELEMENT(TokenType::Break);
        INSERT_ELEMENT(TokenType::Return);
        INSERT_ELEMENT(TokenType::Static);
        INSERT_ELEMENT(TokenType::Void);
        INSERT_ELEMENT(TokenType::Char);
        INSERT_ELEMENT(TokenType::Short);
        INSERT_ELEMENT(TokenType::Int);
        INSERT_ELEMENT(TokenType::Long);
        INSERT_ELEMENT(TokenType::Unsigned);
        INSERT_ELEMENT(TokenType::Signed);
        INSERT_ELEMENT(TokenType::Struct);
        INSERT_ELEMENT(TokenType::Const);
        INSERT_ELEMENT(TokenType::Sizeof);
        INSERT_ELEMENT(TokenType::AddAssign);
        INSERT_ELEMENT(TokenType::SubAssign);
        INSERT_ELEMENT(TokenType::MultAssign);
        INSERT_ELEMENT(TokenType::DivideAssign);
        INSERT_ELEMENT(TokenType::LeftAssign);
        INSERT_ELEMENT(TokenType::RightAssign);
        INSERT_ELEMENT(TokenType::AndAssign);
        INSERT_ELEMENT(TokenType::OrAssign);
        INSERT_ELEMENT(TokenType::XorAssign);
        INSERT_ELEMENT(TokenType::OrOp);
        INSERT_ELEMENT(TokenType::AndOp);
        INSERT_ELEMENT(TokenType::EqOp);
        INSERT_ELEMENT(TokenType::NeOp);
        INSERT_ELEMENT(TokenType::LeOp);
        INSERT_ELEMENT(TokenType::GeOp);
        INSERT_ELEMENT(TokenType::LeftOp);
        INSERT_ELEMENT(TokenType::RightOp);
        INSERT_ELEMENT(TokenType::IncOp);
        INSERT_ELEMENT(TokenType::DecOp);
        INSERT_ELEMENT(TokenType::PtrOp);
        INSERT_ELEMENT(TokenType::LParen);
        INSERT_ELEMENT(TokenType::RParen);
        INSERT_ELEMENT(TokenType::LBracket);
        INSERT_ELEMENT(TokenType::RBracket);
        INSERT_ELEMENT(TokenType::LSquare);
        INSERT_ELEMENT(TokenType::RSquare);
        INSERT_ELEMENT(TokenType::Semicolon);
        INSERT_ELEMENT(TokenType::Comma);
        INSERT_ELEMENT(TokenType::Colon);
        INSERT_ELEMENT(TokenType::Equal);
        INSERT_ELEMENT(TokenType::Star);
        INSERT_ELEMENT(TokenType::Pipe);
        INSERT_ELEMENT(TokenType::Caret);
        INSERT_ELEMENT(TokenType::Ampersand);
        INSERT_ELEMENT(TokenType::LessThan);
        INSERT_ELEMENT(TokenType::GreaterThan);
        INSERT_ELEMENT(TokenType::Plus);
        INSERT_ELEMENT(TokenType::Minus);
        INSERT_ELEMENT(TokenType::Slash);
        INSERT_ELEMENT(TokenType::Percent);
        INSERT_ELEMENT(TokenType::Tilde);
        INSERT_ELEMENT(TokenType::Bang);
        INSERT_ELEMENT(TokenType::Period);
        INSERT_ELEMENT(TokenType::EndOfFile);
#undef INSERT_ELEMENT
        return result;
    }();

    return out << strings.at(value);
}

#endif //C_COMPILER_TOKEN_HPP
