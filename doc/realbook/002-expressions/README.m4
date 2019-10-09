define(case_title, Algebraic expressions)
---
title: case_title
inputs: expr hoc
tags: PRINT = expressions TEXT
...

case_nav

# case_title


These examples show how algebraic expressions work in wasora, which is basically as they should. The basic operators are composed by the four basic operations `+`, `-`, `*` and `/` plus the exponentation operator `^`. Parenthesis also work as expected, and can be nested up to any level. Other operators such as comparisons and logicals are introduced in other examples. 

## expr.was

Expressions works as expected. If you find one case where they do not, please report it to the author of wasora.

```wasora
include(expr.was)dnl
```

```bash
include(expr.term)dnl
```

By default `PRINT` introduces a tab between its arguments. This behavior can be changed with the `SEPARATOR` keyword.

## hoc.was

When `PRINT` finds a new argument, it tries to evaluate it. If it does not make any sense, then wasora treats it as a text string. However, if a certain argument that constitutes a valid algebraic expression is to be treated as a text, it should be prefixed with the `TEXT` keyword, as the following example illustrates.

```wasora
include(hoc.was)dnl
```

```bash
include(hoc.term)dnl
```

Note that the double quotes are not needed, as there are no spaces in `1+2*sin(0.7)`. However, it is a good practice to leave them for text strings, as one might want to add extra spaces in the future.


case_nav
