#include <iostream>

#include "AST.h"

#define C_LMB "\033[38;5;202m"
#define C_ARG "\033[38;5;215m"
#define C_DOT "\033[38;5;202m"

#define C_VAR "\033[38;5;153m"
#define C_CON "\033[38;5;133m"

#define C_SYM "\033[38;5;231m"

#define C_ASG "\033[38;5;133m"

#define C_SUC "\033[38;5;83m"
#define C_ERR "\033[38;5;203m"

#define C_RES "\033[m"

AST::Node::Node(Type type, size_t position, size_t length):
  type(type),
  position(position),
  length(length) {
}

AST::Node::~Node() {
}

AST::Node::Type AST::Node::get_type() const {
  return type;
}

std::string AST::Node::get_type_string() const {
  static char const *const names[] {
    "Variable", "Constant", "Abstraction", "Application", "Assignment",
  };
  return std::string { names[static_cast<int>(type)] };
}

AST::Variable::Variable(int bruijn_index, size_t position, size_t length):
  Node(Type::Variable, position, length),
  bruijn_index(bruijn_index) {
  //
}

AST::Variable::~Variable() {
  //
}

const std::string AST::Variable::to_string() {
  if (bruijn_index and bind_count - bruijn_index >= 0) {
    std::string binding_name = bindings.at(bind_count - bruijn_index)->name;
    return C_VAR + binding_name + C_RES;
  }
  else {
    return C_VAR + std::to_string(bruijn_index) + C_RES;
  }
}

const std::string AST::Variable::to_simplified_string() {
  return std::to_string(bruijn_index);
}

AST::Node *AST::Variable::copy() {
  return new Variable(bruijn_index, position, length);
}

void AST::Variable::offset_indexes(int offset, int current) {
  if (offset < 0 and bruijn_index + offset - current == 0)
    throw RuntimeException("Unreplaced variable had its bind deleted", position, length);
  if (bruijn_index > current)
    bruijn_index += offset;
}

std::set<int> AST::Variable::free_variables(int current_index) {
  if (bruijn_index > current_index)
    return std::set<int>{bruijn_index - current_index};
  return std::set<int>();
}

AST::Node *AST::Variable::beta_reduce(Node *new_term, int current_index) {
  //std::cout << "beta reduce on variable " << to_simplified_string() << ".\n";
  if (bruijn_index == current_index) {
    Node *copy = new_term->copy();
    copy->offset_indexes(current_index);
    delete this;
    return copy;
  }
  return this;
}

AST::Node *AST::Variable::simplify() {
  //std::cout << "simplify variable " << to_simplified_string() << ".\n";
  return this;
}

void AST::Variable::update_name_shadowing(std::map<std::string, int> &binds, size_t position, size_t length) {
  this->position = position;
  this->length = length;
}

AST::Constant::Constant(std::string name, size_t position, size_t length):
  Node(Type::Constant, position, length),
  name(name) {
  //
}

AST::Constant::~Constant() {
  //
}

const std::string AST::Constant::to_string() {
  return C_CON + name + C_RES;
}

const std::string AST::Constant::to_simplified_string() {
  return name;
}

AST::Node *AST::Constant::copy() {
  return new Constant(name, position, length);
}

void AST::Constant::offset_indexes(int offset, int current) {
  //
}

std::set<int> AST::Constant::free_variables(int current_index) {
  return std::set<int>();
}

AST::Node *AST::Constant::beta_reduce(Node *new_term, int current_index) {
  //std::cout << "beta reduce on constant " << to_simplified_string() << ".\n";
  return this;
}

AST::Node *AST::Constant::simplify() {
  //std::cout << "simplify constant " << to_simplified_string() << ".\n";
  return this;
}

void AST::Constant::update_name_shadowing(std::map<std::string, int> &binds, size_t position, size_t length) {
  this->position = position;
  this->length = length;

  auto entry = binds.find(name);
  if (entry != binds.end()) {
    int level = entry->second;
    //std::cout << "found previous at " << level << "\n";

    int count = 2;

    Abstraction *ptr = bindings.at(level);
    while (ptr->previous_bind > -1) {
      ptr = bindings.at(ptr->previous_bind);
      ++count;
    }

    //std::cout << "'" << name << "' changed to '" << name << "(" << count << ")'.\n";
    name = name + "(" + std::to_string(count) + ")";
  }
}

