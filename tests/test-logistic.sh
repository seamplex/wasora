#!/bin/bash
# build the logistic map for a window of r that shows chaotic behaviour
# jeremy theler

# locate and include runtest.sh
. locateruntest.sh

${wasorabin} ${testdir}logistic.was > logistic.dat
outcome=$?

plot "plot 'logistic.dat' ps 0.1 pt 5 lt 3 ti ''"

exit $outcome
