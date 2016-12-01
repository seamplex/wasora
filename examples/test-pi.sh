#!/bin/bash
# compute pi in eight different ways
# jeremy theler

# locate and include runtest.sh
. locateruntest.sh

# run wasora and compute the sum of the errors with awk
${wasorabin} ${testdir}pi.was | awk '{print $0;err+=($2>5e-4)}END{exit err}'
outcome=$?

# exit
exit $outcome
