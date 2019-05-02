define(case_title, The Fibonacci sequence)
---
title: case_title
inputs: fibo_vector fibo_iterative fibo_formula
tags: VECTOR SIZE PRINT_VECTOR FORMAT IF ELSE ENDIF := static_steps vecsize sqrt 
...

# case_title

A thorough discussion of the beauty and applications of the Fibonacci sequence is of course out of both this Real Book and its author league. For sure the reader is already familiar with this set of natural numbers. The three examples that follow show how to use wasora to compute the Fibonacci numbers, first by constructing a vector of a certain size and setting each element to be equal to the sum of the two previous elements, then iteratively computing

$$ f_{n} = f_{n-1} + f_{n-2} $$

and then by using the dazzling closed-form formula

$$ f_{n} = \frac{\phi^n + (1-\phi)^n}{\sqrt{5}} $$

where $\phi= (1+\sqrt{5})/2$ is the golden ratio. As cumbersome as it may seem, this formula is easily obtained by solving the explicit difference equation, for example with the aid of the $Z$-transform:

$$ f_{n} = z f_{n} + z^2 f_{n} $$

$$ z^2 + z - 1 = 0 $$

$$ z = \frac{1 \pm \sqrt{5}}{2} $$

The general solution is therefore

$$ f_n = k_1 \left( \frac{1+\sqrt{5}}{2} \right)^n + k_2 \left(\frac{1-\sqrt{5}}{2} \right)^n $$

where $k_1$ and $k_2$ should be determined as to fulfill the initial conditions $f_1 = 1$ and $f_2=1$. The solution is

$$
\begin{align*}
 k_1 &= +\frac{1}{\sqrt{5}}\\
 k_2&= -\frac{1}{\sqrt{5}}
\end{align*}
$$

and the closed-form formula follows.


## fibo_vector.was

The first $N$ elements of the Fibonacci sequence can be computed within a vector $f$ of a size $N$ as the following simple wasora input shows. The first two elements $f_i$ for $i=1$ and $i=2$ are set to one and then the next $N-2$ elements are computed as $f_i = f_{i-1} + f_{i-2}$.

```wasora
include(fibo_vector.was)dnl
```

```bash
include(fibo_vector.term)dnl
```


## fibo_iterative.was

As in its original form the Fibonacci sequence is presented as a recurrence relation, it seems an appropriate example be used to introduce wasora's iterative computation capabilities. Three variables $f_n$, $f_{n-1}$ and $f_{n-2}$ are use to compute the recurrence. For the special case for $n=1$ and $n=2$ they are initialized to one. Then, each step consists of computing the new value for $f_n$ and shifting $f_{n-1}$ to $f_{n-2}$ first and $f_n$ to $f_{n-1}$ afterward. The first twenty five numbers are printed to the screen. As the computations are performed using double-precision floating point arithmetic, the maximum element that can be computed with this example is $n = 1476$, case that the user is encouraged to try. For $n = 1477$ a not-a-number error is issued.

```wasora
include(fibo_iterative.was)dnl
```

```bash
include(fibo_iterative.term)dnl
```


## fibo_formula.was

The following input solves the very same problem but instead of iterative solving the recurrence relation, it uses the closed-form formula derived above. A function $f(n)$ is defined and then printed for $n=1,\dots,25$. Note that the floating point precision is enough to obtain integral values for every $n$.

```wasora
include(fibo_formula.was)dnl
```

```bash
include(fibo_formula.term)dnl
```




case_nav
