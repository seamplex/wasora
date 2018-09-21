#!/bin/bash
# compute pi in eight different ways
. locateruntest.sh
checkida

# remove stale output file
output="lorenz.dat"
rm -rf ${output}

runwasora lorenz.was > lorenz.dat
outcome=$?

# plot
cat << EOF > lorenz.gp
set ticslevel 0
set xlabel "x"
set ylabel "y"
set zlabel "z"
splot 'lorenz.dat' u 2:3:4 w l lt 2 ti ''
EOF
plot lorenz

m4 quotes.m4 lorenz.md.m4 >> test-suite.md

exit $outcome
