#!/bin/bash
# build the famous butterfly-shaped lorenz attractor
# jeremy theler

. locateruntest.sh
checkida

${wasorabin} ${testdir}lorenz.was > lorenz.dat
outcome=$?

plot "splot 'lorenz.dat' u 2:3:4 w l lt 2 ti ''"

exit $outcome
