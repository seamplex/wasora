#!/bin/sh
# this file should not be called directly from the shell,
# it ought to be included from within run.sh for each case
if [ -e ./wasora ]; then
 wasorabin="./wasora"
 testdir="examples/"
elif [ -e ./wasora.exe ]; then
 if [ "x`which wine`" = "x" ]; then
   wasorabin="./wasora.exe"
 else
   wasorabin="wine ./wasora.exe"
 fi
 testdir="examples/"
elif [ -e ../wasora ]; then
 wasorabin="../wasora"
 testdir=""
elif [ -e ../wasora.exe ]; then
 if [ "x`which wine`" = "x" ]; then
   wasorabin="../wasora.exe"
 else
   wasorabin="wine ../wasora.exe"
 fi
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

if [ ! -z "`which dos2unix`" ]; then
 trim="dos2unix"
elif [ ! -z "`which d2u`" ]; then
 trim="d2u"
else
 trim="cat"
fi

if [ `which gnuplot` ]; then
 hasgnuplot=1
fi


# calls gnuplot with the provided command if it is installed
# function plot {
#  if [ "x`which gnuplot`" != "x" ]; then
#   if [ "x`uname | cut -c-6`" = "xCYGWIN" ]; then
#    if [ "x`ps -e | grep X | wc -l`" = "x0" ]; then
#     XWin.exe -multiwindow -clipboard -silent-dup-error > /dev/null &
#     sleep 2
#    fi
#    export DISPLAY=:0.0
#   fi
#   gnuplot -p -e "$1"
#  fi
# }
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
