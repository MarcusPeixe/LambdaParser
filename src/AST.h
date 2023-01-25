#pragma once

#include <string>
#include <map>
#include <vector>
#include <set>

#include "ParserExceptions.h"

class AST {
public:
  class Node;
  class Variable;
  class Abstraction;
  class Application;

  // NODES

  class Node {
  public:
    friend class AST;
    enum class Type {
      Variable, Constant, Abstraction, Application, Assignment
    };

    Node(Type type, size_t position, size_t length);
    virtual ~Node();

    Type get_type() const;
    std::string get_type_string() const;

  private:
    virtual const std::string to_string() = 0;
    virtual const std::string to_simplified_string() = 0;
    virtual Node *copy() = 0;
    virtual void offset_indexes(int offset, int current = 0) = 0;
    virtual std::set<int> free_variables(int current_index = 0) = 0;
    virtual Node *beta_reduce(Node *new_term, int current_index = 0) = 0;
    virtual Node *simplify() = 0;
    virtual void update_name_shadowing(std::map<std::string, int> &binds, size_t position, size_t length) = 0;

    const Type type;
    size_t position;
    size_t length;
  };

  class Variable : public Node {
  public:
    friend class AST;
    Variable(int bruijn_index, size_t position, size_t length);
    ~Variable();

  private:
    const std::string to_string();
    const std::string to_simplified_string();
    Node *copy();
    void offset_indexes(int offset, int current = 0);
    std::set<int> free_variables(int current_index = 0);
    Node *beta_reduce(Node *new_term, int current_index = 0);
    Node *simplify();
    void update_name_shadowing(std::map<std::string, int> &binds, size_t position, size_t length);

    int bruijn_index;
  };

  class Constant : public Node {
  public:
    friend class AST;
    Constant(std::string name, size_t position, size_t length);
    ~Constant();

  private:
    const std::string to_string();
    const std::string to_simplified_string();
    Node *copy();
    void offset_indexes(int offset, int current = 0);
    std::set<int> free_variables(int current_index = 0);
    Node *beta_reduce(Node *new_term, int current_index = 0);
    Node *simplify();
    void update_name_shadowing(std::map<std::string, int> &binds, size_t position, size_t length);

    Node *resolve();

    std::string name;
  };

  class Abstraction : public Node {
  public:
    friend class AST;
    Abstraction(std::string name, Node *term, size_t position, size_t length, int previous_bind = -1);
    ~Abstraction();

  private:
    const std::string to_string();
    const std::string to_simplified_string();
    Node *copy();
    void offset_indexes(int offset, int current = 0);
    std::set<int> free_variables(int current_index = 0);
    Node *beta_reduce(Node *new_term, int current_index = 0);
    Node *simplify();
    void update_name_shadowing(std::map<std::string, int> &binds, size_t position, size_t length);

    Node *eta_reduce();

    std::string name;
    Node *term;
    int previous_bind;
  };

  class Application : public Node {
  public:
    friend class AST;
    Application(Node *term1, Node *term2, size_t position, size_t length);
    ~Application();

  private:
    const std::string to_string();
    const std::string to_simplified_string();
    Node *copy();
    void offset_indexes(int offset, int current = 0);
    std::set<int> free_variables(int current_index = 0);
    Node *beta_reduce(Node *new_term, int current_index = 0);
    Node *simplify();
    void update_name_shadowing(std::map<std::string, int> &binds, size_t position, size_t length);

    Node *term1;
    Node *term2;
  };

  class Assignment : public Node {
  public:
    friend class AST;
    Assignment(std::string name, Node *term, size_t position, size_t length);
    ~Assignment();

  private:
    const std::string to_string();
    const std::string to_simplified_string();
    Node *copy();
    void offset_indexes(int offset, int current = 0);
    std::set<int> free_variables(int current_index = 0);
    Node *beta_reduce(Node *new_term, int current_index = 0);
    Node *simplify();
    void update_name_shadowing(std::map<std::string, int> &binds, size_t position, size_t length);

    std::string name;
    Node *term;
  };

  static std::string to_string(Node *node);

  static std::string solve(Node *node, std::string expression);

  static void init();
  static Node *get_constant(std::string name);
  static void set_constant(std::string name, Node *value);
  static void remove_constant(std::string name);
  static void end();

private:
  static std::string to_simplified_string(Node *node);

  static std::vector<Abstraction *> bindings;
  static int bind_count;

  static std::map<std::string, Node *> dictionary;

  static void print_error(const ParserException &exception, std::string expression);
};