define(case_title, One-dimensional functions)
---
title: case_title
inputs: algebraic inline file vectors satwater
tags: FUNCTION PRINT_FUNCTION MIN MAX STEP VAR FILE FILE_PATH VECTORS NUMBER OUTPUT_FILE INTERPOLATION cspline akima linear sin cos log sqrt derivative integral
...

# case_title

This example shows how functions can be defined and used in wasora.



## algebraic.was

Two algebraic functions $f(x)$ and $g(x)$ are defined and afterward printed and plotted using [qdp](https://bitbucket.org/seamplex/qdp), along with the integral of their product.

```wasora
include(algebraic.was)dnl
```

```bash
include(algebraic.term)dnl
```

![Output of `algebraic.was`](algebraic-dat.svg)

## inline.was

Now, another function called\ $f(x)$ is defined by a set of scattered points. By default, wasora interpolates the data with linear functions. If `PRINT_FUNCTION` does not get a range, it prints the definition points. Otherwise, the function is interpolated at the corresponding points. Finally,\ $f(x)$ can be evaluated at any value of\ $x$.

```wasora
include(inline.was)dnl
```

```bash
include(inline.term)dnl
```

![Output of `inline.was`](inline.svg)


## file.was

This example shows three different methods of one-dimensional interpolation provided by wasora (indeed, by the [GNU Scientific Library](https://www.gnu.org/software/gsl/)). The data is read from a file, so the three functions use the same data set but are interpolated  in different ways. Recall that whenever a one-dimensional point-wise function called `f` is defined, three new variables are also defined:

 1. `f_a` contains the first point-wise value of the independent variable
 2. `f_b` contains the last point-wise value of the independent variable
 3. `f_n` contains the number of points of definition

```wasora
include(file.was)dnl
```

```bash
include(file.term)dnl
```

![Output of `file.was`](file-dat.svg)


## vectors.was

The same data set is now used to define the same three functions $f(x)$, $g(x)$ and $h(x)$ as before but using vectors instead of files. The usage of the `derivative` and `integral` functionals to construct new functions of $x$ by means of an intermediate variable~$x'$ is ilustrated. Also, note that the size of the vector `datax` is defined as a constant `NUMBER`. Refer to the documentation of the `VECTOR` keyword for further information.


```wasora
include(vectors.was)dnl
```

```bash
include(vectors.term)dnl
```

![Derivatives](derivatives-dat.svg)

![Integrals](integrals-dat.svg)


## satwater.was

This example builds functions of saturated water properties by reading a file, which can be freely downloaded from [NIST](http://webbook.nist.gov/chemistry/fluid/), containing several columns with data. The terminal shows the actual content of the file. The first two columns contain the saturation temperature and pressure, and the third one contains the liquid density. First, saturation pressure vs. temperature and the inverse saturation temperature vs. pressure functions are constructed. Also functions of the saturated liquid density as a function of the temperature and of the pressure are constructed by selecting the appropriate columns as the independent and dependent variable. Finally, the partial derivatives of the density with respect to temperature and pressure are defined as continuous functions, with the derivative functional acting on the akima-interpolated data set. Three out of the many possible curves are plotted in this example, giving the optional range to explicitly show how the interpolation works. The original points are also plotted for comparison. More columns could have also been retained from the original data set obtained from [NIST](http://webbook.nist.gov/chemistry/fluid/).

```wasora
include(satwater.was)dnl
```

```bash
include(satwater.term)dnl
```

![Saturation pressure](psat.svg)

![Saturation temperature](Tsat.svg)

![Derivative of density with respect to temperature $\partial \rho/\partial T$](drhodt.svg)

case_nav
