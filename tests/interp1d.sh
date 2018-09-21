#!/bin/bash
# generate some random data (x,y) and write it into an ascii file
# then read that file and interpret the pairs as function y = f(x) and interpolate
. locateruntest.sh

rm -f f.dat f-interp.dat

# generate the data into f.dat
runwasora gendata1d.was > f.dat

# read back the data and interpolate it
runwasora interp1d.was
outcome=$?

# plot
cat << EOF > interp1d.gp
set grid
set xlabel "x"
set ylabel "f"
plot 'f.dat' pt 2 ti 'data',\
     'f-interp.dat' u 1:2 w l lw 1 lt 1 ti 'linear',\
     'f-interp.dat' u 1:3 w l lw 2 lt 7 ti 'akima',\
     'f-interp.dat' u 1:4 w l lw 2 lt 8 ti 'steffen'
EOF
plot interp1d

m4 quotes.m4 interp1d.md.m4 >> test-suite.md

# exit
exit $outcome
