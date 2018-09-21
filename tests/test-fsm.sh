#!/bin/bash
# fit the semi-empirical weiszacker's formula to experimental data
# jeremy theler

. locateruntest.sh

${wasorabin} ${testdir}fsm.was > fsm.dat
outcome=$?

plot "set cbrange [0:9]; set view map; set xlabel 'A'; set ylabel 'Z'; splot 'fsm.dat' u 1:2:4 w p pt 57 palette ti ''"

exit $outcome
