#include "ParserExceptions.h"

ParserException::ParserException(const std::string message, size_t position, size_t length):
  message(message),
  position(position),
  length(length) {
  //
}

std::string ParserException::get_message() const {
  return message;
}

size_t ParserException::get_position() const {
  return position;
}

size_t ParserException::get_length() const {
  return length;
}

TokenException::TokenException(const std::string message, size_t position, size_t length):
  ParserException(message, position, length) {
  //
}

std::string TokenException::get_name() const {
  return "TokenException";
}

ParsingException::ParsingException(const std::string message, size_t position) :
  ParserException(message, position, 1) {
  //
}

std::string ParsingException::get_name() const {
  return "ParsingException";
}

RuntimeException::RuntimeException(const std::string message, size_t position, size_t length) :
  ParserException(message, position, length) {

}

std::string RuntimeException::get_name() const {
  return "RuntimeException";
}

StackEntry::StackEntry(const std::string function, size_t position) :
  function(function),
  position(position) {
}

std::string StackEntry::get_function() const {
  return function;
}

size_t StackEntry::get_position() const {
  return position;
}

StackTrace::StackTrace():
  stack() {
  //
}

void StackTrace::push(const std::string function, size_t position) {
  //std::cout << "Entered function \"" + function + "\".\n";
  stack.push(StackEntry(function, position));
}

void StackTrace::pop() {
  stack.pop();
}

bool StackTrace::empty() const {
  return stack.empty();
}

StackEntry StackTrace::top() const {
  return stack.top();
}
