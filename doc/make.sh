# pandoc --template=template.html --mathjax --toc --smart --metadata=title:wasora ../README.md -o html/README.html
# pandoc --template=template.html --mathjax --toc --smart FAQ.md -o html/FAQ.html
# TODO citations
# pandoc --template=template.html --mathjax --toc --smart --number-sections description.md -o html/description.html
# pandoc --template=template.html --mathjax --toc --smart --number-sections realbook.md -o html/realbook.html

# pandoc --template=template.texi wasora.md -o wasora.texi


# cd ../examples
# ./showcase-html.sh > ../doc/showcase.md
# cd ../doc
# 
# pandoc --template=template.html --mathjax --toc --smart showcase.md -o html/showcase.html
# cp ../examples/lorenz.svg html

m4 reference.m4 > reference.md
