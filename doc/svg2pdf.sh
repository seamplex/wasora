#!/bin/bash 

function callinkscape {                                                                                                                             
 inkscape -z $1 --export-pdf=`basename $1 .svg`.pdf
}

if [ -z `which inkscape` ]; then
 echo "error: inkscape not installed"
 exit 1
fi

if [ -z "$1" ]; then
  for i in *.svg; do
    callinkscape $i
  done
else
  callinkscape $1
fi

