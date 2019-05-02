define(case_title, Hello world)
---
title: case_title
inputs: hello
tags: PRINT TEXT
...

# case_title

The classical example that displays “hello world!” at the standard output and exits.

## hello.was

To solve the canonical [Hello World!](http://en.wikipedia.org/wiki/Hello_world_program) problem with [wasora](https://www.seamplex.com/wasora), just prepare a text file like the one shown below and call it for example `hello.was`. Then run wasora with `hello.was` as its only argument as also shown below, and you are done. If this is the first wasora example you read, please do try to run it by yourself before proceeding with the other ones.

```wasora
include(hello.was)dnl
```

```bash
include(hello.term)dnl
```

The keyword `TEXT` is not strictly needed (unless given the `IMPLICIT NONE` keyword), as the construction `hello world!` does not resolve to any known symbol. However, it is a good practice to explictly precede any text with the `TEXT` keyword.

case_nav
