#!/bin/bash
# compute pi in eight different ways
# jeremy theler

# locate and include runtest.sh
. locateruntest.sh

# remove stale output file
output="pi.txt"
rm -rf ${output}

# run wasora and compute the sum of the errors with awk
runwasora pi.was | tee ${output} 
awk '{err+=($2>5e-4)}END{exit err}' ${output}
outcome=$?

cat << EOF >> test-suite.md
# Compute $\pi$ in eight different ways

Did you know all these ways to compute [$\pi$](https://en.wikipedia.org/wiki/Pi)?

~~~wasora
`cat pi.was`
~~~

~~~
$ wasora pi.was
`cat ${output}`
$
~~~

EOF

# exit
exit $outcome
