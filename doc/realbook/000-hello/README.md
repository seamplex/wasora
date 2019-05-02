
...
title: Hello world
inputs: hello
tags: PRINT TEXT
---

# Hello world

The classical example that displays “hello world!” at the standard output and exits.

## hello.was

To solve the canonical [Hello World!](http://en.wikipedia.org/wiki/Hello_world_program) problem with [wasora](https://www.seamplex.com/wasora), just prepare a text file like the one shown below and call it for example `hello.was`. Then run wasora with `hello.was` as its only argument as also shown below, and you are done. If this is the first wasora example you read, please do try to run it by yourself before proceeding with the other ones.

```wasora
PRINT TEXT "hello world!"
```

```bash
$ wasora hello.was
hello world!	
$ 
```

The keyword `TEXT` is not strictly needed (unless given the `IMPLICIT NONE` keyword), as the construction `hello world!` does not resolve to any known symbol. However, it is a good practice to explictly precede any text with the `TEXT` keyword.

-------
:::{.text-center}
[Previous](../) | [Index](../) | [Next](../001-peano)
:::
