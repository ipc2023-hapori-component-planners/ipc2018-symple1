#ifndef PARSER_H
#define PARSER_H

#include "logical_expression.h"

#include <limits>
#include <memory>
#include <string>

struct Token {
  enum Type { OP, CONST, LPAREN, RPAREN, VAR, END, INVALID };

  Token(Type _type = INVALID, std::string val = "")
      : type(_type), value(val) {}

  Type type;
  // Integer value for constants, name for variables, operator name for
  // operators, empty string otherwise
  std::string value;
};

// Generates tokens from a given string
class Lexer {

public:
  Lexer(std::string const& s)
      : input(s), previousToken(Token(Token::END)), returnPrevToken(false) {}

  // Generates the next token and removes it from the input string
  Token getNextToken();

  // After revert the next token returned will be the previous token
  void revert() { returnPrevToken = true; }

  // Input string, modified during token generation
  std::string input;

private:
  // The previous token in case we want to "unget" the current token, maybe
  // because we went too far
  Token previousToken;
  bool returnPrevToken;
};

class Parser {
  // EBNF of our context free grammar:
  // sign = "+" | "-"
  // op =  sign | "*"
  // digit = "0"|...|"9"
  // number = [sign] {digit}[.{digit}]
  // variable = {"a"|"A",...,"z"|"Z"|"_"}
  // expression = number | [sign] variable | "(" op {expression} ")"
  // paranthExpression = "(" expression ")"
  
public:
   std::shared_ptr<LogicalExpression> parse(std::string const& input) const;

private:
  std::shared_ptr<LogicalExpression> parseExpression(Lexer& lexer) const;
  std::shared_ptr<LogicalExpression> parseOpExpression(Lexer& lexer) const;
};

#endif /* PARSER_H */
