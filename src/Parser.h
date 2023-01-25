#pragma once

#include <string>
#include <map>

#include "ParserExceptions.h"
#include "AST.h"

class Parser {
public:
  static AST::Node *parse(const std::string expression);
  static std::string get_expression();

private:
  Parser() = default;

  // STRING

  static char seek();
  static void next();
  static size_t get_position();
  static void set_position(size_t position);

  static std::string expression;
  static size_t position;

  // TOKENS

  static void skip_space();
  static std::string parse_name_token();
  static std::string parse_number_token();
  static std::string parse_symbol_token();

  // PARSING

  static AST::Node *parse_abstraction_chain();
  static AST::Node *parse_abstraction();
  static AST::Node *parse_parenthesised();
  static AST::Node *parse_variable();
  static AST::Node *parse_term();
  static AST::Node *parse_application_chain();
  static AST::Node *parse_assignment();

  static AST::Abstraction *create_binding(std::string name, size_t start);
  static AST::Node *create_variable(std::string name, size_t start, size_t length);

  // REDUCING

  static StackTrace stack_trace;

  static std::map<std::string, int> bind_levels;
  static int bind_count;

  static void print_error(const ParserException &exception);
};