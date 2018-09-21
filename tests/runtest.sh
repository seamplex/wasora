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
