#include <iostream>

#include "Parser.h"

AST::Node *Parser::parse(const std::string expression) {
  Parser::expression = expression;
  position = 0;
  stack_trace = StackTrace();
  bind_levels = std::map<std::string, int>();
  bind_count = 0;
  try {
    stack_trace.push("parse", get_position());
    AST::Node *node = parse_assignment();

    if (position < expression.length())
      throw TokenException("Invalid element", position, expression.length() - position);
    stack_trace.pop();
    return node;
  }
  catch (const ParserException &exception) {
    print_error(exception);
    return nullptr;
  }
}

std::string Parser::get_expression() {
  return expression;
}

char Parser::seek() {
  if (position == expression.size()) {
    return EOF;
  }
  else {
    return expression[position];
  }
}

void Parser::next() {
  if (position != expression.size()) {
    ++position;
  }
}

size_t Parser::get_position() {
  return position;
}

void Parser::set_position(size_t position) {
  Parser::position = position;
}

std::string Parser::expression;
size_t Parser::position;

void Parser::skip_space() {
  char next_char = seek();
  while (next_char == ' '
    or next_char == '\t'
    or next_char == '\n'
    or next_char == '\r') {
    next();
    next_char = seek();
  }
}

static bool is_alpha(char c) {
  if (c == EOF) return false;
  return (c >= 'a' and c <= 'z')
    or (c >= 'A' and c <= 'Z')
    or (c >= '0' and c <= '9')
    or c == '_';
}

static bool is_digit(char c) {
  return c >= '0' and c <= '9';
}

std::string Parser::parse_name_token() {
  stack_trace.push("parse_name_token", get_position());

  bool has_letters = false;
  bool is_name = false;
  skip_space();
  size_t start = get_position();

  while (is_alpha(seek())) {
    is_name = true;
    if (!is_digit(seek())) has_letters = true;
    next();
  }

  if (has_letters) {
    stack_trace.pop();
    return expression.substr(start, get_position() - start);
  }
  else if (!is_name) {
    set_position(start);
    stack_trace.pop();
    return "";
  }
  else {
    throw TokenException("Invalid name", start, get_position() - start);
  }
}

std::string Parser::parse_number_token() {
  stack_trace.push("parse_number_token", get_position());

  skip_space();
  size_t start = get_position();
  while (is_digit(seek())) {
    //char next_char = seek();
    //number = number * 10 + next_char - '0';
    next();
  }

  if (is_alpha(seek())) {
    throw TokenException("Invalid name", start, get_position() - start + 1);
    //set_position(start);
    //stack_trace.pop();
    //return nullptr;
  }

  stack_trace.pop();
  return expression.substr(start, get_position() - start);
}

std::string Parser::parse_symbol_token() {
  std::string symbol;
  skip_space();
  switch (seek()) {
  case '=': // Assign
  case '.': // Dot
  case '\\':// Lambda
  case '(': // Opening_p
  case ')': // Closing_p
  case '+': // Plus
  case '-': // Minus
  case '*': // Times
  case '/': // Divided
    symbol = expression.substr(position, 1);
    next();
    return symbol;
  default:
    return "";
  };
}

AST::Node *Parser::parse_abstraction_chain() {
  stack_trace.push("parse_abstraction_chain", get_position());
  skip_space();
  size_t start = get_position();

  if (parse_symbol_token() == ".") {
    AST::Node *term = parse_application_chain();

    stack_trace.pop();
    return term;
  }

  std::string name = parse_name_token();
  if (name != "") {
    AST::Node *abstraction = create_binding(name, start);

    stack_trace.pop();
    return abstraction;
  }

  throw ParsingException("Expected identifier or dot", get_position());
}

AST::Node *Parser::parse_abstraction() {
  stack_trace.push("parse_abstraction", get_position());
  skip_space();
  size_t start = get_position();

  if (parse_symbol_token() != "\\") {
    throw ParsingException("Not an abstraction", get_position());
  }

  std::string name = parse_name_token();
  if (name == "") {
    throw ParsingException("Not an abstraction", get_position());
  }

  AST::Node *abstraction = create_binding(name, start);

  stack_trace.pop();
  return abstraction;
}

AST::Node *Parser::parse_parenthesised() {
  stack_trace.push("parse_parenthesised", get_position());

  if (parse_symbol_token() != "(") {
    throw ParsingException("Not a parenthesised term", get_position());
  }

  AST::Node *term = parse_application_chain();

  if (parse_symbol_token() != ")") {
    throw ParsingException("Missing closing parenthesis", get_position());
  }

  stack_trace.pop();
  return term;
}

