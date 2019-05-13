# How to generate wasora’s documentation

Compile `reference.m4` with `m4` and redirect it to `reference.md` 

```
m4 reference.m4 > reference.md
```


This will call `reference.sh` which parses the parsing source file.


## Generate the wasora logo in PDF format

```
$ inkscape --export-area-drawing --export-pdf=logo.pdf logo.svg
```

## Have pandoc support wasora

You're in luck.  This gets much easier in the dev version of
pandoc (to be released eventually as 2.0); you can just use
a command line option --syntax-defintion=FILE.

You might try getting a binary from
pandoc-extras/pandoc-nightly, but be aware that there are
many other changes as well.