AST::Node *AST::Constant::resolve() {
  Node *value = get_constant(name);
  if (value) {
    //std::cout << "Resolving constant " << name << "\n";
    std::map<std::string, int> binds;
    for (size_t i = 0; i < bindings.size(); ++i) {
      //std::cout << "variable " << bindings.at(i)->name << " found\n";
      binds.insert_or_assign(bindings.at(i)->name, i);
    }
    Node *resolved = value->copy();
    resolved->update_name_shadowing(binds, position, length);
    delete this;
    return resolved;
  }
  else
    return this;
}

AST::Abstraction::Abstraction(std::string name, Node *term, size_t position, size_t length, int previous_bind):
  Node(Type::Abstraction, position, length),
  name(name),
  term(term),
  previous_bind(previous_bind) {
  //
}

AST::Abstraction::~Abstraction() {
  delete term;
}

const std::string AST::Abstraction::to_string() {
  bindings.push_back(this);
  ++bind_count;
  std::string term_string = term->to_string();
  --bind_count;
  bindings.pop_back();
  /*if (previous_bind > -1)
    return C_LMB "\\*" C_ARG + name + C_DOT "." + term_string + C_RES;*/
  return C_LMB "\\" C_ARG + name + C_DOT "." + term_string + C_RES;
}

const std::string AST::Abstraction::to_simplified_string() {
  return "L " + term->to_simplified_string();
}

AST::Node *AST::Abstraction::copy() {
  return new Abstraction(name, term->copy(), position, length, previous_bind);
}

void AST::Abstraction::offset_indexes(int offset, int current) {
  term->offset_indexes(offset, current + 1);
}

std::set<int> AST::Abstraction::free_variables(int current_index) {
  return term->free_variables(current_index + 1);
}

AST::Node *AST::Abstraction::beta_reduce(Node *new_term, int current_index) {
  //std::cout << "beta reduce on abstraction " << to_simplified_string() << ".\n";
  if (previous_bind > -1) {
    std::set<int> variables = new_term->free_variables();
    //std::cout << "Free variables: ";
    //for (auto &x : variables) {
    //  std::cout << x << " ";
    //}
    //std::cout << "\n";
    for (auto &x : variables) {
      //std::cout << "var = " << x << "\n";
      //std::cout << "var - current_index = " << (bind_count - current_index - x) << "\n";
      //std::cout << name << " == " << bindings.at(bind_count - current_index - x)->name << "?\n";
      if (bindings.at(bind_count - current_index - x)->name == name) {
        int count = 1;

        Abstraction *ptr = this;
        while (ptr->previous_bind > -1) {
          ptr = bindings.at(ptr->previous_bind);
          ++count;
        }

        //std::cout << "'" << name << "' changed to '" << name << "(" << count << ")'.\n";
        name = name + "(" + std::to_string(count) + ")";
        break;
      }
    }
  }

  bindings.push_back(this);
  ++bind_count;

  term = term->beta_reduce(new_term, current_index + 1);

  --bind_count;
  bindings.pop_back();
  return this;
}

AST::Node *AST::Abstraction::simplify() {
  //std::cout << "simplify abstraction " << to_simplified_string() << ".\n";
  bindings.push_back(this);
  ++bind_count;

  std::string before, after;
  before = term->to_simplified_string();
  term = term->simplify();
  after = term->to_simplified_string();

  --bind_count;
  bindings.pop_back();

  if (before != after) {
    return this;
  }
  else {
    return eta_reduce();
  }
}

void AST::Abstraction::update_name_shadowing(std::map<std::string, int> &binds, size_t position, size_t length) {
  this->position = position;
  this->length = length;

  auto entry = binds.find(name);
  if (entry == binds.end()) {
    binds.insert({ name, bind_count });
    ++bind_count;

    term->update_name_shadowing(binds, position, length);

    --bind_count;
    binds.erase(name);
  }
  else {
    int level = entry->second;
    previous_bind = level;
    //std::cout << "found previous at " << level << "\n";

    entry->second = bind_count;
    ++bind_count;

    term->update_name_shadowing(binds, position, length);

    --bind_count;
    entry->second = level;
  }
}

