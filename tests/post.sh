#!/bin/bash

style=pygments

# https://cdn.jsdelivr.net/npm/katex@0.10.0-rc.1/dist/
# https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.5/MathJax.js?config=TeX-AMS_CHTML-full 

if [ ! -z "`which pandoc`" ]; then
  pandoc -s test-suite.md  \
    --template=template.html \
    --highlight-style=${style} \
    --number-sections \
    --table-of-contents \
    --toc-depth=1 \
    --syntax-definition=../doc/wasora.xml \
    --mathjax=https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.5/MathJax.js?config=TeX-AMS_CHTML-full  \
    --css=css/normalize.css \
    --css=css/skeleton.css \
    -o test-suite.html
#   xdg-open test-suite.html
fi
