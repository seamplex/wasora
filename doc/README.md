# How to generate wasora’s documentation

The documentation of wasora is contained in a set of files that act as a source for many different output formats (html, pdf, sphinx, plain text, etc) using the meta-markup tool doconce. Besides, the keywords, special variables, built-in functions, etc. are described in the very source code where they are implemented so the distance between the code that performs an integration and the description of what the parameters should be are located within two lines of distance in the same file.


## Generate the wasora logo in PDF format

    $ inkscape --export-area-drawing --export-pdf=logo.pdf logo.svg

## Have pandoc support wasora

You're in luck.  This gets much easier in the dev version of
pandoc (to be released eventually as 2.0); you can just use
a command line option --syntax-defintion=FILE.

You might try getting a binary from
pandoc-extras/pandoc-nightly, but be aware that there are
many other changes as well.
