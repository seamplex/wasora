#!/bin/bash
# read some known numbers from a file, send them to another instance through
# shared memory and check they were sucessfully read by the other end
# jeremy theler

# locate and include runtest.sh
. locateruntest.sh

# generate a data file with known numbers
echo 1e-1 1.23456 0.999999999999999 -9.876543210987654321e2 > data.dat

# call two instances of wasora in parallel
${wasorabin} ${testdir}io-readfile-writeshm.was > io-file.dat &
${wasorabin} ${testdir}io-readshm.was | tee io-shm.dat


# check the result
if [[ "x`diff io-file.dat io-shm.dat`" = "x" ]] && [[ "x`cat io-shm.dat`" = "x0.1 1.23456 1 -987.654" ]]; then
  outcome=0
 else
  outcome=99
fi

# exit
exit $outcome
