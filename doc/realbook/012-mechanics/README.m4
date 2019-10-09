define(case_title, Newton[!,!] Lagrange & Hamilton)
---
title: case_title
inputs: pendulum period-vs-amplitude pendulum-video
tags: INCLUDE PHASE_SPACE = .= := IF ENDIF PARAMETRIC MIN MAX OUTER_STEPS HISTORY HEADER COLUMNS READ ASCII_FILE_PATH MODE derivative sin cos done end_time infinite dt root sqrt
...

case_nav

# case_title


This is a tale about mixing [GSL](http://www.gnu.org/software/gsl)  and [SUNDIALS](https://computation.llnl.gov/casc/sundials/). It continues on the next chapter.


![Newton, Lagrange and Hamilton](guys.jpg){.img-responsive}

Once upon a time there were three guys named Isaac, Joseph-Louis and William, respectively. Suddenly, they found a bob of mass\ $m$ hanging from a string of length\ $\ell$ in presence of a gravitational field of intensity\ $g$. They could see a certain angle\ $\theta$ between the string and the vertical direction. They all agreed that\ $\theta=0$ was taken when the bob was at the bottommost position and $\theta>0$ int the counter-clockwise direction.

![Simple pendulum](simple.svg){.img-responsive}

The first one said, “I think 

$$ m \ell \ddot{\theta} + m g \sin\theta = 0 $$

and as my friend Galileo told me, it can be seen the mass\ $m$ does not matter as it can be taken out of the expression without affecting\ $\dot{\theta}$ .” “Let me see if it matters or not”---replied Joseph-Louis. “It seems to me that

$$ \frac{d}{dt} \left( \frac{\partial \mathcal{L}}{\partial \dot{\theta}} \right) = \frac{\partial \mathcal{L}}{\partial \theta} $$

where

$$ \mathcal{L}(\theta, \dot{\theta}) = \frac{1}{2} m (\ell \dot{\theta})^2 + mg\ell \cos\theta $$

and apparently\ $m$ appears in both sides, so I agree with Isaac that it should not matter after all.” Finally, William said “I like your idea, but what about if

$$
\dot{\theta}   = +\frac{\partial \mathcal{H}}{\partial p_\theta}
$$

$$
\dot{p}_\theta = -\frac{\partial \mathcal{H}}{\partial \theta}
$$

where now

$$
\mathcal{H}(\theta, p_\theta) = \frac{p_\theta^2}{2m\ell^2} - mg\ell \cos\theta
$$

Mmmmmm. How can you two tell me\ $m$ does not matter?”




## pendulum.was


So, after the tailor-made joke, we thus proceed to solve the simple pendulum using the three discussed approaches. In order to do that, we prepare a main input file that defines the parameters of the problem and writes the output, but includes the filename indicated in the commandline containing the actual equations to solve. The three options of included files are:

Newton’s equations in `newton.was`, reduced from a single second-order differential equation to two first-order equations:

```wasora
include(newton.was)dnl
```

Lagrange’s equations in `lagrange.was`, computed as numerical derivatives using dummy variables of the algebraic Lagrangian written as a wasora function:

```wasora
include(lagrange.was)dnl
```

Hamilton equations in `hamilton.was`, idem:

```wasora
include(hamilton.was)dnl
```


The main input is called `pendulum.was`, and the formulation used to solve the problem is selected from the commandline using the replacement mechanism of the construction `$1` in the wasora input file:


```wasora
include(pendulum.was)dnl
```

```bash
include(pendulum.term)dnl
```


![](pendulum.svg){.img-fluid}

It is noteworthy that in both Lagrangian and Hamiltonian formulations, there is no need to manually write the analytical partial derivatives of $\mathcal{L}(\theta, \dot{\theta})$ and $\mathcal{H}(\theta, p_\theta)$. The functional `derivative` provided by wasora can be used to numerically compute the derivative, arriving at a situation where the actual differential equations that [SUNDIALS](https://computation.llnl.gov/casc/sundials/) solves numerically are also numerically computed by [GSL](http://www.gnu.org/software/gsl/). And the users---i.e. you and me---do not need to bother about the details. But even better, we can if we wanted to, because [wasora](https://www.seamplex.com/wasora), [GSL](http://www.gnu.org/software/gsl/) and [SUNDIALS](https://computation.llnl.gov/casc/sundials/) are free software so the source code is avaiable and we have the freedom to modify it.

The figure above shows the angular position $\theta$ vs. time as obtained by the three formulations. As the differential equations are not exactly the same for each case, the time steps chosen by the solver differ and the actual times where the solutions are obtain differ. And as the problem does not have an analytical solution (remember the $\theta(t)$ is not a sine because we are solving the non-linear equation and not the linear approximation $\sin \theta \approx \theta$ for small $\theta$), we cannot plot the actual error comited by each case. Nevertheless, it is clear that the three solutions coincide. It is left as an exercise to the reader---i.e. you---to show that in effect, the mass $m$ does not affect $\theta(t)$.


## period-vs-amplitude.was

```wasora
include(period-vs-amplitude.was)dnl
```

```bash
include(period-vs-amplitude.term)dnl
```


![](period-vs-amplitude.svg){.img-fluid}


## pendulum-video

```wasora
include(pendulum-video.was)dnl
```

```bash
include(pendulum-video.term)dnl
```

<video width="480" height="480" autoplay loop class="img-fluid">
 <source src="newton.webm" type="video/webm">
 <source src="newton.mp4" type="video/mp4">
 Your browser does not support the video tag.
</video> 


case_nav
