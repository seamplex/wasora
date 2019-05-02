
---
title: The Fibonacci sequence
inputs: fibo_vector fibo_iterative fibo_formula
tags: VECTOR SIZE PRINT_VECTOR FORMAT IF ELSE ENDIF := static_steps vecsize sqrt 
...

# The Fibonacci sequence

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
# the fibonacci sequence as a vector
VECTOR f SIZE 25

f(i)<1:2> = 1
f(i)<3:vecsize(f)> = f(i-2) + f(i-1)

PRINT_VECTOR f FORMAT %g
```

```bash
$ wasora fibo_vector.was
1
1
2
3
5
8
13
21
34
55
89
144
233
377
610
987
1597
2584
4181
6765
10946
17711
28657
46368
75025
$ 
```


## fibo_iterative.was

As in its original form the Fibonacci sequence is presented as a recurrence relation, it seems an appropriate example be used to introduce wasora's iterative computation capabilities. Three variables $f_n$, $f_{n-1}$ and $f_{n-2}$ are use to compute the recurrence. For the special case for $n=1$ and $n=2$ they are initialized to one. Then, each step consists of computing the new value for $f_n$ and shifting $f_{n-1}$ to $f_{n-2}$ first and $f_n$ to $f_{n-1}$ afterward. The first twenty five numbers are printed to the screen. As the computations are performed using double-precision floating point arithmetic, the maximum element that can be computed with this example is $n = 1476$, case that the user is encouraged to try. For $n = 1477$ a not-a-number error is issued.

```wasora
# the fibonacci sequence as an iterative problem

#static_iterations = 1476
static_steps = 25

IF step_static=1|step_static=2
 f_n = 1
 f_nminus1 = 1
 f_nminus2 = 1
ELSE
 f_n = f_nminus1 + f_nminus2
 f_nminus2 = f_nminus1
 f_nminus1 = f_n
ENDIF

PRINT %g step_static f_n 
```

```bash
$ wasora fibo_iterative.was
1	1
2	1
3	2
4	3
5	5
6	8
7	13
8	21
9	34
10	55
11	89
12	144
13	233
14	377
15	610
16	987
17	1597
18	2584
19	4181
20	6765
21	10946
22	17711
23	28657
24	46368
25	75025
$ 
```


## fibo_formula.was

The following input solves the very same problem but instead of iterative solving the recurrence relation, it uses the closed-form formula derived above. A function $f(n)$ is defined and then printed for $n=1,\dots,25$. Note that the floating point precision is enough to obtain integral values for every $n$.

```wasora
# the fibonacci sequence using the closed-form formula as a function
phi = (1+sqrt(5))/2 
f(n) := (phi^n - (1-phi)^n)/sqrt(5)
PRINT_FUNCTION f MIN 1 MAX 25 STEP 1 FORMAT %g
```

```bash
$ wasora fibo_formula.was
1	1
2	1
3	2
4	3
5	5
6	8
7	13
8	21
9	34
10	55
11	89
12	144
13	233
14	377
15	610
16	987
17	1597
18	2584
19	4181
20	6765
21	10946
22	17711
23	28657
24	46368
25	75025
$ 
```




-------
:::{.text-center}
[Previous](../005-lorenz) | [Index](../) | [Next](../007-functions)
:::
