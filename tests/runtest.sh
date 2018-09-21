#!/bin/bash
# this file should not be called directly from the shell

if [ -e ../wasora ]; then
 wasorabin="../wasora"
elif [ ! -z "`which wasora`" ]; then
  wasorabin=`which wasora`
else
 echo "do not know how to run wasora :("
 exit 1
fi

# runs wasora
function runwasora {
 $wasorabin ${1} ${2} ${3} ${4} ${5} ${6}
 outcome=${PIPESTATUS[0]}
 if [ $outcome -ne 0 ]; then
   exit $outcome
 fi
}


if [ `which gnuplot` ]; then
 hasgnuplot=1
fi

# calls gnuplot with the provided command if it is installed
function plot {
 if [ ! -z "$2" ]; then
   format=$2
 else
   format=svg
 fi
 rm -f $1.$format
 if [ "x`which gnuplot`" != "x" ]; then
   gnuplot -e "set terminal $format; set output \"$1.$format\"" $1.gp
 fi
}




# checks if wasora is compiled with ida and skips the test if necessary
function checkida {
 if [ `${wasorabin} -v | grep SUNDIAL | wc -l` = 0 ]; then
  echo "wasora was not compiled with IDA, skipping test"
  exit 77
 fi
}
