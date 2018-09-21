#!/bin/bash
# compute first 15 numbers of the fibonacci series and check the result
. locateruntest.sh

./pre.sh

# remove stale output file
output="fibonacci.txt"
rm -rf ${output}

# call wasora
runwasora fibonacci.was | tee ${output}

# check the result
if [ "x`tail -n1 ${output} | tr -d \r`" = "x610" ]; then
 outcome=0
else
 outcome=99
fi

m4 quotes.m4 fibonacci.md.m4 >> test-suite.md

# exit
exit $outcome