AST::Node *Parser::parse_variable() {
  stack_trace.push("parse_variable", get_position());
  skip_space();
  size_t start = get_position();

  std::string name = parse_name_token();
  if (name != "") {
    AST::Node *variable = create_variable(name, start, get_position() - start);

    stack_trace.pop();
    return variable;
  }
  else {
    stack_trace.pop();
    return nullptr;
  }
}

AST::Node *Parser::parse_term() {
  stack_trace.push("parse_term", get_position());
  skip_space();
  size_t start = get_position();

  {
    std::string symbol = parse_symbol_token();
    set_position(start);
    if (symbol == "\\") {
      AST::Node *abstraction = parse_abstraction();
      stack_trace.pop();
      return abstraction;
    }
    else if (symbol == "(") {
      AST::Node *parenthesised = parse_parenthesised();
      stack_trace.pop();
      return parenthesised;
    }
  }
  {
    std::string name = parse_name_token();
    set_position(start);
    if (name != "") {
      AST::Node *variable = parse_variable();
      stack_trace.pop();
      return variable;
    }
  }

  stack_trace.pop();
  return nullptr;
}

AST::Node *Parser::parse_application_chain() {
  stack_trace.push("parse_application_chain", get_position());
  skip_space();
  size_t start = get_position();

  AST::Node *term = parse_term();
  if (!term) {
    throw ParsingException("Expected term", get_position());
  }

  while (true) {
    AST::Node *next_term = parse_term();
    if (!next_term) break;

    term = new AST::Application(term, next_term, start, get_position() - start);
  }

  stack_trace.pop();
  return term;
}

AST::Node *Parser::parse_assignment() {
  stack_trace.push("parse_assignment", get_position());
  skip_space();
  size_t start = get_position();

  std::string name = parse_name_token();
  if (name == "") {
    set_position(start);
    return parse_application_chain();
  }

  if (parse_symbol_token() != "=") {
    //throw ParsingException("Not an assignment", get_position());
    set_position(start);
    return parse_application_chain();
  }

  AST::Node *term = parse_application_chain();

  stack_trace.pop();
  return new AST::Assignment(name, term, start, get_position() - start);
}

AST::Abstraction *Parser::create_binding(std::string name, size_t start) {
  auto entry = bind_levels.find(name);
  if (entry == bind_levels.end()) {
    bind_levels.insert(std::make_pair(name, bind_count));
    ++bind_count;

    AST::Node *term = parse_abstraction_chain();
    get_position();

    --bind_count;
    bind_levels.erase(name);

    return new AST::Abstraction(name, term, start, get_position() - start);
  }
  else {
    int level = entry->second;
    entry->second = bind_count;
    ++bind_count;
    AST::Node *term = parse_abstraction_chain();

    --bind_count;
    entry->second = level;

    return new AST::Abstraction(name, term, start, get_position() - start, level);
  }

}

AST::Node *Parser::create_variable(std::string name, size_t start, size_t length) {
  auto entry = bind_levels.find(name);
  if (entry == bind_levels.end()) {
    return new AST::Constant(name, start, length);
  }
  else {
    int level = entry->second;
    return new AST::Variable(bind_count - level, start, length);
  }
}

StackTrace Parser::stack_trace;

std::map<std::string, int> Parser::bind_levels;
int Parser::bind_count;

void Parser::print_error(const ParserException &exception) {
  expression += " ";

  size_t position = exception.get_position(), length = exception.get_length();
  /*if (position >= expression.length())
    position = 0;*/
  if (position + length >= expression.length())
    length = expression.length() - position;

  std::cout << "\n" << exception.get_name() << "! " << exception.get_message() << " at " << position << ".\n"
    << "\033[31m" << expression.substr(0, position)
    << "\033[37;41m" << expression.substr(position, length)
    << "\033[0;31m" << expression.substr(position + length)
    << "\033[m\n";

  while (!stack_trace.empty()) {
    StackEntry entry = stack_trace.top();
    size_t position = entry.position;

    std::cout << "- At function \"" + entry.function + "\"\n"
      << "\033[41;37m" << expression.substr(0, position)
      << "\033[40;31m" << expression.substr(position)
      << "\033[m\n";

    stack_trace.pop();
  }
}

