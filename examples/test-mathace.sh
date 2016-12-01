#!/bin/bash
# solve knuth's mystery equation using a 2d quasi-random sequence
# jeremy theler

. locateruntest.sh

${wasorabin} ${testdir}mathace.was > mathace.dat
outcome=$?

plot "plot 'mathace.dat' ps 0.1 pt 5 ti ''"
exit $outcome
