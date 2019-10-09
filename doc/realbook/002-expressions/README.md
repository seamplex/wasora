
---
title: Algebraic expressions
inputs: expr hoc
tags: PRINT = expressions TEXT
...

-------

:::{.text-center}
[Previous](../001-peano) | [Index](../) | [Next](../003-lag)
:::

# Algebraic expressions


These examples show how algebraic expressions work in wasora, which is basically as they should. The basic operators are composed by the four basic operations `+`, `-`, `*` and `/` plus the exponentation operator `^`. Parenthesis also work as expected, and can be nested up to any level. Other operators such as comparisons and logicals are introduced in other examples. 

## expr.was

Expressions works as expected. If you find one case where they do not, please report it to the author of wasora.

```wasora
# algebraic expressions do whatever one expects them to do
# thus a should evaluate to 7
a = 1 + 2*3
# and b should evaluate to 9
b = (1+2) * 3

# expressions can contain variables and functions, whose
# arguments may be expressions themselves, and so on
c = (1 + sin(pi/4)^(5/2))/(1 - log(abs(a-b)))

PRINT %g a b c

# when an expression appears as an argument of a keyword and
# contains spaces it should be written within double quotes
PRINT %g 1 -1 "1 -1"
```

```bash
$ wasora expr.was
7	9	4.62909
1	-1	0
$ 
```

By default `PRINT` introduces a tab between its arguments. This behavior can be changed with the `SEPARATOR` keyword.

## hoc.was

When `PRINT` finds a new argument, it tries to evaluate it. If it does not make any sense, then wasora treats it as a text string. However, if a certain argument that constitutes a valid algebraic expression is to be treated as a text, it should be prefixed with the `TEXT` keyword, as the following example illustrates.

```wasora
# actually arguments of the PRINT keyword may be either text
# strings, scalar expresions, vectors or matrices (more about
# these latter two issues in more complex examples)

# the example of Kernigham & Ritchie HOC code
PRINT TEXT "1+2*sin(0.7)" "=" %.6f 1+2*sin(0.7)
```

```bash
$ wasora hoc.was
1+2*sin(0.7)	=	2.288435
$ 
```

Note that the double quotes are not needed, as there are no spaces in `1+2*sin(0.7)`. However, it is a good practice to leave them for text strings, as one might want to add extra spaces in the future.


-------

:::{.text-center}
[Previous](../001-peano) | [Index](../) | [Next](../003-lag)
:::
