#include "lexer.hpp"
#include <istream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

const std::map<char, TokenType> StandardTokens = {
    {',', TokenType::Comma},
    {'(', TokenType::LParen},
    {')', TokenType::RParen},
    {'{', TokenType::LBracket},
    {'}', TokenType::RBracket},
    {'[', TokenType::LSquare},
    {']', TokenType::RSquare},
    {';', TokenType::Semicolon},
    {':', TokenType::Colon},
    {'=', TokenType::Equal},
    {'*', TokenType::Star},
    {'|', TokenType::Pipe},
    {'^', TokenType::Caret},
    {'&', TokenType::Ampersand},
    {'<', TokenType::LessThan},
    {'>', TokenType::GreaterThan},
    {'+', TokenType::Plus},
    {'-', TokenType::Minus},
    {'/', TokenType::Slash},
    {'%', TokenType::Percent},
    {'~', TokenType::Tilde},
    {'!', TokenType::Bang},
    {'.', TokenType::Period},
};

const std::map<std::string, TokenType> DoubleTokens = {
    {"+=", TokenType::AddAssign},
    {"-=", TokenType::SubAssign},
    {"*=", TokenType::MultAssign},
    {"/=", TokenType::DivideAssign},
    {"&=", TokenType::AndAssign},
    {"|=", TokenType::OrAssign},
    {"^=", TokenType::XorAssign},
    {"%=", TokenType::ModAssign},
    {"||", TokenType::OrOp},
    {"&&", TokenType::AndOp},
    {"==", TokenType::EqOp},
    {"!=", TokenType::NeOp},
    {"<=", TokenType::LeOp},
    {">=", TokenType::GeOp},
    {"<<", TokenType::LeftOp},
    {">>", TokenType::RightOp},
    {"++", TokenType::IncOp},
    {"--", TokenType::DecOp},
    {"->", TokenType::PtrOp},
};

const std::string IdentifierFirstChar(
    "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");

const std::string IdentifierChars = IdentifierFirstChar + "0123456789";

const std::map<std::string, TokenType> Keywords = {
    {"fn", TokenType::FunctionDeclaration},
    {"if", TokenType::If},
    {"else", TokenType::Else},
    {"while", TokenType::While},
    {"do", TokenType::Do},
    {"for", TokenType::For},
    {"break", TokenType::Break},
    {"continue", TokenType::Continue},
    {"return", TokenType::Return},
    {"static", TokenType::Static},
    {"void", TokenType::Void},
    {"char", TokenType::Char},
    {"short", TokenType::Short},
    {"int", TokenType::Int},
    {"long", TokenType::Long},
    {"unsigned", TokenType::Unsigned},
    {"signed", TokenType::Signed},
    {"struct", TokenType::Struct},
    {"const", TokenType::Const},
    {"sizeof", TokenType::Sizeof},
};

constexpr std::string NumberConstantChars = "xXuUlL";

Token createToken(const TokenType type, const std::string &value) {
    return {value, type};
}

std::string parseIdentifier(std::istream &ss) {
    std::string id;

    signed char currentChar;

    while (IdentifierChars.find(currentChar = ss.get()) != std::string::npos) {
        id += currentChar;
    }

    ss.seekg(-1, std::istream::cur);

    return id;
}

TokenType parseKeyword(const std::string &identifier) {
    auto tt = TokenType::Identifier;

    for (auto const &[matcher, type]: Keywords) {
        if (identifier == matcher) {
            tt = type;
            break;
        }
    }

    return tt;
}

std::string parseNumber(std::istream &ss) {
    std::string number;
    bool isHex = false;

    do {
        const char currentChar = ss.get();
        if (currentChar == 'x' || currentChar == 'X') {
            isHex = true;
        }
        number += currentChar;
    }
    while (std::isdigit(ss.peek()) || NumberConstantChars.find(ss.peek()) != std::string::npos || (isHex && std::string("abcdefABCDEF").find(ss.peek()) != std::string::npos));

    //ss.seekg(-1, std::istream::cur);

    return number;
}

