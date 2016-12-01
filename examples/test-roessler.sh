#!/bin/bash
# solve the rossler chaotic system in one wasora instance and
# send the data through shared memory to another that plots
# the result (but knows nothing about the equations)
# jeremy theler

. locateruntest.sh
checkida

# call two instances of wasora
${wasorabin} --no-debug ${testdir}roessler-sender.was &
${wasorabin} --no-debug ${testdir}roessler-receiver.was > roessler.dat

# check the result
if [ "x`head -n2 roessler.dat | tail -n1 | awk '{print $4}' | ${trim}`" = "x3.123456" ]; then
 outcome=0
else
 outcome=99
fi

plot "splot 'roessler.dat' u 2:3:4 w l lt 1 ti ''"

exit $outcome
