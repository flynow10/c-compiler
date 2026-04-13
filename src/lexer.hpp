#ifndef LEXER_H
#define LEXER_H

#include <istream>
#include <vector>

#include "token.hpp"

class Lexer {
public:
  Lexer();
  void tokenize(std::istream &code);
  Token eat(TokenType expectedType);
  Token pop();
  void save_cursor();
  void succeed();
  void back_track();
  void store_type_name(const std::string &name);
  [[nodiscard]] Token peek() const;
  [[nodiscard]] Token peek(int offset) const;
  [[nodiscard]] bool has_token() const;

private:
  int currentToken;
  std::vector<Token> tokens;
  std::vector<std::string> typeNames;
  std::stack<int> cursorStack;
};

#endif // !LEXER_H
