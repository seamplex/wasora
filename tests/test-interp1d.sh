#!/bin/bash
# generate some random data (x,y) and write it into an ascii file
# then read that file and interpret the pairs as function y = f(x) and interpolate
# jeremy theler

# locate and include runtest.sh
. locateruntest.sh

# generate the data into f.dat
${wasorabin} ${testdir}gendata1d.was > f.dat

# read back the data and interpolate it
${wasorabin} ${testdir}interp1d.was
outcome=$?

# call the plot function
plot "plot 'f.dat' pt 2 ti 'data', 'f-interp.dat' w l lt 3 ti 'linear', 'f-interp.dat' u 1:3 w l lw 3 lt 7 ti 'akima', 'f-interp.dat' u 1:4 w l lw 2 lt 8 ti 'steffen'"

# exit
exit $outcome
