#!/bin/bash

style=pygments


if [ ! -z "`which pandoc`" ]; then
  pandoc -s test-suite.md  \
    --template=template.html \
    --highlight-style=${style} \
    --number-sections \
    --table-of-contents \
    --syntax-definition=../doc/wasora.xml \
    --katex=css/ \
    --css=css/normalize.css \
    --css=css/skeleton.css \
    -o test-suite.html
#   xdg-open test-suite.html
fi
