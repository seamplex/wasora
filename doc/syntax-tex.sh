#!/bin/sh
# 
# $ ./syntax-tex.sh > ../syntax.tex
# 

. ./keywords.sh

# primary keywords
echo "\lstdefinelanguage{wasora}{"
echo "morekeywords={"
for kw in $UPPER; do
  echo "      $kw,"
done
echo "},"

# secondary keywords (TO-DO)
echo "morekeywords={[2]"
echo "},"

# special variables
echo "morekeywords={[3]"
for kw in $VARS; do
  echo "      $kw,"
  echo -n "      $kw"
  echo "_0,"
done
echo "},"

# functions
echo "morekeywords={[4]"
for kw in $FUNCS; do
  echo "      $kw,"
done
echo "},"

cat << EOF
sensitive=true,
morecomment=[l]{\#},
morestring=[b]\",
}
EOF
