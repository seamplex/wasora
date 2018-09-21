# One-dimensional interpolation

Wasora (actually [GSL](https://www.gnu.org/software/gsl/)) provides a few different one-dimensional function interpolation schemes.
First, generate random data with `gendata1d.was` and then read it back and interpolate a function $f(x)$.

## Input files

~~~wasora
include(gendata1d.was)
~~~

~~~wasora
include(interp1d.was)
~~~

## Execution

~~~
$ wasora gendata1d.was | tee f.dat
include(f.dat)
$ wasora interp1d.was
$
~~~

![One-dimensional interpolation schemes](interp1d.svg)\
