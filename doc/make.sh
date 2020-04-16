#!/bin/bash

if [ -z "`which pandoc`" ]; then 
 echo "error: pandoc not installed"
 exit 1
fi

# create the reference markdown from the commented sources
# including a TOC
touch reference-toc.md
m4 reference.m4 > reference.md
pandoc reference.md --toc --template=toc.template -o reference-toc.md
m4 reference.m4 > reference.md

# the reference for the manual is slightly different due to texinfo
m4 header.m4 reference-manual.m4 > reference-manual.md

# just for completeness
# pandoc help.md -t plain > help.txt


# manpage
# m4 header.m4 wasora.1.md | pandoc -s -t man -o wasora.1


# manual
m4 header.m4 wasora.md | sed 's/text{/r{/' | \
  pandoc --toc --template template.texi \
         --filter pandoc-crossref -o wasora.texi
sed -i 's/@verbatim/@smallformat\n@verbatim/' wasora.texi
sed -i 's/@end verbatim/@end verbatim\n@end smallformat/' wasora.texi         

