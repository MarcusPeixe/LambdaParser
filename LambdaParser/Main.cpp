#include <iostream>
#include <memory>
#include "Parser.h"

int main(int argc, const char *argv[]) {
  std::string expression;
  std::unique_ptr<AST::Node> node;
  AST::init();

  //std::getline(std::cin, expression);
  //expression = "(\b.b (\x y.y) (\x y.x)) \x y.x";
  //expression = "(\x y.(\z.(\x.z x) (\y.z y)) (x y))";
  //expression = "aaaa = bbb \x y z.x y z";
  //expression = "(\x.x x f) (\x.x x f)";

  while (true) {
    std::cout << "\nType a new lambda expression:\n> ";
    std::getline(std::cin, expression);
    if (expression == "") break;

    node.reset(Parser::parse(expression));
    if (!node) continue;

    std::string result = AST::solve(node.get(), expression);
    if (result == "") continue;
    //std::cout << "Expression: " << AST::to_string(node.get()) << "\n";
    //std::cout << "Result:     " << result << "\n";
    std::cout << "\n= " << result << "\n";
  }

  AST::end();
  return 0;
}

/*

true = \x y.x
false = \x y.y
not = \x.x false true
and = \x y.x y false
or = \x y.x true y
xor = \x y.or (and x (not y)) (and y (not x))

> (\b. b(\x.\y.y)(\x.\y.x)) \x.\y.x

= (\b.b (\x.\y.y) (\x.\y.x)) (\x.\y.x)
= (\x.\y.x) (\x.\y.y) (\x.\y.x)
= (\x.\y.y)




> (\b.b (\x.\y.y) (\x.\y.x)) (\x.\y.x)

Apl {
  t1: Abs {
    arg: Var "b"
    t: Apl {
      t1: Apl {
        t1: Exp "b"
        t2: Abs {
          arg: Var "x"
          t: Abs {
            arg: Var "y"
            t: Exp "y"
          }
        }
      }
      t2: Abs {
        arg: Var "x"
        t: Abs {
          arg: Var "y"
          t: Exp "x"
        }
      }
    }
  }
  t2: Abs {
    arg: Var "x"
    t: Abs {
      arg: Var "y"
      t: Exp "x"
    }
  }
}


\x y.(\z.(\x.z x) (\y.z y)) (x y)
>\x.\y.(\z.(\x.z x) (\y.z y)) (x y)
>\x.\y.(\z.z z) (x y)
>\x.\y.x y (x y)

(\a.(\b.(\c.(\d.d c b a))))

(L (L (L (L 1 2 3 4))))

rev a b c d
(\x.x a) b c d
(\x.x b a) c d

*/