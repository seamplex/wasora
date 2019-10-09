define(case_title, A simple differential equation)
---
title: case_title
inputs: exp exp-dt
tags: PHASE_SPACE PRINT HEADER = .= end_time min_dt max_dt rel_error
...

case_nav


# case_title


The examples in this section show how a single ordinary differential equation can be solved with wasora. Indeed this is one of its main features, namely the ability to solve systems of differential-algebraic equations written as natural algebraic expressions. In particular, the equation the examples solve is

$$ \frac{dx}{dt} = -x $$


with the initial condition $x_0 = 1$, which has the trivial analytical solution $x(t) = e^{-t}$.


## exp.was

As clearly defined in wasora’s design basis, simple problems ought to be solved by means of simple inputs. Here is a solid example of this behavior.

```wasora
include(exp.was)dnl
```

```bash
include(exp.term)dnl
```

![Output of `exp.was`](exp.svg)


By default, wasora adjusts the time step so an estimation of the relative numerical error is bounded within a range given by the variable `rel_error`, which has an educated guess by default. It can be seen in the figure that `dt` starts with small values and grows as the conditions allow it.

## exp-dt.was

The time steps wasora take can be limited by means of the special variables `min_dt` and `max_dt`. If they both are zero (as they are by default), wasora is free to choose `dt` as it considers appropriate. If `max_dt` is non-zero, `dt` will be bounded even if the conditions are such that bigger time steps would not introduce large errors. On the other hand, if `min_dt` is non-zero, the time step is guaranteed not to be smaller that the specified value. However, it should be noted that wasora may need to take several internal times step to keep the error bounded. In the limiting case where `min_dt = max_dt`, the time step can be set exactly although, again, wasora may take internal steps.

This situation is illustrated with the following input, which is run for five combinations of `min_dt` and `max_dt`.

```wasora
include(exp-dt.was)dnl
```

```bash
include(exp-dt.term)dnl
```

![Output of `exp.was`](exp-dt.svg)

![Errors with respect to the analytical solution](exp-error.svg)


It can be seen that all the solutions coincide with the analytical expression. Even if the time step is set to a big fixed value, the error commited by the numerical solver with respect to the exact solution is the same as wasora iterates internally as needed. In general, the fastest condition is where `dt` is not bounded as wasora minimizes iterations by automatically adjusting its value. However, it is clear that controlling the time step can be useful some times. A further control can be obtained by means of the `TIME_PATH` keyword.


case_nav
