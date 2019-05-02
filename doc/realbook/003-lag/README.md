
...
title: First-order lags
inputs: lag compact quasi-sine
tags: end_time dt lag HEADER
---

# First-order lags

Whenever the special variable `end_time` is non-zero, wasora enters into transient mode. These examples introduce transient problems by illustrating how first-order lags can be used to filter signals.


## lag.was

This example generates a signal $r(t)$ which is zero except for $a < t < b$, where it takes the value one. Then, the signal $y(t)$  is computed as a first-order lag of $r(t)$ with a characteristic time $\tau$. The output consists of three columns containing $t$, $r(t)$ and $y(t)$. By using the keyword `HEADER` a commented line is pre-prended to the output with a textual representation of the expressions passed to `PRINT` which are automatically understood by "qdp": "https://bitbucket.org/gtheler/qdp" so a reasonable figure can be obtained with virtually no effort, as shown in terminal mimic where the output of wasora is piped to `qdp`.

```wasora
# this is a transient problem and lasts 5 units of time
end_time = 5
# each time step is equal to 1/20th of a unit of time
dt = 1/20

# some parameters, which we define as constants
CONST a b tau
a = 1
b = 3
tau = 1.234

# signal r is equal to zero except for a < t < b
r = 0
r[a:b] = 1

# signal y is equal to signal r fitered through a lag
# of characteristic time tau
y = lag(r, tau)

PRINT t r y

# exercise: investigate how the result of the lag
# depends on the time step
```

```bash
$ wasora lag.was | qdp -o lag
$ 
```

![Output of `lag.was`](lag.svg)

## compact.was

Instead of writing the long input shown in `lag.was`, we could have obtained the same result with a couple of lines. Indeed, the terminal shows that the output of this input is the same as the one of the previous longer example.

```wasora
# the preceeding example could have been written in fewer
# lines as follows (although the SPOT rule is broken)
end_time = 5
dt = 1/20
PRINT t heaviside(t-1)-heaviside(t-3) lag(heaviside(t-1)-heaviside(t-3),1.234)
```

```bash
$ wasora lag.was > lag.dat
$ wasora compact.was > compact.dat
$ diff -s lag.dat compact.dat
Files lag.dat and compact.dat are identical
$ 
```

The reported difference is due to the presence of the `HEADER` keyword in the first input so [qdp](https://bitbucket.org/gtheler/qdp) can automatically label the bullets.
kate 

## quasi-sine.was

Not only does this example illustrate the usage of a first-order lag, but also of a point-wise defined function\ $s(t)$ (more on one-dimensional functions in case\ [007-functions](../007-functions)). In this case, the data is interpolated using the Akima method, and `end_time` is set to the variable `s_b` which contains the last value of the one-dimensional function `s` (incidentally, `s_a` contains the first value). 

```wasora
FUNCTION s(t) INTERPOLATION akima DATA {
0     1
1     1+0.25
2     1-0.25
3     1+0.5
4     1-0.5
5     1+0.75
6     1-0.75
7     1+0.75
8     1-0.75
9     1+0.9
10    1-0.9
12    1
15    1
18    1    
20    1  
}

end_time = s_b

PRINT t s(t) lag(s(t),1) HEADER
```

```bash
$ wasora quasi-sine.was | qdp -o quasi-sine
$ 
```

![Output of `quasi-sine.was`](quasi-sine.svg)

-------
:::{.text-center}
[Previous](../002-expressions) | [Index](../) | [Next](../004-exp)
:::
