
---
title: The Peano axiom
inputs: peano
tags: PRINT expressions
...

-------

:::{.text-center}
[Previous](../000-hello) | [Index](../) | [Next](../002-expressions)
:::

# The Peano axiom

This example shows how the sum $1+1$ can be computed and printed to the standard output---i.e. the screen---in a variety of ways.

## peano.was

The `PRINT` keyword may take algebraic expressions, which are rendered to ASCII expressions using a `scanf`-type format specifier. The default format is `%e` (see the C library reference for `scanf` for details). The last format specifier given is used to render each expression. New `PRINT` keywords reset the format specifier to the default. Expressions may be mixed with text, as shown in the last line.

```wasora
PRINT 1+1      # with default format
PRINT %g 1+1   # with numeric format

# by means of an intermediate variable
a = 1+1
PRINT %g a 

# with an informative text
PRINT "Peano says that one plus one equals" %.0f a
```

```bash
$ wasora peano.was
2
2
2
Peano says that one plus one equals	2
$ 
```

-------

:::{.text-center}
[Previous](../000-hello) | [Index](../) | [Next](../002-expressions)
:::
