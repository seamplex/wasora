define(case_title, The Lorenz chaotic system)
---
title: case_title
inputs: lorenz
tags: PHASE_SPACE CONST PRINT HEADER .= = 
...

# case_title

This example shows how to solve the chaotic [Lorenz’ dynamical system](http://en.wikipedia.org/wiki/Lorenz_system)---you know the one of the butterfly. The differential equations are

$$
\begin{cases}
\dot{x} &= \sigma \cdot (y - x)\\
\dot{y} &= x \cdot (r - z) - y\\
\dot{z} &= xy - bz\\
\end{cases}
$$

where $\sigma=10$, $b=8/3$ and $r=28$ are the classical parameters that generate the butterfly as presented by Edward Lorenz back in his seminal 1963 paper [Deterministic non-periodic flow](http://journals.ametsoc.org/doi/abs/10.1175/1520-0469%281963%29020%3C0130%3ADNF%3E2.0.CO%3B2).

## lorenz.was

Please note the beauty of both the Lorenz system and the associated wasora input.

```wasora
include(lorenz.was)dnl
```

```bash
include(lorenz.term)dnl
```

The ability to solve the Lorenz system---that has both intrigued and inspired me since I was old enough to understand differential equations---with such simple and concise instructions shows me that indeed wasora has something to provide to the scientific/engineering community.

dnl See also the [besssugo plugin quickstart examples](http://www.talador.com.ar/jeremy/wasora/besssugo/#thelorenzsystem) for videos of applications of wasora to solve and study the Lorenz equations---and other chaotic dynamical systems.

![Lorenz as a function of time](lorenz2d.svg)

![The Lorenz attractor in phase space](lorenz3d.svg)

case_nav
