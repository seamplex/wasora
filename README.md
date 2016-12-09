% wasora's an advanced suite for optimization & reactor analysis

![wasora](doc/logo.svg){.img-responsive}\ 


[Wasora](https://www.seamplex.com/wasora) is a free computational tool designed to aid a cognizant expert---i.e. you, whether an engineer, scientist, technician, geek, etc.---to analyze complex systems by solving mathematical problems by means of a high-level plain-text input file containing algebraic expressions, data for function interpolation, differential equations and output instructions amongst other facilities. At a first glance, it may look as another high-level interpreted programming language, but---hopefully---it is not: wasora should be seen as a [syntactically-sweetened](http://en.wikipedia.org/wiki/Syntactic_sugar) way to ask a computer to perform a certain mathematical calculation. For example, see [here](https://www.seamplex.com/wasora/realbook/real-._realbook006.html) to find how the famous [Lorenz system](http://en.wikipedia.org/wiki/Lorenz_system) may be solved by writing the three differential equations into a plain-text input file as humanly-friendly as possible.

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

The code heavily relies on the numerical routines provided by the [GNU Scientific Library](http://www.gnu.org/software/gsl/), whose installation is mandatory. In fact, wasora can be seen as a high-level front-end to GSL's numerical procedures. The solution of differential-algebraic systems of equations is performed using the [SUNDIALS IDA Library](http://computation.llnl.gov/casc/sundials/main.html), although this feature usage is optional. See the file `INSTALL` for details about compilation and installation.


# Quick-start


If you are impatient to run wasora (or have failed to follow the instructions in [INSTALL](INSTALL.md)), open a terminal in any GNU/Linux box (may be a VirtualBox box) and run:

```
curl https://www.seamplex.com/wasora/get.sh | sh
```

Make sure you have `curl` installed and if you are behind a proxy, that both `http_proxy` and `https_proxy` are properly set. If you get any error, either

 a. See the detailed explanation in [INSTALL](INSTALL.md)  
 b. Ask for help in the mailing list at <https://groups.google.com/a/seamplex.com/forum/#!forum/wasora>

If these instructions are non-sense to you, go directly to point b.


# Running wasora

Following a design decision, wasora reads a plain-text file referred to as the _input file_ that contains a set of alphanumeric keywords with their corresponding arguments that define a certain mathematical problem that is to be solved. See the file `examples/parser.was` that explains how wasora parses its input files.

Assuming wasora is installed in a directory listed in the `$PATH` variable and that the input file is named `input.was`, then the proper execution instruction is

```
$ wasora input.was
```

There exist some command line options---that may be consulted using the `--help` option---that are detailed discussed in the complete documentation. In particular, the `--version` option shows information about the wasora version and the libraries it was linked against:

```
$ wasora --version
wasora v0.5.24-gd413ad5
wasora’s an advanced suite for optimization & reactor analysis

 last commit on Wed Dec 7 12:26:02 2016 -0300
 compiled on 2016-12-08 07:32:03 by gtheler@tom ( )
 with gcc (Debian 4.9.2-10) 4.9.2 using -Wall and linked against
  GNU Scientific Library version 1.16
  SUNDIALs Library version 2.5.0
  GNU Readline version 6.3

 wasora is copyright (C) 2009-2016 jeremy theler
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

[waspy](https://bitbucket.org/wasora/wasora/waspy)
:   runs Python code within wasora sharing variables, vectors and matrices

[xdfrrpf](https://bitbucket.org/tenuc/wasora/xdfrrpf)
:   eXtracts Data From RELAP Restart-Plot Files


The set of wasora plus one or more of its plugins is referred to as the _wasora suite_.


# The wasora Real Book

As jazz, wasora is best mastered when played. The wasora Realbook, like the [original](http://en.wikipedia.org/wiki/Real_Book), introduces fully-usable examples of increasing complexity and difficulty. The examples come with introductions, wasora inputs, terminal mimics, figures and discussions. They range from simple mechanical systems, chaotic attractors and even blackjack strategies. Take a look at the [index](https://www.seamplex.com/wasora/realbook/real-genindex.html) to see how a certain keyword is used in a real-world application. 


# Licensing

Wasora is distributed under the terms of the [GNU General Public License](http://www.gnu.org/copyleft/gpl.html) version 3 or (at your option) any later version.


# Further information

Home page: <https://www.seamplex.com/wasora>  
Repository: <https://bitbucket.org/seamplex/wasora>  
Mailing list and bug reports: <wasora@seamplex.com>  (you need to subscribe first at <wasora+subscribe@seamplex.com>)  
Follow us: [Twitter](https://twitter.com/seamplex/) [YouTube](https://www.youtube.com/channel/UCC6SzVLxO8h6j5rLlfCQPhA) [LinkedIn](https://www.linkedin.com/company/seamplex/) [Bitbucket](https://bitbucket.org/seamplex/)

----------------------------------------------------

wasora is copyright (C) 2009--2016 Jeremy Theler  
wasora is licensed under [GNU GPL version 3](http://www.gnu.org/copyleft/gpl.html) or (at your option) any later version.  
wasora is free software: you are free to change and redistribute it.  
There is NO WARRANTY, to the extent permitted by law.  
See the file [`COPYING`](https://bitbucket.org/wasora/wasora/src/default/COPYING) for copying conditions.  
