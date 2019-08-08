% The wasora Realbook
% Jeremy Theler
%

![](wasorarealbook.svg){.img-fluid}\ 


__Abstract.__ As with jazz, [wasora](https://www.seamplex.com/wasora) is best mastered when played. Welcome thus to *The wasora Realbook* which, as the [original](http://en.wikipedia.org/wiki/Real_Book), will guide you through wasora with annotated examples of increasing complexity and difficulty. The examples come with introductions, wasora inputs, terminal mimics, figures and discussions. This book is a living document that is eager to be fed by users willing to share experiences.

# Introduction

[Wasora](https://www.seamplex.com/wasora) is a free computational tool designed to aid a cognizant expert---i.e. you, whether an engineer, scientist, technician, geek, etc---to analyze complex systems by solving mathematical problems by means of a high-level plain-text input file containing algebraic expressions, data for function interpolation, differential equations and output instructions amongst other facilities.

At a first glance, it may look as another high-level interpreted programming language, but---hopefully---it is not: wasora should be seen as a syntactically-sweetened way to ask a computer to perform a certain mathematical calculation. For example, the famous Lorenz system may be solved by writing the three differential equations into a plain-text input file as humanly-friendly as possible:

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

PRINT t x y z
```

Although its ultimate subject is optimization, it may hopefully help you with the tough calculations that usually appear when working with problems that have some kind of complexity, allowing the user to focus on what humans perform best---expert judgment and reaching conclusions.

wasora may be extended to perform particular computations by the implementation of dynamically-loaded plugins (such as [milonga](https://www.seamplex.com/milonga) to solve the multigroup neutron diffusion equation over unstructured grids, [fino](https://www.seamplex.com/fino) to solve partial differential equations--particularly mechanical problems, or [besssugo](https://www.seamplex.com/besssugo) to build scientific videos out of wasora computations). See the [home page](https://www.seamplex.com/wasora) for more information about plugins.



## About the examples

The examples contained in the wasora Real Book are taken from a variety of engineering cases where a certain amount of mathematical computations have to be performed. They illustrate how wasora can be used. Some of them are serious, some of them are funny. The structure is the same for every example:

 1. a brief introduction and/or description to the problem to be solved is given 
 2. a wasora input is shown that completely or partially solves the problem
 3. an hypothetical terminal is mimicked which illustrates how wasora is to be called, optionally with invocation to further tools for plotting or comparing results 
 4. the results in the form of figures are shown and discussed

Most of the figures are created by [qdp](https://github.com/seamplex/qdp) which stands for *quick & dirty plot* and is a shell script that interfaces with [pyxplot](http://pyxplot.org.uk/) to generate plots of data within a single command.


# Contents

include(toc.md)

