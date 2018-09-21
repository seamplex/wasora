#!/bin/bash
# compute pi in eight different ways
. locateruntest.sh

# remove stale output file
output="pi.txt"
rm -rf ${output}

# run wasora and compute the sum of the errors with awk
runwasora pi.was | tee ${output} 
awk '{err+=($2>5e-4)}END{exit err}' ${output}
outcome=$?

m4 quotes.m4 pi.md.m4 >> test-suite.md

# exit
exit $outcome
