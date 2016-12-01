#!/bin/sh
# this file should not be called directly from the shell,
# it ought to be included from within run.sh for each case
if [ -e ./wasora ]; then
 wasorabin="./wasora"
 testdir="examples/"
elif [ -e ../wasora ]; then
 wasorabin="../wasora"
 testdir=""
elif [ "`which wasora`" != "" ]; then
  wasorabin=`which wasora`
  if [ -e fibonacci.was ]; then
   testdir=""
  elif [ -e examples/fibonacci.was ]; then
   testdir="examples/"
  else
   echo "do not know where the examples are :("
  fi
else
 echo "do not know how to run wasora :("
 exit 1
fi

if [ `which gnuplot` ]; then
 hasgnuplot=1
fi

function plot {
 if [[ "x`which gnuplot`" != "x" && ! -z "$DISPLAY" ]]; then
  gnuplot -p -e "$1"
 fi
}



# checks if wasora is compiled with ida and skips the test if necessary
function checkida {
 if [ `${wasorabin} -v | grep SUNDIAL | wc -l` = 0 ]; then
  echo "wasora was not compiled with IDA, skipping test"
  exit 77
 fi
}
