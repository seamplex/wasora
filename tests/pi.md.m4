# Computing $\pi$ in eight different ways

Did you know all these ways to compute [$\pi$](https://en.wikipedia.org/wiki/Pi)?

 1. Using the special variable `pi` which is a constant with the value of the macro `M_PI` included in the `math.h` header of the GNU\ C library.
  
 2. As four times the arc tangent of one:
 
    $$
    \pi = 4 \cdot \arctan(1)
    $$
  
 3. Finding the abscissae\ $x$ where\ $\tan(x)=0$ in the range\ $[3:3.5]$.
  
 4. As the square root of the integral between\ $-\infty$ and\ $+\infty$ of the Guassian bell:
  
    $$
    \pi = \sqrt{\int_{-\infty}^{+\infty} e^{-x^2} \, dx}
    $$
  
 5. As the area of a circle of radius one, computed as a numerical integral of a function\ $f(x,y)$ which is equal to one if\ $x^2+y^2 < 1$ and zero otherwise
   
    $$
    \pi = \int_{-1}^1 \int_{-1}^1 f(x,y) \, dx \, dy
    $$
  
 6. As the area of a circle of radius one, computed as the integral of one over a circle parametrized in terms of\ $x$ and\ $y$
   
    $$
    \pi = \int_{-1}^1 \int_{-\sqrt{1-y^2}}^{+\sqrt{1-y^2}} \, dx \, dy
    $$
  
 7. Using the [Gregory-Leibniz](https://en.wikipedia.org/wiki/Leibniz_formula_for_%CF%80) sum:
   
    $$
    \pi =  4 \cdot \sum_{i=1}^{\infty} \frac{(-1)^{i+1}}{2i-1}
    $$
  
 8. Using the [Abraham-Sharp](http://mathworld.wolfram.com/PiFormulas.html) sum:
   
    $$
    \pi =  \sum_{i=0}^{\infty} \frac{2 \cdot (-1)^i \cdot 3^{\left(\frac{1}{2}-i\right)}}{2i+1}
    $$

## Input file

~~~wasora
include(pi.was)
~~~

## Execution

~~~
$ wasora pi.was
include(pi.txt)
$
~~~


## Explanation

A vector `piapprox` of size\ 8 is defined (the name `pi` is already reserved for the special variable which contains the double-precision representation of\ $\pi$). The elements are assigned according to the eight ways of estimating\ $\pi$ as requested. The elements of the vector and its difference with the scalar\ `pi` are printed with the instruction `PRINT_VECTOR`. The format is set to twenty decimal places with a dummy holder for the eventual negative sign (note the quotation marks so the whole string `% .20f` is recognized as a single token by wasora).

Of course the first two elements are equal to\ `pi`. The root of\ $\tan(x)$ is as close to\ $\pi$ as the precision of the numerical root-finding procedure used by wasora (i.e. the algorithms provided by the GSL). An optional fifth argument can be provided to the functional `root` which sets the relative tolerance for the method. The default is good enough for most computations though. The integral of\ $\exp(-x^2)$ is done numerically with an adaptive method. Instead of passing the special variable `infinite` as the integral limits, it is best to use a number\ $a$ such that\ $\exp(-a^2) \approx 0$ so the intervals where the most useful part of the function are effectively utilized by the algorithm (also provided by the GSL). The fifth element comes from a numerical integral which “counts” how many integration points from the unit square “fall” into the unit circle, whilst the six elements “stretches” the integration points into the circle and not allowing them to “get out” of it. Therefore, as expected, the latter is closer to\ $\pi$ than the former. The last two infinite sums are of course translated into finite sums. The first is simpler but a lot of terms are needed in order to obtain a good value. The last one is more complex but only a few terms are enough to obtain a good value of\ $\pi$. 