AST::Node *AST::Abstraction::eta_reduce() {
  //std::cout << "eta reduce on abstraction " << to_simplified_string() << ".\n";
  //if (term->get_type() == Type::Application) {
  //  std::cout << "is application.\n";
  //  if (((Application *) term)->term2->get_type() == Type::Variable) {
  //    std::cout << "has variable.\n";
  //    std::cout << "bruijn index: " << ((Variable *) ((Application *) term)->term2)->bruijn_index << "\n";

  //    std::cout << "Free variables: ";
  //    for (auto &x : ((Application *) term)->term1->free_variables()) {
  //      std::cout << x << " ";
  //    }
  //    std::cout << "\n";

  //    std::cout << "variable count: " << ((Application *) term)->term1->free_variables().count(1) << "\n";
  //  }
  //  else {
  //    std::cout << "has no variable.\n";
  //  }
  //}
  //else {
  //  std::cout << "is not application.\n";
  //}

  if (term->get_type() == Type::Application
    and ((Application *) term)->term2->get_type() == Type::Variable
    and ((Variable *) ((Application *) term)->term2)->bruijn_index == 1
    and ((Application *) term)->term1->free_variables().count(1) == 0) {

    Node *copy = ((Application *) term)->term1->copy();

    copy->offset_indexes(-1);

    delete this;
    return copy;
  }
  else {
    return this;
  }
}

AST::Application::Application(Node *term1, Node *term2, size_t position, size_t length):
  Node(Type::Application, position, length),
  term1(term1),
  term2(term2) {
  //
}

AST::Application::~Application() {
  delete term1;
  delete term2;
}

const std::string AST::Application::to_string() {
  std::string output = "";

  if (term1->get_type() == Type::Abstraction) {
    output += C_SYM "(" + term1->to_string() + C_SYM ") ";
  }
  else {
    output += term1->to_string() + " ";
  }

  if (term2->get_type() == Type::Application) {
    output += C_SYM "[" + term2->to_string() + C_SYM "]";
  }
  else if (term2->get_type() == Type::Abstraction) {
    output += C_SYM "(" + term2->to_string() + C_SYM ")";
  }
  else {
    output += term2->to_string();
  }
  return output;
}

const std::string AST::Application::to_simplified_string() {
  std::string output;
  if (term1->get_type() == Type::Abstraction) {
    output = "(" + term1->to_simplified_string() + ") ";
  }
  else {
    output += term1->to_simplified_string() + " ";
  }

  if (term2->get_type() == Type::Application) {
    output += "[" + term2->to_simplified_string() + "]";
  }
  else if (term2->get_type() == Type::Abstraction) {
    output += "(" + term2->to_simplified_string() + ")";
  }
  else {
    output += term2->to_simplified_string();
  }
  return output;
}

AST::Node *AST::Application::copy() {
  return new Application(term1->copy(), term2->copy(), position, length);
}

void AST::Application::offset_indexes(int offset, int current) {
  term1->offset_indexes(offset, current);
  term2->offset_indexes(offset, current);
}

std::set<int> AST::Application::free_variables(int current_index) {
  std::set<int> result;
  for (auto &x : term1->free_variables(current_index)) {
    result.insert(x);
  }
  for (auto &x : term2->free_variables(current_index)) {
    result.insert(x);
  }
  return result;
}

AST::Node *AST::Application::beta_reduce(Node *new_term, int current_index) {
  //std::cout << "beta reduce on application " << to_simplified_string() << ".\n";
  term1 = term1->beta_reduce(new_term, current_index);
  term2 = term2->beta_reduce(new_term, current_index);
  return this;
}

AST::Node *AST::Application::simplify() {
  //std::cout << "simplify application " << to_simplified_string() << ".\n";
  std::string before, after;

  before = term1->to_simplified_string();
  term1 = term1->simplify();
  after = term1->to_simplified_string();
  if (before != after) return this;

  before = term2->to_simplified_string();
  term2 = term2->simplify();
  after = term2->to_simplified_string();
  if (before != after) return this;

  if (term1->get_type() == Type::Abstraction) {
    term1 = term1->beta_reduce(term2);
    //std::cout << "Finished applying to " << to_simplified_string() << ".\n";
    Abstraction *term1_abstraction = (Abstraction *) term1;
    term1_abstraction->term->offset_indexes(-1);
    Node *copy = term1_abstraction->term->copy();
    //std::cout << "Finished copying " << copy->to_simplified_string() << ".\n";
    delete this;
    return copy;
  }
  else if (term1->get_type() == Type::Constant) {
    term1 = ((Constant *) term1)->resolve();
    return this;
  }

  return this;
}

void AST::Application::update_name_shadowing(std::map<std::string, int> &binds, size_t position, size_t length) {
  this->position = position;
  this->length = length;

  term1->update_name_shadowing(binds, position, length);
  term2->update_name_shadowing(binds, position, length);
}

