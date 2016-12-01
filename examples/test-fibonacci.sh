#!/bin/bash
# compute first 15 numbers of the fibonacci series and check the result
# jeremy theler

# locate and include runtest.sh
. locateruntest.sh

# remove stale output file
fdat="fibo.dat"
rm -rf ${fdat}

# call wasora
${wasorabin} ${testdir}fibonacci.was | tee ${fdat}

# check the result
if [ "x`tail -n1 ${fdat} | tr -d \r`" = "x610" ]; then
 outcome=0
else
 outcome=99
fi

# exit
exit $outcome
