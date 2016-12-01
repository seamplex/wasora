
[wasora](https://wasora.bitbucket.org) is a free computational tool designed to aid a cognizant expert---i.e. you, whether an engineer, scientist, technician, geek, etc.---to analyze complex systems by solving mathematical problems by means of a high-level plain-text input file containing algebraic expressions, data for function interpolation, differential equations and output instructions amongst other facilities. At a first glance, it may look as another high-level interpreted programming language, but---hopefully---it is not: wasora should be seen as a [syntactically-sweetened](http://en.wikipedia.org/wiki/Syntactic_sugar) way to ask a computer to perform a certain mathematical calculation. For example, see [here](https://wasora.bitbucket.org/realbook/._realbook006.html) to find how the famous [Lorenz system](http://en.wikipedia.org/wiki/Lorenz_system) may be solved by writing the three differential equations into a plain-text input file as humanly-friendly as possible.

Although its ultimate subject is optimization, it may hopefully help you with the tough calculations that usually appear when working with problems that have some kind of complexity, allowing the user to focus on what humans perform best---expert judgment and reaching conclusions. Some of its main features include

  * evaluation of [algebraic expressions](https://wasora.bitbucket.org/realbook/._realbook003.html)
  * [one](https://wasora.bitbucket.org/realbook/._realbook008.html) and [multi-dimensional](https://wasora.bitbucket.org/realbook/._realbook010.html) function interpolation
  * [scalar](https://wasora.bitbucket.org/realbook/._realbook002.html), [vector](https://wasora.bitbucket.org/realbook/._realbook007.html) and matrix operations
  * numerical [integration](https://wasora.bitbucket.org/realbook/._realbook009.html), [differentiation](https://wasora.bitbucket.org/realbook/._realbook013.html) and [root finding](https://wasora.bitbucket.org/realbook/._realbook015.html) of functions
  * possibility to solve [iterative](https://wasora.bitbucket.org/realbook/._realbook018.html) and/or [time-dependent](https://wasora.bitbucket.org/realbook/._realbook004.html) problems
  * adaptive [integration of systems of differential-algebraic equations](https://wasora.bitbucket.org/realbook/._realbook020.html)
  * I/O from files and shared-memory objects (with optional synchronization using semaphores)
  * execution of [arbitrary code](https://wasora.bitbucket.org/realbook/._realbook012.html) provided as shared object files
  * [parametric runs using quasi-random sequence numbers](https://wasora.bitbucket.org/realbook/._realbook016.html) to efficiently sweep a sub-space of parameter space 
  * solution of systems of non-linear algebraic equations
  * non-linear fit of scattered data to [one](https://wasora.bitbucket.org/realbook/._realbook015.html) or [multidimensional](https://wasora.bitbucket.org/realbook/._realbook017.html) functions
  * [non-linear multidimensional optimization](https://wasora.bitbucket.org/realbook/._realbook015.html)
  * management of unstructured grids
  * complex extensions by means of plugins

The code heavily relies on the numerical routines provided by the [GNU Scientific Library](http://www.gnu.org/software/gsl/), whose installation is mandatory. In fact, wasora can be seen as a high-level front-end to GSL's numerical procedures. The solution of differential-algebraic systems of equations is performed using the [SUNDIALS IDA Library](http://computation.llnl.gov/casc/sundials/main.html), although this feature usage is optional. See the file `INSTALL` for details about compilation and installation.


# Running wasora

Following a design decision, wasora reads a plain-text file referred to as the _input file_ that contains a set of alphanumeric keywords with their corresponding arguments that define a certain mathematical problem that is to be solved. See the file `examples/parser.was` that explains how wasora parses its input files.

If you obtained the source tree---either by downloading the tarball or by cloning the [mercurial repository](https://bitbucket.org/wasora/wasora)---wasora has to be compiled to obtain a binary executable (see the file `INSTALL` for details). If you downloaded a binary tarball for your architecture, the executable should be located in the root directory of the distribution. This executable can be either installed in a system-wide location (for example in `/usr/bin`), into a directory contained in the user's `$PATH` environment variable (for example in `$HOME/bin`) or even in present working directory (i.e. where the input file is). The appropriate decision is up to the user. In any case, wasora expects the name of the input file (or a path if it is not located in the current directory, although this situation may mangle the access to other needed files) as the first argument. Assuming wasora is installed in a directory listed in the `$PATH` variable and that the input file is named `input.was`, then the proper execution instruction is

    $ wasora input.was

There exist some command line options---that may be consulted using the `--help` option---that are detailed discussed in the complete documentation. In particular, the `--version` option shows information about the wasora version and the libraries it was linked against:

```
$ wasora --version
wasora 0.4.35  (c0750cdeca3f 2015-12-28 11:59 -0300)
wasora's an advanced suite for optimization & reactor analysis

 rev hash c0750cdeca3f8cf8eae4b80ec3cfdfd85321f5b0
 last commit on 2015-12-28 11:59 -0300 (rev 189)

 compiled on 2015-12-28 14:34:30 by jeremy@tom (linux-gnu x86_64)
 with gcc (Debian 4.9.2-10) 4.9.2 using -O2 and linked against
  GNU Scientific Library version 1.16
  GNU Readline version 6.3
  SUNDIALs Library version 2.5.0


 wasora is copyright (C) 2009-2015 jeremy theler
 licensed under GNU GPL version 3 or later.
 wasora is free software: you are free to change and redistribute it.
 There is NO WARRANTY, to the extent permitted by law.
```

The input file may also ask for command line options---for example to pass run-time arguments so the same input file can be used to solve similar problems---by referring them as `$1`, `$2`, etc. These `$n` expressions are literally replaced by the command line arguments provided after the input file. So for example, the following single-line input file (which can be found in `examples/calc.was`):

```wasora
PRINT %g $1
```

can be used as a command-line calculator:

    $ wasora calc.was 1+1
    2
    $

See the `examples/parser.was` file, the [Examples & test suite] and [The wasora Real Book] sections below for examples of usage of arguments.


# Examples & test suite

After the compilation of the code (that follows the standard `./configure && make` procedure, see `INSTALL` for details), one recommended step is to run the test suite with

    $ make check

It consists of ten examples of application that use different kind of the features provided by wasora. They work both as examples of usage and as a suite of tests that check that wasora implements correctly the functionalities that are expected. A more detailed set of examples that illustrate real applications of wasora in a wide variety of fields---ranging from classical mechanical systems up to analysis of blackjack strategies---can be found in [The wasora Real Book]. Some of the cases in the test suite generate graphical data which is shown on the screen using [gnuplot](http://www.gnuplot.info/), provided it is installed.

The `make check` command may not show the actual output of the examples but the overall result (i.e. whether the test passed, the test failed or the test was skipped). Each individual test may be repeated by executing the `test-*.sh` scripts located in the `examples` subdirectory.

# Plugins

Almost any single feature included in the code was needed at least once by [the author](http://www.talador.com.ar/jeremy) during his career in the nuclear industry. Nevertheless, wasora is aimed at solving general mathematical problems (see below for a description of the [wasora Real Book](#the-wasora-real-book)). Should a particular calculation be needed, wasora's features may be extended by the implementation of dynamically-loaded plugins, for example:

[besssugo](https://wasora.bitbucket.org/besssugo)
:   builds scientific videos out of wasora computations

[fino](https://bitbucket.org/wasora/wasora/fino)
:   solves partial differential equations using the finite element method

[milonga](https://wasora.bitbucket.org/milonga)
:   solves the multigroup neutron diffusion equation

[waspy](https://bitbucket.org/wasora/wasora/waspy)
:   runs Python code within wasora sharing variables, vectors and matrices

[xdfrrpf](https://bitbucket.org/tenuc/wasora/xdfrrpf)
:   eXtracts Data From RELAP Restart-Plot Files


A template that can be used to write an _ad-hoc_ plugin can be cloned with

    $ hg https://bitbucket.org/wasora/skel

The set of wasora plus one or more of its plugins is referred to as the _wasora suite_.


# The wasora Real Book

As jazz, wasora is best mastered when played. The wasora Realbook, like the [original](http://en.wikipedia.org/wiki/Real_Book), introduces fully-usable examples of increasing complexity and difficulty. The examples come with introductions, wasora inputs, terminal mimics, figures and discussions. They range from simple mechanical systems, chaotic attractors and even blackjack strategies. Take a look at the [index](https://wasora.bitbucket.org/realbook/genindex.html) to see how a certain keyword is used in a real-world application. 


# Further information

See the file `INSTALL` for compilation and installation instructions.  
See the directory `examples` for the test suite and other examples.  
See the contents of directory `doc` for full documentation.  

Home page(s): <https://wasora.bitbucket.org>  & <http://wasora.bitbucket.org>  
Mailing list and bug reports: <https://groups.google.com/a/seamplex.com/forum/#!forum/wasora>  
FAQ: <http://wasora.bitbucket.org/FAQ.html>  

----------------------------------------------------

wasora is copyright (C) 2009--2016 Germán (jeremy) Theler  
wasora is licensed under [GNU GPL version 3](http://www.gnu.org/copyleft/gpl.html) or (at your option) any later version.  
wasora is free software: you are free to change and redistribute it.  
There is NO WARRANTY, to the extent permitted by law.  
See the file [`COPYING`](https://bitbucket.org/wasora/wasora/src/default/COPYING) for copying conditions.  
