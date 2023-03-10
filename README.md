# Lambda Calculus interpreter

This project implements a very simple and rudimentary interpreter for
[Lambda Calculus](https://en.wikipedia.org/wiki/Lambda_calculus "Wikipedia")
expressions.

## What is Lambda Calculus

Lambda Calculus is a system for writing lambda expressions and evaluating them.
The syntax is:

* $x$ - variable
* $(\lambda x.\ M)$ - abstraction ($M$ is a lambda sub-expression)
* $M\ N$ - application ($M$ and $N$ are lambda sub-expressions)

And the rules are:

* $(\lambda a.\ a\ b) \rightarrow (\lambda x.\ x\ b)$ $\alpha$-conversion
(basic renaming of variables in a function).
* $(\lambda a.\ a\ b\ c)\ (d\ e) \rightarrow (d\ e)\ b\ c$ - $\beta$-reduction
(applying a function to an argument expression).

You can think of abstractions (also called functions) as some sort of macro
definitions, and their $\beta$-reductions as macro expansions. The
$\alpha$-conversions are sometimes necessary to avoid naming collisions.
After applying all reductions, you will get the beta-normal (irreducible)
form of the expression.

Lambda Calculus is turing-complete. Read more at the Wikipedia article linked
above.

## How to use

With this interpreter, you can evaluate lambda expressions, define constants
and assign expressions to them, and delete these constants.

When you run the program, it will prompt you for an expression.

```
Type a new lambda expression:
> 
```

You can type a lambda expression (replace $\lambda$'s with backslashes `\` ).

```
> (\x.x (\a.(\b.b)) (\a.(\b.a))) (\a.(\b.b))
...
= \a.\b.a
```

Please note:
* The abstraction parser is greedy, so everything after a dot will be part of
  the function (you can use parentheses to enclose the function and tell it
  where to stop).
* If you chain abstractions (like in `\a.\b.a`), you can use rewrite it as
  `\a b.a` as a form of syntactic sugar.
* Function applications are done by just writing a function with its arguments
  in front of it. Parentheses are only needed if you want to change the order
  of application. E.g.: `func1 arg1 arg2 (func2 arg3)`

So the previous expression is equivalent to:

```
> (\x.x (\a b.b) (\a b.a)) \a b.b
...
= \a.\b.a
```

You can define constants

```
> true = \a.\b.a
> false = \a.\b.b
> not = \x.x false true
```

and use them later.

```
> not false
...
= true
```

Notice that this last example was identical to the previous one, just with
some terms defined as constants. In Lambda Calculus, you can implement some
data types, such as booleans and their logic operations.

Here what we've done is define `true` as a function that takes two arguments
and returns the first one, and `false` as a function that also takes two
arguments but returns the second one. `not` can then be defined as a function
that takes a boolean `x` and evaluates `x false true`. You can hopefully see
why that works, and how one could go on implementing the other logic gates.

To undefine a constant, you can assign it to itself

```
> not = not
= Deleted constant not
```

To quit, just press <kbd>Enter</kbd> (submit an empty expression).

This interpreter points out syntax errors and prints a "parsing" stack trace.

## Build instructions

To build this project on Linux, open up a terminal, navigate to the directory
where you want to clone the repository to, and type:

```bash
git clone https://github.com/MarcusPeixe/LambdaParser.git
cd LambdaParser
make
```

to run it, type:

```bash
./main.out
```

---