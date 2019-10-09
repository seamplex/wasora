---
title: Wasora’s an advanced suite for optimization & reactor analysis
lang: en-US
...

![Logo](doc/logo.svg){.img-fluid}\ 


[Wasora](https://www.seamplex.com/wasora) is a (relatively) high-level tool to perform mathematical computations. It also provides a framework which other [particular computational codes](#plugins) can use. It is a free computational tool designed to aid a cognizant expert---i.e. you, whether an engineer, scientist, technician, geek, etc.---to analyze complex systems by solving mathematical problems by means of a high-level plain-text input file containing

 * algebraic expressions,
 * data for function interpolation,
 * differential equations, and
 * output instructions

among other facilities. For further details, read the [technical description](https://www.seamplex.com/wasora/description.html).

For example, the famous chaotic [Lorenz’ dynamical system](http://en.wikipedia.org/wiki/Lorenz_system)---the one of the butterfly---whose differential equations are

$$
\begin{cases}
\dot{x} &= \sigma \cdot (y - x)\\
\dot{y} &= x \cdot (r - z) - y\\
\dot{z} &= xy - bz
\end{cases}
$$

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

![The Lorenz attractor computed by wasora](examples/lorenz.svg){.img-responsive}


At a first glance, wasora may look as another high-level interpreted programming language, but---hopefully---it is not: wasora should be seen as a [syntactically-sweetened](http://en.wikipedia.org/wiki/Syntactic_sugar) way to ask a computer to perform a certain mathematical computation (which is what computers do).

Although its ultimate subject is optimization, it may hopefully help you with the tough calculations that usually appear when working with problems that have some kind of complexity, allowing the user to focus on what humans perform best---expert judgment and reaching conclusions. Some of its main features include

  * evaluation of [algebraic expressions](https://www.seamplex.com/wasora/doc/realbook/002-expressions)
  * [one](https://www.seamplex.com/wasora/doc/realbook/007-functions) and [multi-dimensional](https://www.seamplex.com/wasora/doc/realbook/010-2dfunctions) function interpolation
  * [scalar](https://www.seamplex.com/wasora/doc/realbook/001-peano), [vector](https://www.seamplex.com/wasora/doc/realbook/006-fibonacci) and matrix operations
  * numerical [integration](https://www.seamplex.com/wasora/doc/realbook/008-integrals), [differentiation](https://www.seamplex.com/wasora/doc/realbook/017-double-pendulum) and [root finding](https://www.seamplex.com/wasora/doc/realbook/020-gsl) of functions
  * possibility to solve [iterative](https://www.seamplex.com/wasora/doc/realbook/030-buffon) and/or [time-dependent](https://www.seamplex.com/wasora/doc/realbook/003-lag) problems
  * adaptive [integration of systems of differential-algebraic equations](https://www.seamplex.com/wasora/doc/realbook/018-waterwheel)
  * I/O from files and shared-memory objects (with optional synchronization using semaphores)
  * execution of [arbitrary code](https://www.seamplex.com/wasora/doc/realbook/015-primes) provided as shared object files
  * [parametric runs using quasi-random sequence numbers](https://www.seamplex.com/wasora/doc/realbook/025-mathace) to efficiently sweep a sub-space of parameter space 
  * solution of systems of non-linear algebraic equations
  * non-linear fit of scattered data to [one](https://www.seamplex.com/wasora/doc/realbook/020-gsl) or [multidimensional](https://www.seamplex.com/wasora/doc/realbook/028-mass) functions
  * [non-linear multidimensional optimization](https://www.seamplex.com/wasora/doc/realbook/020-gsl)
  * computations using multidimensional functions defined over of unstructured grids
  * complex extensions by means of [Plugins]

The code heavily relies on the numerical routines provided by the [GNU Scientific Library](http://www.gnu.org/software/gsl/), whose installation is mandatory. In fact, wasora can be seen as a high-level front-end to GSL's numerical procedures. The solution of differential-algebraic systems of equations is performed using the [SUNDIALS IDA Library](http://computation.llnl.gov/casc/sundials/main.html), although this feature usage is optional.


# Quick-start

> It is really worth any amount of time and effort to get away from Windows if you are doing computational science.
>
> <https://lists.mcs.anl.gov/pipermail/petsc-users/2015-July/026388.html>

Open a terminal in a GNU/Linux box (may be a VirtualBox box) and make sure you install the following packages (the last two are optional):

```
sudo apt-get install m4 make autoconf automake gcc git findutils libgsl-dev libsundials-dev libreadline-dev
```

Clone the wasora repository, bootstrap, configure, compile and check:

```
git clone https://github.com/seamplex/wasora.git
cd wasora
./autogen.sh
./configure
make
make check
```

If you have root access, install it system-wide:

```
sudo make install
```

If you get any error, including packages not found or other any issue, ask for help in the mailing list at <https://www.seamplex.com/lists.html>.

## Keeping up to date

To update wasora, go to the directory where the code has been previously cloned and run

```
git pull
./autogen.sh
./configure
make
make check
sudo make install
```

You can receive an email notification each time wasora is updated by watching the [Github repository](https://github.com/seamplex/wasora.git). Log in to Github and in the upper-right corner, click Watching from the "Watch" drop-down menu. 
See the file `INSTALL` for detailed installation instructions.


# Running wasora

Following a design decision, wasora reads a plain-text file referred to as the _input file_ that contains a set of alphanumeric keywords with their corresponding arguments that define a certain mathematical problem that is to be solved. See the file `examples/parser.was` that explains how wasora parses its input files.

In order to execute wasora, the compiled binary should be located by the system. The usual way to do this is to execute the `sudo make install` step above. But for those cases where root access is no available, an alternative is is to copy the `wasora` binary to a `bin` directory within the user’s home and add this path (if it is not already) to the `PATH` environment variable:

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

[fino](https://www.seamplex.com/fino)
:   solves partial differential equations using the finite element method

[milonga](https://www.seamplex.com/milonga)
:   solves the multigroup neutron diffusion equation

[xdfrrpf](https://bitbucket.org/tenuc/wasora/xdfrrpf)
:   eXtracts Data From RELAP Restart-Plot Files


The set of wasora plus one or more of its plugins is referred to as the _wasora suite_.


# The wasora Real Book

As jazz, wasora is best mastered when played. The wasora Realbook, like the [original](http://en.wikipedia.org/wiki/Real_Book), introduces fully-usable examples of increasing complexity and difficulty. The examples come with introductions, wasora inputs, terminal mimics, figures and discussions. They range from simple mechanical systems, chaotic attractors and even blackjack strategies:

 * [Introduction](https://www.seamplex.com/wasora/doc/realbook/index)
 * [Hello World](https://www.seamplex.com/wasora/doc/realbook/000-hello)
 * [The Peano axiom](https://www.seamplex.com/wasora/doc/realbook/001-peano)
 * [Algebraic expressions](https://www.seamplex.com/wasora/doc/realbook/002-expressions)
 * [First-order lags](https://www.seamplex.com/wasora/doc/realbook/003-lag)
 * [A simple differential equation](https://www.seamplex.com/wasora/doc/realbook/004-exp)
 * [The Lorenz chaotic system](https://www.seamplex.com/wasora/doc/realbook/005-lorenz)
 * [The Fibonacci sequence](https://www.seamplex.com/wasora/doc/realbook/006-fibonacci)
 * [One-dimensional functions](https://www.seamplex.com/wasora/doc/realbook/007-functions)
 * [Having fun with integrals](https://www.seamplex.com/wasora/doc/realbook/008-integrals)
 * [Two-dimensional functions](https://www.seamplex.com/wasora/doc/realbook/010-2dfunctions)
 * [Newton, Lagrange & Hamilton](https://www.seamplex.com/wasora/doc/realbook/012-mechanics)
 * [Finding prime numbers](https://www.seamplex.com/wasora/doc/realbook/015-primes)
 * [The double pendulum](https://www.seamplex.com/wasora/doc/realbook/017-double-pendulum)
 * [The chaotic Lorenzian waterwheel](https://www.seamplex.com/wasora/doc/realbook/018-waterwheel)
 * [GNU Scientific Library examples rewritten](https://www.seamplex.com/wasora/doc/realbook/020-gsl)
 * [Math Ace](https://www.seamplex.com/wasora/doc/realbook/025-mathace)
 * [Semi-empirical mass formula fit](https://www.seamplex.com/wasora/doc/realbook/028-mass)
 * [Buffon's needle](https://www.seamplex.com/wasora/doc/realbook/030-buffon)
 * [Point reactor kinetics---direct and inverse](https://www.seamplex.com/wasora/doc/realbook/042-point-kinetics)



# Licensing

Wasora is distributed under the terms of the [GNU General Public License](http://www.gnu.org/copyleft/gpl.html) version 3 or (at your option) any later version. The following text was borrowed from the [Gmsh documentation](http://gmsh.info/doc/texinfo/gmsh.html#Copying-conditions), and replacing “Gmsh” with “wasora” gives:

> Wasora is “free software”; this means that everyone is free to use it and to redistribute it on a free basis. Wasora is not in the public domain; it is copyrighted and there are restrictions on its distribution, but these restrictions are designed to permit everything that a good cooperating citizen would want to do. What is not allowed is to try to prevent others from further sharing any version of wasora that they might get from you.
>
> Specifically, we want to make sure that you have the right to give away copies of wasora, that you receive source code or else can get it if you want it, that you can change wasora or use pieces of wasora in new free programs, and that you know you can do these things.
>
> To make sure that everyone has such rights, we have to forbid you to deprive anyone else of these rights. For example, if you distribute copies of wasora, you must give the recipients all the rights that you have. You must make sure that they, too, receive or can get the source code. And you must tell them their rights.
>
> Also, for our own protection, we must make certain that everyone finds out that there is no warranty for wasora. If wasora is modified by someone else and passed on, we want their recipients to know that what they have is not what we distributed, so that any problems introduced by others will not reflect on our reputation.
>
> The precise conditions of the license for wasora are found in the [General Public License](https://github.com/seamplex/wasora/blob/master/COPYING) that accompanies the source code. Further information about this license is available from the GNU Project webpage <http://www.gnu.org/copyleft/gpl-faq.html>.


# Further information

Home page: <https://www.seamplex.com/wasora>  
Repository: <https://github.com/seamplex/wasora>  
Mailing list and bug reports: <wasora@seamplex.com>  (you need to subscribe first at <wasora+subscribe@seamplex.com>)  
Follow us: [Twitter](https://twitter.com/seamplex/) [YouTube](https://www.youtube.com/channel/UCC6SzVLxO8h6j5rLlfCQPhA) [LinkedIn](https://www.linkedin.com/company/seamplex/) [Github](https://github.com/seamplex)

----------------------------------------------------

wasora is copyright ©2009--2019 Jeremy Theler   
wasora is licensed under [GNU GPL version 3](http://www.gnu.org/copyleft/gpl.html) or (at your option) any later version.  
wasora is free software: you are free to change and redistribute it.  
There is NO WARRANTY, to the extent permitted by law.  
See the file [`COPYING`](https://github.com/seamplex/wasora/blob/master/COPYING) for copying conditions.  
