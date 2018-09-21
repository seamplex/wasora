#!/bin/bash
# build an histogram out of actual experimental data
# (300 measures of the period of a simple pendulum taken back in 2004)
# jeremy theler

# locate and include runtest.sh
. locateruntest.sh

# call wasora
${wasorabin} ${testdir}histogram.was histogram-samples 300 15 2.25 2.55 | tee histogram.dat
outcome=$?

# call the plot function
plot "plot 'histogram.dat' with histeps"

# exit
exit $outcome
