define(case_title, Two-dimensional functions)
---
title: case_title
inputs: paraboloid nearest rectangle scattered compwater
tags: FUNCTION := FILE_PATH DATA INTERPOLATION nearest rectangle PRINT_FUNCTION MIN MAX STEP NUMBER FILE OUTPUT_FILE 
...

case_nav

# case_title


These examples illustrate the facilities wasora provides to interpolate two-dimensional functions. As shown in section [007-functions], multidimensional functions may be defined using algebraic expressions, inline data, external files, vectors or dynamically-loaded routines. The main focus of this section is to illustrate the difference between algebraic and pointwise-defined two-dimensional functions, and to further illustrate the difference between a nearest-neighbor interpolation (based on a $k$-dimensional tree structure) and a rectangle interpolation (based on finite-elements-like shape functions for quadrangles).

## paraboloid.was

This input defines an algebraic function $f(x,y)$ and uses `PRINT_FUNCTION` to dump its contents (as three columns containing $x$, $y$ and $f(x,y)$ as shown in the terminal mimic) to the standard output. The range is mandatory because $f(x,y)$ is defined by an algebraic expression.

As far as the function $f(x,y)$ is concerned, $a$ and $b$ are taken as parameters. Even though they can change over time, the value they take is the value they have when $f(x,y)$ is evaluated. In this case, $f(x,y)$ is evaluated when the instruction `PRINT_FUNCTION` is executed, so the output is written with $b=2$.

```wasora
include(paraboloid.was)dnl
```

```bash
include(paraboloid.term)dnl
```

![](paraboloid.svg)

![](paraboloid2d.svg)

## nearest.was

This input defines a two-dimensional pointwise-defined function $g(x,y)$ using data inlined in the input file using the `DATA` keyword. As can be seen, expressions are allowed. However, they are evaluated at parse-time so references to variables should be avoided, as they will default to zero. For that end, numbers defined with the `NUMBER` keyword should be used. By default, pointwise-defined multidimensional functions are interpolated by the nearest neighbor algorithm (i.e. default is `INTERPOLATION nearest`). When calling `PRINT_FUNCTION` with no range, the definition points are printed. When a range is given, the function gets evaluated at the grid points.

In this case, first the function $g(x,y)$ is dumped into a file called `g_def.dat` with no range. Then, the function is dumped into a file called `g_int.dat` using a range and thus resulting in an interpolated output using nearest neighbors.

```wasora
include(nearest.was)dnl
```

```bash
include(nearest.term)dnl
```

![](nearest.svg)

![](nearest2d.svg)


## rectangle.was

This time, a function $h(x,y)$ is defined by reading point-wise data from a file. This file is `g_def.dat` which are the definition points of the function $g(x,y)$ of the previous example. Note that when reading function data from a file, no expressions are allowed. Function $h(x,y)$ is interpolated using the `rectangle` method. The interpolated data is written in a file called `h_int.dat`, in which the function $h(x,y)$ is evaluated at the very same points the function $g(x,y)$ of the previous example was.

```wasora
include(rectangle.was)dnl
```

```bash
include(rectangle.term)dnl
```

![](rectangle.svg)

![](rectangle2d.svg)

## scattered.was

The last example of two-dimensional interpolation involves a pointwise-defined function~$s(x,y)$ using scattered data, i.e. not necessarily over a rectangular grid. In this case---and with no information about any underlying finite-element-like mesh---wasora can use either a nearest-neighbor interpolation or a Shepard-like inverse distance weighting. For the original Shepard method, the only parameter than can be tweaked is the exponent~$p$ of the distance in the weight~$w_i=1/d_i^p$. For the modified Shepard algorithm, the radius~$r$ of the nearest neighbors taken into account is to be provided. These nearest neighbors are found using a $k$-dimensional tree, that is a very efficient way of doing this task. For complex functions, all the alternatives should be investigated taking into account accuracy and code speed. 

```wasora
include(scattered.was)dnl
```

```bash
include(scattered.term)dnl
```

![](scattered1a.svg)

![](scattered2a.svg)

![](scattered3a.svg)

![](scattered1b.svg)

![](scattered2b.svg)

![](scattered3b.svg)

![](scattered1c.svg)

![](scattered2c.svg)

![](scattered3c.svg)

![](scattered2d1.svg)

![](scattered2d2.svg)

![](scattered2d3.svg)

The figures illustrate how the multidimensional data interpolation scheme work for pointwise defined functions over scattered data. Nearest neighbors give constant values for each voronoi triangle whilst Shepard-based algorithms provide continuous surfaces.


## compwater.was

This example shows an extension of the example about saturated water in section ref{007-functions} by giving properties of compressed water as a function of pressure $p$ and temperature $T$. The file `compwater.txt` contains some properties of water as a function of temperature and pressure in a rectangular grid over the pressure-temperature space. The enthalpy is not contained in the file, but it can be computed from pressure $p$, the internal energy $u(p,T)$ and the specific volume $v(p,T)$ as

$$
h(p,t) = u(p,T) + p \cdot v(p,T)
$$



```wasora
include(compwater.was)dnl
```

```bash
include(compwater.term)dnl
```

![](compwater.svg)

The figure shows the enthalpy of compressed water as a continuous function of $p$ and $T$ and the discrete experimental data.


case_nav
