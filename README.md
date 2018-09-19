---
title: Wasora’s an advanced suite for optimization & reactor analysis
author: Jeremy Theler
lang: en-US
numbersections: true
formats: html tex pdf
template: light
...

![Logo](doc/logo.svg){.img-responsive}\ 


[Wasora](https://www.seamplex.com/wasora) is a (relatively) high-level tool to perform mathematical computations. It also provides a framework which other [particular computational codes](#plugins) can use. It is a free computational tool designed to aid a cognizant expert---i.e. you, whether an engineer, scientist, technician, geek, etc.---to analyze complex systems by solving mathematical problems by means of a high-level plain-text input file containing

 * algebraic expressions,
 * data for function interpolation,
 * differential equations, and
 * output instructions

amongst other facilities.

For example, the famous chaotic [Lorenz’ dynamical system](http://en.wikipedia.org/wiki/Lorenz_system)---the one of the butterfly---whose differential equations are

\begin{align*}
\dot{x} &= \sigma \cdot (y - x)\\
\dot{y} &= x \cdot (r - z) - y\\
\dot{z} &= xy - bz\\
\end{align*}
where $\sigma=10$, $b=8/3$ and $r=28$ are the classical parameters that generate the butterfly as presented by Edward Lorenz back in his seminal 1963 paper [Deterministic non-periodic flow](http://journals.ametsoc.org/doi/abs/10.1175/1520-0469%281963%29020%3C0130%3ADNF%3E2.0.CO%3B2), can be solved with wasora by writing the equations in the input file as naturally as possible, as illustrated in the input file that follows:

```wasora
# lorenz’ seminal dynamical system
PHASE_SPACE x y z
end_time = 40

CONST sigma r b
sigma = 10            # parameters
r = 28
b = 8/3

x_0 = -11             # initial conditions
y_0 = -16
z_0 = 22.5

# the dynamical system
x_dot .= sigma*(y - x)
y_dot .= x*(r - z) - y
z_dot .= x*y - b*z

PRINT t x y z HEADER
```

Following the [UNIX Philosophy](https://en.wikipedia.org/wiki/Unix_philosophy), wasora’s output can be piped for example to [Gnuplot](http://gnuplot.info/) in order to obtain a beautiful figure:

```
wasora lorenz.was | gnuplot -e "set terminal svg; set output 'lorenz.svg'; set ticslevel 0; splot '-' u 2:3:4 w l ti ''"

```
![The Lorenz attractor computed by wasora](examples/lorenz.svg){.img-responsive}\ 


At a first glance, wasora may look as another high-level interpreted programming language, but---hopefully---it is not: wasora should be seen as a [syntactically-sweetened](http://en.wikipedia.org/wiki/Syntactic_sugar) way to ask a computer to perform a certain mathematical calculation.

Although its ultimate subject is optimization, it may hopefully help you with the tough calculations that usually appear when working with problems that have some kind of complexity, allowing the user to focus on what humans perform best---expert judgment and reaching conclusions. Some of its main features include

  * evaluation of [algebraic expressions](https://www.seamplex.com/wasora/realbook/real-002-expressions.html)
  * [one](https://www.seamplex.com/wasora/realbook/real-007-functions.html) and [multi-dimensional](https://www.seamplex.com/wasora/realbook/real-010-2dfunctions.html) function interpolation
  * [scalar](https://www.seamplex.com/wasora/realbook/real-001-peano.html), [vector](https://www.seamplex.com/wasora/realbook/real-006-fibonacci.html) and matrix operations
  * numerical [integration](https://www.seamplex.com/wasora/realbook/real-008-integrals.html), [differentiation](https://www.seamplex.com/wasora/realbook/real-017-double-pendulum.html) and [root finding](https://www.seamplex.com/wasora/realbook/real-020-gsl.html) of functions
  * possibility to solve [iterative](https://www.seamplex.com/wasora/realbook/real-030-buffon.html) and/or [time-dependent](https://www.seamplex.com/wasora/realbook/real-003-lag.html) problems
  * adaptive [integration of systems of differential-algebraic equations](https://www.seamplex.com/wasora/realbook/real-018-waterwheel.html)
  * I/O from files and shared-memory objects (with optional synchronization using semaphores)
  * execution of [arbitrary code](https://www.seamplex.com/wasora/realbook/real-015-primes) provided as shared object files
  * [parametric runs using quasi-random sequence numbers](https://www.seamplex.com/wasora/realbook/real-025-mathace.html) to efficiently sweep a sub-space of parameter space 
  * solution of systems of non-linear algebraic equations
  * non-linear fit of scattered data to [one](https://www.seamplex.com/wasora/realbook/real-020-gsl.html) or [multidimensional](https://www.seamplex.com/wasora/realbook/real-028-mass.html) functions
  * [non-linear multidimensional optimization](https://www.seamplex.com/wasora/realbook/real-020-gsl.html)
  * management of unstructured grids
  * complex extensions by means of [Plugins]

The code heavily relies on the numerical routines provided by the [GNU Scientific Library](http://www.gnu.org/software/gsl/), whose installation is mandatory. In fact, wasora can be seen as a high-level front-end to GSL's numerical procedures. The solution of differential-algebraic systems of equations is performed using the [SUNDIALS IDA Library](http://computation.llnl.gov/casc/sundials/main.html), although this feature usage is optional.


# Quick-start

> It is really worth any amount of time and effort to get away from Windows
if you are doing computational science.
>
> <https://lists.mcs.anl.gov/pipermail/petsc-users/2015-July/026388.html>

Open a terminal in a GNU/Linux box (may be a VirtualBox box) and make sure you install the following packages (the last two are optional):

```
sudo apt-get install m4 make autoconf automake gcc git findutils libgsl-dev libsundials-serial-dev libreadline-dev
```

Clone the wasora repository, bootstrap, configure, compile and check:

```
git clone https://bitbucket.org/seamplex/wasora/
cd wasora
./autogen.sh
./configure
make
make check
```

If you get any error, including packages not found or other any issue, ask for help in the mailing list at <https://www.seamplex.com/lists.html>.

## Keeping up to date

To update wasora, go to the directory where the code has been previously clone and run

```
git pull
./autogen.sh
./configure
make
make check
```

See the file `INSTALL` for detailed installation instructions.


# Running wasora

Following a design decision, wasora reads a plain-text file referred to as the _input file_ that contains a set of alphanumeric keywords with their corresponding arguments that define a certain mathematical problem that is to be solved. See the file `examples/parser.was` that explains how wasora parses its input files.

In order to execute wasora, the compiled binary should be located by the system. One way of doing this is copying (as root) the `wasora` executable to a system-wide binary directory such as `/usr/local/bin`:

```
sudo cp wasora /usr/local/bin
```

Another alternative that does not require root access is to copy it to a `bin` directory within the user’s home and add this path (if it is not already) to the `PATH` environment variable:

```
mkdir -p $HOME/bin
cp wasora $HOME/bin
echo 'PATH=$PATH:$HOME/bin' >> $HOME/.bashrc
```

Close the terminal and open a new one, and `wasora` ought to be available from any directory.


## Invocation

Assuming an input file named `input.was` exists in the current directory, then the proper execution instruction is

```
$ wasora input.was
```

There exist some command line options---that may be consulted using the `--help` option---that are detailed discussed in the complete documentation. In particular, the `--version` option shows information about the wasora version and the libraries it was linked against:

```
$ wasora --version
wasora v0.5.252-gbf723b9
wasora’s an advanced suite for optimization & reactor analysis

 last commit on Tue Sep 18 06:40:31 2018 -0300
 compiled on 2018-09-18 18:41:15 by gtheler@hera ( )
 with gcc (Debian 8.2.0-6) 8.2.0 using -O2 and linked against
  GNU Scientific Library version 2.5

 wasora is copyright (C) 2009-2018 jeremy theler
 licensed under GNU GPL version 3 or later.
 wasora is free software: you are free to change and redistribute it.
 There is NO WARRANTY, to the extent permitted by law.
$ 
```

The input file may also ask for command line options---for example to pass run-time arguments so the same input file can be used to solve similar problems---by referring them as `$1`, `$2`, etc. These `$n` expressions are literally replaced by the command line arguments provided after the input file. So for example, the following single-line input file (which can be found in `examples/calc.was`):

```wasora
PRINT %g $1
```

can be used as a command-line calculator:

```
$ wasora calc.was 1+1
2
$
```

See the `examples/parser.was` file, the [Examples & test suite] and [The wasora Real Book] sections below for examples of usage of arguments.


# Examples & test suite

After the compilation of the code (that follows the standard `./configure && make` procedure, see `INSTALL` for details), one recommended step is to run the test suite with

```
$ make check
```

It consists of ten examples of application that use different kind of the features provided by wasora. They work both as examples of usage and as a suite of tests that check that wasora implements correctly the functionalities that are expected. A more detailed set of examples that illustrate real applications of wasora in a wide variety of fields---ranging from classical mechanical systems up to analysis of blackjack strategies---can be found in [The wasora Real Book]. Some of the cases in the test suite generate graphical data which is shown on the screen using [gnuplot](http://www.gnuplot.info/), provided it is installed.

The `make check` command may not show the actual output of the examples but the overall result (i.e. whether the test passed, the test failed or the test was skipped). Each individual test may be repeated by executing the `test-*.sh` scripts located in the `examples` subdirectory.

# Plugins

Almost any single feature included in the code was needed at least once by [the author](http://www.seamplex.com/jeremy) during his career in the nuclear industry. Nevertheless, wasora is aimed at solving general mathematical problems (see below for a description of the [wasora Real Book](#the-wasora-real-book)). Should a particular calculation be needed, wasora's features may be extended by the implementation of dynamically-loaded plugins, for example:

[besssugo](https://bitbucket.org/seamplex/bessugo)
:   builds scientific videos out of wasora computations

[fino](https://www.seamplex.com/fino)
:   solves partial differential equations using the finite element method

[milonga](https://www.seamplex.com/milonga)
:   solves the multigroup neutron diffusion equation

[waspy](https://bitbucket.org/seamplex/waspy)
:   runs Python code within wasora sharing variables, vectors and matrices

[xdfrrpf](https://bitbucket.org/tenuc/wasora/xdfrrpf)
:   eXtracts Data From RELAP Restart-Plot Files


The set of wasora plus one or more of its plugins is referred to as the _wasora suite_.


# The wasora Real Book

As jazz, wasora is best mastered when played. The wasora Realbook, like the [original](http://en.wikipedia.org/wiki/Real_Book), introduces fully-usable examples of increasing complexity and difficulty. The examples come with introductions, wasora inputs, terminal mimics, figures and discussions. They range from simple mechanical systems, chaotic attractors and even blackjack strategies:

 * [Introduction](https://www.seamplex.com/wasora/realbook/index.html)
 * [Hello World](https://www.seamplex.com/wasora/realbook/real-000-hello.html)
 * [The Peano axiom](https://www.seamplex.com/wasora/realbook/real-001-peano.html)
 * [Algebraic expressions](https://www.seamplex.com/wasora/realbook/real-002-expressions.html)
 * [First-order lags](https://www.seamplex.com/wasora/realbook/real-003-lag.html)
 * [A simple differential equation](https://www.seamplex.com/wasora/realbook/real-004-exp.html)
 * [The Lorenz chaotic system](https://www.seamplex.com/wasora/realbook/real-005-lorenz.html)
 * [The Fibonacci sequence](https://www.seamplex.com/wasora/realbook/real-006-fibonacci.html)
 * [One-dimensional functions](https://www.seamplex.com/wasora/realbook/real-007-functions.html)
 * [Having fun with integrals](https://www.seamplex.com/wasora/realbook/real-008-integrals.html)
 * [Two-dimensional functions](https://www.seamplex.com/wasora/realbook/real-010-2dfunctions.html)
 * [Newton, Lagrange & Hamilton](https://www.seamplex.com/wasora/realbook/real-012-mechanics.html)
 * [Finding prime numbers](https://www.seamplex.com/wasora/realbook/real-015-primes.html)
 * [The double pendulum](https://www.seamplex.com/wasora/realbook/real-017-double-pendulum.html)
 * [The chaotic Lorenzian waterwheel](https://www.seamplex.com/wasora/realbook/real-018-waterwheel.html)
 * [GNU Scientific Library examples rewritten](https://www.seamplex.com/wasora/realbook/real-020-gsl.html)
 * [Math Ace](https://www.seamplex.com/wasora/realbook/real-025-mathace.html)
 * [Semi-empirical mass formula fit](https://www.seamplex.com/wasora/realbook/real-028-mass.html)
 * [Buffon's needle](https://www.seamplex.com/wasora/realbook/real-030-buffon.html)
 * [Point reactor kinetics---direct and inverse](https://www.seamplex.com/wasora/realbook/real-042-point-kinetics.html)



# Licensing

Wasora is distributed under the terms of the [GNU General Public License](http://www.gnu.org/copyleft/gpl.html) version 3 or (at your option) any later version.


# Further information

Home page: <https://www.seamplex.com/wasora>  
Repository: <https://bitbucket.org/seamplex/wasora>  
Mailing list and bug reports: <wasora@seamplex.com>  (you need to subscribe first at <wasora+subscribe@seamplex.com>)  
Follow us: [Twitter](https://twitter.com/seamplex/) [YouTube](https://www.youtube.com/channel/UCC6SzVLxO8h6j5rLlfCQPhA) [LinkedIn](https://www.linkedin.com/company/seamplex/) [Bitbucket](https://bitbucket.org/seamplex/)

----------------------------------------------------

wasora is copyright (C) 2009--2018 Jeremy Theler  
wasora is licensed under [GNU GPL version 3](http://www.gnu.org/copyleft/gpl.html) or (at your option) any later version.  
wasora is free software: you are free to change and redistribute it.  
There is NO WARRANTY, to the extent permitted by law.  
See the file [`COPYING`](https://bitbucket.org/wasora/wasora/src/default/COPYING) for copying conditions.  
