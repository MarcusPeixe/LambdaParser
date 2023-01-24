#pragma once

#include <string>
#include <stack>

class ParserException;
class TokenException;
class ParsingException;
class StackTrace;
class StackEntry;

// EXCEPTIONS

class ParserException {
public:
  ParserException(const std::string message, size_t position, size_t length);

  std::string get_message() const;
  size_t get_position() const;
  size_t get_length() const;

  virtual std::string get_name() const = 0;

private:
  const std::string message;
  size_t position, length;
};

class TokenException : public ParserException {
public:
  TokenException(const std::string message, size_t position, size_t length);

  std::string get_name() const;
};

class ParsingException : public ParserException {
public:
  ParsingException(const std::string message, size_t position);

  std::string get_name() const;
};

class RuntimeException : public ParserException {
public:
  RuntimeException(const std::string message, size_t position, size_t length);

  std::string get_name() const;
};

// STACK TRACE

class StackEntry {
public:
  StackEntry(const std::string function, size_t position);

  std::string get_function() const;
  size_t get_position() const;

  const std::string function;
  size_t position;
};

class StackTrace {
public:
  StackTrace();

  void push(const std::string function, size_t position);
  void pop();
  bool empty() const;
  StackEntry top() const;

private:
  std::stack<StackEntry> stack;
};