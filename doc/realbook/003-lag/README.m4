define(case_title, First-order lags)
...
title: case_title
inputs: lag compact quasi-sine
tags: end_time dt lag HEADER
---

# case_title

Whenever the special variable `end_time` is non-zero, wasora enters into transient mode. These examples introduce transient problems by illustrating how first-order lags can be used to filter signals.


## lag.was

This example generates a signal $r(t)$ which is zero except for $a < t < b$, where it takes the value one. Then, the signal $y(t)$  is computed as a first-order lag of $r(t)$ with a characteristic time $\tau$. The output consists of three columns containing $t$, $r(t)$ and $y(t)$. By using the keyword `HEADER` a commented line is pre-prended to the output with a textual representation of the expressions passed to `PRINT` which are automatically understood by "qdp": "https://bitbucket.org/gtheler/qdp" so a reasonable figure can be obtained with virtually no effort, as shown in terminal mimic where the output of wasora is piped to `qdp`.

```wasora
include(lag.was)dnl
```

```bash
include(lag.term)dnl
```

![Output of `lag.was`](lag.svg)

## compact.was

Instead of writing the long input shown in `lag.was`, we could have obtained the same result with a couple of lines. Indeed, the terminal shows that the output of this input is the same as the one of the previous longer example.

```wasora
include(compact.was)dnl
```

```bash
include(compact.term)dnl
```

The reported difference is due to the presence of the `HEADER` keyword in the first input so [qdp](https://bitbucket.org/gtheler/qdp) can automatically label the bullets.
kate 

## quasi-sine.was

Not only does this example illustrate the usage of a first-order lag, but also of a point-wise defined function\ $s(t)$ (more on one-dimensional functions in case\ [007-functions](../007-functions)). In this case, the data is interpolated using the Akima method, and `end_time` is set to the variable `s_b` which contains the last value of the one-dimensional function `s` (incidentally, `s_a` contains the first value). 

```wasora
include(quasi-sine.was)dnl
```

```bash
include(quasi-sine.term)dnl
```

![Output of `quasi-sine.was`](quasi-sine.svg)

case_nav
