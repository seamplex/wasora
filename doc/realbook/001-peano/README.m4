define(case_title, The Peano axiom)
---
title: case_title
inputs: peano
tags: PRINT expressions
...

case_nav

# case_title

This example shows how the sum $1+1$ can be computed and printed to the standard output---i.e. the screen---in a variety of ways.

## peano.was

The `PRINT` keyword may take algebraic expressions, which are rendered to ASCII expressions using a `scanf`-type format specifier. The default format is `%e` (see the C library reference for `scanf` for details). The last format specifier given is used to render each expression. New `PRINT` keywords reset the format specifier to the default. Expressions may be mixed with text, as shown in the last line.

```wasora
include(peano.was)dnl
```

```bash
include(peano.term)dnl
```

case_nav
