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

  [[nodiscard]] int _get_cursor() const;

  /**
   * @warning Only use in conjunction with _get_cursor
   */
  void _set_cursor(int cursor);

  void succeed();
  void back_track();
  [[nodiscard]] Token peek() const;
  [[nodiscard]] Token peek(int offset) const;
  [[nodiscard]] bool has_token() const;

private:
  int currentToken;
  std::vector<Token> tokens;
  std::stack<int> cursorStack;
};

#endif // !LEXER_H