bool parseDouble(const char currentChar, std::istream &ss,
                      std::vector<Token> &tokens) {
    std::string str(1, currentChar);
    str += static_cast<char>(ss.peek());
    for (auto const &[matcher, type]: DoubleTokens) {
        if (str == matcher) {
            tokens.push_back(createToken(type, str));
            ss.get();
            return true;
        }
    }

    return false;
}

std::string parseEscapedString(std::istream &ss, const char endChar) {
    std::string str;
    char currentChar;
    while ((currentChar = ss.get()) != endChar && currentChar != '\n') {
        if (currentChar == '\\') {
            str += ss.get();
        } else {
            str += currentChar;
        }
    }

    return str;
}

std::string parseDirective(std::istream &ss) {
    std::string str;
    std::getline(ss, str);

    return str;
}

Lexer::Lexer() : currentToken(0) {
}

void Lexer::tokenize(std::istream &code) {
    char currentChar;

    while ((currentChar = code.get()) != -1) {
        if (std::isspace(currentChar)) {
            continue;
        }

        // if (currentChar == '#') {
        //     tokens.push_back(createToken(TokenType::Directive, parseDirective(code)));
        //     continue;
        // }

        if (currentChar == '/' && code.peek() == '/') {
            while ((currentChar = code.get()) != -1 && currentChar != '\n') {}
            continue;
        }

        if (currentChar == '"') {
            tokens.push_back(createToken(TokenType::StringLiteral, parseEscapedString(code, '"')));
            continue;
        }

        if (currentChar == '\'') {
            tokens.push_back(createToken(TokenType::Constant, parseEscapedString(code, '\'')));
            continue;
        }

        if (currentChar == '<') {
            if (code.get() == '<' && code.peek() == '=') {
                tokens.push_back(createToken(TokenType::LeftAssign, "<<="));
                code.get();
                continue;
            }
            code.seekg(-1, std::istream::cur);
        }

        if (currentChar == '>') {
            if (code.get() == '>' && code.peek() == '=') {
                tokens.push_back(createToken(TokenType::RightAssign, ">>="));
                code.get();
                continue;
            }
            code.seekg(-1, std::istream::cur);
        }

        if (parseDouble(currentChar, code, tokens)) {
            continue;
        }

        if (std::isdigit(currentChar)) {
            code.seekg(-1, std::istream::cur);
            std::string num = parseNumber(code);
            tokens.push_back(createToken(TokenType::Constant, num));
            continue;
        }

        if (IdentifierFirstChar.find(currentChar) != std::string::npos) {
            code.seekg(-1, std::istream::cur);
            std::string id = parseIdentifier(code);
            const TokenType tt = parseKeyword(id);
            tokens.push_back(createToken(tt, id));
            continue;
        }

        for (auto const &[matcher, type]: StandardTokens) {
            if (currentChar == matcher) {
                std::string value(1, matcher);
                tokens.push_back(createToken(type, value));
                break;
            }
        }
    }

    tokens.push_back(createToken(TokenType::EndOfFile, std::string()));
}

Token Lexer::pop() {
    auto token = peek();
    currentToken++;
    return token;
}

void Lexer::save_cursor() {
    cursorStack.push(currentToken);
}

int Lexer::_get_cursor() const{
    return currentToken;
}

void Lexer::_set_cursor(const int cursor) {
    currentToken = cursor;
}

void Lexer::succeed() {
    cursorStack.pop();
}

void Lexer::back_track() {
    currentToken = cursorStack.top();
    cursorStack.pop();
}

Token Lexer::eat(const TokenType expectedType) {
    if (!has_token()) {
        throw std::runtime_error("Reached end of stream");
    }
    auto token = pop();
    if (token.type() != expectedType) {
        std::stringstream ss("Incorrect token type! Expected: ");

        ss << expectedType << " expected but received " << token.type();

        std::string error;
        getline(ss, error);
        throw std::runtime_error(error);
    }
    return token;
}

Token Lexer::peek() const { return peek(0); }

Token Lexer::peek(const int offset) const {
    return tokens.at(currentToken + offset);
}

bool Lexer::has_token() const { return currentToken < tokens.size(); }
