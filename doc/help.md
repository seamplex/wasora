`-d` or `--debug`

:    Start in debug mode


`--node-debug`

:    Ignore standard input, avoid debug mode


`-l` or `--list`

:    List defined symbols and exit


`-h` or `--help`

:    Display this help and exit

`-i` or `--info`

:    Display detailed code information and exit


`-v` or `--version`

:    Display version information and exit


Input file instructions are read from standard input if a dash `-` is passed as `input-file`.

Wasora accepts _optional extra arguments_ which are then verbatimly replaced in the input file as `$1`, `$2`, and so on. So for example if an input file has a line like this

```wasora
PRINT "$1 squared is" ($1)^2
```

Then two different meshes called `one.msh` and `two.msh` can successively be used in two runs with the same input file by calling Fino as 

```
$ wasora input.was 2
2 squared is    4
$ wasora input.was 3
3 squared is    9
$ 
```

