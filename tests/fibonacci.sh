#!/bin/bash
# compute first 15 numbers of the fibonacci series and check the result
. locateruntest.sh

./pre.sh

# remove stale output file
output="fibo.dat"
rm -rf ${output}

# call wasora
runwasora fibonacci.was | tee ${output}

# check the result
if [ "x`tail -n1 ${output} | tr -d \r`" = "x610" ]; then
 outcome=0
else
 outcome=99
fi

cat << EOF >> test-suite.md
# The Fibonacci sequence

Create a vector of size 15 with the first elements of the [Fibonacci numbers](https://en.wikipedia.org/wiki/Fibonacci_number) with the following input file:

~~~wasora
`cat fibonacci.was`
~~~

And run it like this:

~~~
$ wasora fibonacci.was
`cat ${output}`
$
~~~

EOF

# exit
exit $outcome
