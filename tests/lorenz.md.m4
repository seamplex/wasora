# The Lorenz attractor

[Edward Lorenz](https://en.wikipedia.org/wiki/Edward_Norton_Lorenz) introduced the nowadays-famous [dynamical system](https://en.wikipedia.org/wiki/Lorenz_system) in his seminal 1963 paper [Deterministic Nonperiodic Flow](http://dx.doi.org/10.1175/1520-0469(1963)020<0130:DNF>2.0.CO;2)
  
\begin{cases}
\dot{x} &= \sigma \  (y - x) \\
\dot{y} &= x \  (r - z) - y \\
\dot{z} &= xy - bz
\end{cases}
  
Wasora can be used to solve it by writing the equations in the input file as naturally as possible, as illustrated in the input file that follows.

## Input file

~~~wasora
include(lorenz.was)
~~~

## Execution

~~~
$ wasora lorenz.was | tee lorenz.dat
[...]
esyscmd(tail lorenz.dat)
$
~~~

![The attractor obtained by wasora after solving the Lorenz system with the nominal parameters.](lorenz.svg)

## Explanation

By using the `PHASE_SPACE` keyword, a three-dimensional phase-space spanned by variables\ $x$, $y$ and\ $z$ is defined. Therefore, wasora expects now three differential-algebraic equations involving these three variables and its time derivatives. The special variable `end_time` is set to forty, thus the system will be solved for the non-dimensional time range\ $0 < t < 40$. Parameters\ $\sigma$, $r$ and\ $b$ are assigned constant values, which by the way are the ones used by Lorenz in his original paper. The initials conditions are set by assigning values to the special symbols `x_0`, `y_0` and `z_0` which represent the initial values of said variables. These assignments are evaluated and processed only when\ $t=0$ and are ignored for\ $t>0$. The following lines define the dynamical system by adding a dot before the equal sign, i.e. “`.=`”. This construction tells wasora that the assignent is not a regular one but rather that a differential-algebraic expression is being defined. The postfix `_dot` indicates that the time derivative of the function is being referenced. Finally, the `PRINT` instruction writes into the standard output the current non-dimensional time\ $t$ and the three variables that constitute the solution of the dynamical system as time advances. Starting from\ $t=0$, wasora (actually the IDA library) chooses an appropriate time step so as to keep the numerical error bounded.
