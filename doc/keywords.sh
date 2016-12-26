#!/bin/bash

src="../src"
parsers="../src/parser.c ../src/mesh/parser.c"
inits="../src/init.c ../src/mesh/init.c"
for plugin in milonga fino besssugo xdfrrpf; do
  if [ -e ../../${plugin}/src/parser.c ]; then
    parsers="${parsers} ../../${plugin}/src/parser.c"
  fi 
  if [ -e ../../${plugin}/init.c ]; then
    inits="${inits} ../../${plugin}/src/init.c"
  fi 
done


UPPER1=`grep strcasecmp ${parsers} | sed -r 's/[^"]*("[^"]*")?/ \1/g;s/" +"/\n/g;s/ *"//g'| sort | mawk '$1 in p{next}{p[$1];print}' | mawk '/^[A-Z]/'`
UPPER2=`grep keywords   ${parsers} | sed -r 's/[^"]*("[^"]*")?/ \1/g;s/" +"/\n/g;s/ *"//g'| sort | mawk '$1 in p{next}{p[$1];print}' | mawk '/^[A-Z]/'`
LOWER=`grep strcasecmp  ${parsers} | sed -r 's/[^"]*("[^"]*")?/ \1/g;s/" +"/\n/g;s/ *"//g'| sort | mawk '$1 in p{next}{p[$1];print}' | mawk '/^[a-z]/'`
VARS=`grep variable ${inits}       | grep -v "computing" | sed -r 's/[^"]*("[^"]*")?/ \1/g;s/" +"/\n/g;s/ *"//g'| sort | mawk '$1 in p{next}{p[$1];print}'`
FUNCS=`cat ${src}/builtin.h        | sed -r 's/[^"]*("[^"]*")?/ \1/g;s/" +"/\n/g;s/ *"//g'| sort | mawk '$1 in p{next}{p[$1];print}'`
UPPER="${UPPER1} ${UPPER2}"

# echo $UPPER
# 
# 
# 
# PARSERS="../../src/parser.c"
# INITS="../../src/init.c"
# EXTENSIONS="*.was"
# 
# 
# UPPER=`grep strcasecmp $PARSERS | sed -r 's/[^"]*("[^"]*")?/ \1/g;s/" +"/\n/g;s/ *"//g'| mawk '$1 in p{next}{p[$1];print}' | mawk '/^[A-Z]/' | sort`
# LOWER=`grep strcasecmp $PARSERS | sed -r 's/[^"]*("[^"]*")?/ \1/g;s/" +"/\n/g;s/ *"//g'| mawk '$1 in p{next}{p[$1];print}' | mawk '/^[a-z]/' | sort`
# VARS=`grep variable $INITS | sed -r 's/[^"]*("[^"]*")?/ \1/g;s/" +"/\n/g;s/ *"//g'| mawk '$1 in p{next}{p[$1];print}' | sort`
# FUNCS=`cat ../src/builtin.h | sed -r 's/[^"]*("[^"]*")?/ \1/g;s/" +"/\n/g;s/ *"//g'| mawk '$1 in p{next}{p[$1];print}' | sort`
# 
# # echo $UPPER
# # echo $LOWER
# # echo $EXTENSIONS

# grep strcasecmp src/framework/commonparser.c | sed -r 's/[^"]*("[^"]*")?/ \1/g;s/" +"/\n/g;s/ *"//g'| mawk '$1 in p{next}{for (pnt="";p[pnt] < $ 1 && p[pnt];) pnt=p[pnt]; p[$1]=p[pnt]; p[pnt]=$1} END{for(pnt="";pnt=p[pnt];)print pnt}'
