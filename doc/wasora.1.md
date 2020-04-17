% WASORA(1) Wasora User Manual
% Jeremy Theler

# NAME

wasora - wasora’s an advanced suite for optimization & reactor analysis

# SYNOPSIS

wasora [*options*] input-file [*optional_extra_arguments*]...


# DESCRIPTION

[Wasora](https://www.seamplex.com/wasora) is a (relatively) high-level tool to perform mathematical computations. It also provides a framework which other particular computational codes can use. It is a free computational tool designed to aid a cognizant expert---i.e. you, whether an engineer, scientist, technician, geek, etc.---to analyze complex systems by solving mathematical problems by means of a high-level plain-text input file containing

 * algebraic expressions,
 * data for function interpolation,
 * differential equations, and
 * output instructions

among other facilities.

For example, the famous chaotic [Lorenz’ dynamical system](http://en.wikipedia.org/wiki/Lorenz_system)---the one of the butterfly---whose differential equations are

$$\dot{x} = \sigma \cdot (y - x)$$
$$\dot{y} = x \cdot (r - z) - y$$
$$\dot{z} = xy - bz$$

where $\sigma=10$, $b=8/3$ and $r=28$ are the classical parameters that generate the butterfly as presented by Edward Lorenz back in his seminal 1963 paper [Deterministic non-periodic flow](http://journals.ametsoc.org/doi/abs/10.1175/1520-0469%281963%29020%3C0130%3ADNF%3E2.0.CO%3B2), can be solved with wasora by writing the equations in the input file as naturally as possible, as illustrated in the input file that follows:

```{.wasora style=wasora}
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
wasora lorenz.was | gnuplot -e "splot '-' u 2:3:4 w l ti ''"
```

![The Lorenz attractor computed by wasora](lorenz)


At a first glance, wasora may look as another high-level interpreted programming language, but---hopefully---it is not: wasora should be seen as a [syntactically-sweetened](http://en.wikipedia.org/wiki/Syntactic_sugar) way to ask a computer to perform a certain mathematical computation (which is what computers do).

 
# OPTIONS

include(help.md)

# REFERENCE

## Keywords

esyscmd([../reference.sh parser kw | sed 's/^#/##/' .])



## Mesh-related keywords

esyscmd([../reference.sh mesh/parser kw | sed 's/^#/##/' .])



## Variables

esyscmd([../reference.sh init va | sed 's/^#/##/' .])



## Mesh-related variables

esyscmd([../reference.sh mesh/init va | sed 's/^#/##/' .])



## Functions

esyscmd([../reference.sh builtinfunctions fn | sed 's/^#/##/' .])



## Functionals

esyscmd([../reference.sh builtinfunctionals fu | sed 's/^#/##/' .])



## Vector functions

esyscmd([../reference.sh builtinvectorfunctions fv | sed 's/^#/##/' .])



# SEE ALSO

`gnuplot`(1)

The wasora Real Book at <https://www.seamplex.com/wasora/doc/realbook/> contains fully-discussed examples.

The wasora web page contains full source code, updates, examples, V&V cases and full reference:
<https://www.seamplex.com/wasora>.

# AUTHOR

Jeremy Theler <https://www.seamplex.com>
