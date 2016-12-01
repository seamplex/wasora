---
title: Description of the computational tool wasora
author: G. Theler <gtheler@cites-gss.com>
...


Introduction
============

Wasora is a free computational tool that essentially solves the
mathematical equations that are usually encountered in the models that
arise when studying and analyzing engineering systems. In particular,
the code history and the development team
is closely related to nuclear engineering and reactor analysis.
Nevertheless, the code provides a number of basic mathematical
algorithms and methods that make it suitable for solving problems in a
wide variety of engineering and scientific applications, especially when
dealing with dynamical systems. The main focus are parametric runs and
multidimensional optimization of parameters that are themselves the
results of the afore-mentioned models.

Even though wasora is a general mathematical framework, particular
computations (such as specific finite-element formulations of problems
or models of digital control systems) or features (such as real-time
graphical outputs or the possibility to read ad-hoc binary data formats)
may be implemented as dynamically-loadable plugins. The set of codes
that comprise the wasora code plus its plugin is also known as the
*wasora suite*.

The code is free software released under the terms of the GNU Public
License version 3 or, at your option, any later version.
Section [licensing](#license) contains further details about the license of
wasora.

License {#license}
=======

Wasora is free software—both as in free speech and as in free beer,
although the first meaning is far more important than the second one—and
is distributed under the terms of the GNU General Public License
version 3. In words of the Free Software Foundation,

> Nobody should be restricted by the software they use. There are four
> freedoms that every user should have:
>
> 1.  the freedom to use the software for any purpose,
>
> 2.  the freedom to change the software to suit your needs,
>
> 3.  the freedom to share the software with your friends and neighbors,
>     and
>
> 4.  the freedom to share the changes you make.
>
> When a program offers users all of these freedoms, we call it free
> software.
>
> Developers who write software can release it under the terms of the
> GNU GPL. When they do, it will be free software and stay free
> software, no matter who changes or distributes the program. We call
> this copyleft: the software is copyrighted, but instead of using those
> rights to restrict users like proprietary software does, we use them
> to ensure that every user has freedom.

Not only does wasora provide all the four basic freedoms to the software
user, but it also encourages her to study, understand, analyze and hack
it. And of course, to share the associated discoveries, suggestions,
improvements and fixed bugs under the terms of the GNU GPL—especially
with wasora’s original author. To sum up:

> Wasora is free software: you can redistribute it and/or modify it
> under the terms of the GNU General Public License as published by the
> Free Software Foundation, either version 3 of the License, or (at your
> option) any later version.
>
> Wasora is distributed in the hope that it will be useful, but WITHOUT
> ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
> FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
> for more details.

How to refer to wasora
======================

Wasora means “Wasora’s an Advanced Suite for Optimization & Reactor
Analysis”, which is of course a recursive acronym as in “GNU’s Not Unix”
and in “to understand recursion one has first to understand recursion.”
The code name should always be written using lowercase letters, except
when it starts a sentence. In such case, the “W” should be capitalized.
The expression “WASORA” ought to be avoided because

1.  words written in uppercase letters ANNOY READERS

2.  names written in uppercase letters remind of old-fashioned
    inflexible poorly-coded Fortran-based engineering programs

Pronunciation
-------------

The name is originally Spanish, so it should be pronounced
<span>/wɒ’sɔɹɑ/</span> although the English variation
<span>/wɒ’soʊɹɑ/</span> and even the German version
<span>/vɒ’sɔɹɑ/</span> are accepted.

Logo and graphics
-----------------

The official wasora logotype is shown in the figure below. The
original is a vector image in SVG format that can be found in the `doc`
subdirectory of the wasora [repository](https://bitbucket.org/wasora/wasora). Usage in the form
of other vector formats (e.g. PDF or EPS) is allowed. Conversion to
lossless-compressed bitmap formats (e.g. PNG or TIFF) is discouraged but
may be needed if the media format does not support vector graphics (note
that HTML does support plain SVG). Conversion to compressed bitmap
formats with pixel-level degradation (i.e. JPEG) is forbidden.

![The wasora logo, available in the `doc` subdirectory of the [repository](https://bitbucket.org/wasora/wasora).](logo.svg){#logoa}

The logo is distributed under the terms of the GNU GPLv3. It may be
freely modified as long as the distribution satifies the license and the
author of the modifications claims copyright on the changes only.

Development history {#sec:history}
===================

It was at the movies when I first heard about dynamical systems,
non-linear equations and chaos theory. The year was 1993, I was ten
years old and the movie was Jurassic Park. Dr. Ian Malcolm (the
character played by Jeff Goldblum) explained sensitivity to initial
conditions in a memorable scene, which is worth to watch again and again.
Since then, the fact that tiny variations
may lead to unexpected results has always fascinated me. During high
school I attended a very interesting course on fractals and chaos that
made me think further about complexity and its mathematical description.
Nevertheless, not until college was I able to really model and solve the
differential equations that give rise to chaotic behavior.

![Dr. Ian Malcolm (Jeff Goldblum) explaining the
concept of sensitivity to initial conditions in chaotic systems in the
1993 movie *Jurassic Park*.](jurassicpark.jpg)

In fact, initial-value ordinary differential equations arise in a great
variety of subjects in science and engineering. Classical mechanics,
chemical kinetics, structural dynamics, heat transfer analysis and
dynamical systems, amongst other disciplines, heavily rely on equations
of the form

$$\dot{\vec{x}} = \vec{F}(\vec{x})$$

During my years of undergraduate student, whenever I faced these kind of
equations, I had to choose one of the following three options:

1.  program an ad-hoc numerical method such as Euler or Runge-Kutta,
    matching the requirements of the system of equations to solve

2.  use a standard numerical library such as the GNU Scientific Libary
    and code the equations to solve into a C program (or maybe
    in Python)

3.  use a high-level system such as Octave, Maxima, or some non-free
    (and worse) programs[^5]

Of course, each option had its pros and its cons. But none provided the
combination of advantages I was looking for, namely flexibility (option
one), efficiency (option two) and reduced input work (partially given by
option three). Back in those days I ended up wandering between options
one and two, depending on the type of problem I had to solve. However,
even though one can with some effort make the code read some parameters
from a text file, any other drastic change usually requires a
modification in the source code—some times involving a substantial
amount of work—and a further recompilation of the code. This was what I
most disliked about this way of working, but I could nevertheless live
with it.

Regardless of this situation, during my last year of Nuclear
Engineering, I ran into a nuclear reactor model that especially called
my attention and forced me to re-think the ODE-solving problem issue.
The model was implemented in a certain non-free software which I had
been told was the actual panacea for the engineering community—and yet I
was using for the very first time. When I opened the file and took a
look at something that I was told was a graphical representation of the
model, I was not able to understand any of the several screens the model
contained. Afterward, somebody explained to me that a set of
unintelligible blocks that were somehow interconnected in a rather
cumbersome way was how the reactor power was computed. I wish I had a
copy of the screen in order to illustrate how shocking it was to me.

The equation represented by what seemed to me as a complex topology
problem was as simple as @stability-nucengdes

$$d\phi/dt = (\rho - \beta)/\Lambda + \sum_{i=1}^{I} \lambda_i \cdot c_i$$

My first reaction was to ask why someone would prefer such a cumbersome
representation instead of writing something like

```wasora
phi_dot .= (rho - beta)/Lambda * phi + sum(lambda(i)*c(i), i, 1, I)
```

in a plain-text file and let a computer program parse and solve it. I do
not remember what the teacher’s answer was, and I still do not
understand why would somebody prefer to solve a very simple differential
equation by drawing blocks and connecting them with a mouse with no
mathematical sense whatsoever.

That morning I realized that in order to transform a user-defined string
representing a differential equation into something that an ODE-solving
library such as the GNU Scientific Library would understand, only a good
algebraic parser plus some simple interface routines were needed. The
following two years were very time-consuming for me, so I was not able
to undertake such a project. Nevertheless, eventually I earned a
Master’s Degree in 2008 @theler2008 and afterward my focus shifted away
from academic projects into the nuclear industry and some gaps of time
for freelance programming popped up. I started to write wasora from
scratch in my free time, and one of the first features I included was an
adaptation of a small algebraic parser posted online[^6] (which should
be replaced by a more efficient tree-based parser), freely available
under the Creative Commons License. Before I became aware, I was very
close to arriving at a tool that would have met my needs when I was an
engineering student. Moreover, a tool like this one would have been
extremely helpful during the course on non-linear dynamics I took back
in 1999. With some luck, it would also meet somebody else’s needs as
well. This is how wasora entered into the scene.