AST::Assignment::Assignment(std::string name, Node *term, size_t position, size_t length):
  Node(Type::Assignment, position, length),
  name(name),
  term(term) {
  //
}

AST::Assignment::~Assignment() {
  delete term;
}

const std::string AST::Assignment::to_string() {
  return C_ASG + name + C_SYM " = " + term->to_string() + C_RES;
}

const std::string AST::Assignment::to_simplified_string() {
  return name + " = " + term->to_simplified_string();
}

AST::Node *AST::Assignment::copy() {
  return new Assignment(name, term->copy(), position, length);
}

void AST::Assignment::offset_indexes(int offset, int current) {
  throw RuntimeException("Invalid operation on assignment", position, length);
  term->offset_indexes(offset, current);
}

std::set<int> AST::Assignment::free_variables(int current_index) {
  throw RuntimeException("Invalid operation on assignment", position, length);
  return term->free_variables(current_index);
}

AST::Node *AST::Assignment::beta_reduce(Node *new_term, int current_index) {
  throw RuntimeException("Invalid operation on assignment", position, length);
  term = term->beta_reduce(new_term, current_index);
  return this;
}

AST::Node *AST::Assignment::simplify() {
  term = term->simplify();
  return this;
}

void AST::Assignment::update_name_shadowing(std::map<std::string, int> &binds, size_t position, size_t length) {
  throw RuntimeException("Invalid operation on assignment", position, length);
  this->position = position;
  this->length = length;
  term->update_name_shadowing(binds, position, length);
}

std::string AST::to_string(Node *node) {
  bindings = std::vector<Abstraction *>();
  return node->to_string();
}


std::string AST::solve(Node *node, std::string expression) {

  Node *current = node->copy();
  std::string before, after;

  try {
    std::cout << "\n> " << to_string(current) << "\n";

    for (int i = 0; i < 100; ++i) {

      before = current->to_simplified_string();
      current = current->simplify();
      after = current->to_simplified_string();

      //std::cout << "From: " << before << "\n";
      //std::cout << "To:   " << after << "\n";

      if (before == after) goto success;

      std::cout << "= " << to_string(current) << "\n";
    }
    //std::cout << current->get_type_string() << "\n";
    throw RuntimeException("Infinite lambda expression", current->position, current->length);
  success:;
    //std::cout << "Finished!\n";
  }
  catch (const RuntimeException &exception) {
    print_error(exception, expression);
    delete current;
    return "";
  }

  if (current->get_type() == Node::Type::Assignment) {
    Assignment *assignment = (Assignment *) current;
    Node *term = assignment->term->copy();
    std::string assignment_name = assignment->name;
    delete assignment;
    if (term->get_type() == Node::Type::Constant
      and ((Constant *) term)->name == assignment_name) {
      remove_constant(assignment_name);
      return C_ERR "Deleted constant " C_CON + assignment_name + C_RES;
    }
    else {
      set_constant(assignment_name, term);
      return C_SUC "Set constant " C_CON + assignment_name + C_SUC " to " + to_string(term) + C_RES;
    }
  }
  else {
    std::string result = to_string(current);
    delete current;
    return result;
  }
}

void AST::init() {
  dictionary = std::map<std::string, Node *>();
}

AST::Node *AST::get_constant(std::string name) {
  auto entry = dictionary.find(name);
  if (entry == dictionary.end()) {
    return nullptr;
  }
  else {
    return entry->second;
  }
}

void AST::set_constant(std::string name, Node *value) {
  dictionary.insert_or_assign(name, value);
}

void AST::remove_constant(std::string name) {
  dictionary.erase(name);
}

void AST::end() {
  for (auto &x : dictionary) {
    delete x.second;
  }
}

std::string AST::to_simplified_string(Node *node) {
  return node->to_simplified_string();
}

std::vector<AST::Abstraction *> AST::bindings;
int AST::bind_count;

std::map<std::string, AST::Node *> AST::dictionary;

void AST::print_error(const ParserException &exception, std::string expression) {
  expression += " ";
  size_t position = exception.get_position(), length = exception.get_length();
  /*if (position >= expression.length()) {
    position = 0;
    length = expression.length();
  }
  else*/ if (position + length >= expression.length()) {
    length = expression.length() - position;
  }

  std::cout << "\n" << exception.get_name() << "! " << exception.get_message() << " at " << position << ".\n"
    << "\033[31m" << expression.substr(0, position)
    << "\033[37;41m" << expression.substr(position, length)
    << "\033[;31m" << expression.substr(position + length)
    << "\033[m\n";
}
