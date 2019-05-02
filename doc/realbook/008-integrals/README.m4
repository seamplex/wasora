define(case_title, Fun with integrals)
---
title: case_title
inputs: sigma resonance lag-comparison
tags: VAR PRINT TEXT integral gauss_kronrod gauss_legendre lag_bilinear lag_euler exp
...

# case_title

Who can have fun with integrals? Well, of course we nerds do. And, indeed, I get a lot of fun when nowadays I am faced to solving complicated integrals with numerical tools such as [wasora](https://www.saemplex.com/wasora) and compare the situation to the old days of undergraduate school when *integrals* meant *pencil and paper*. Poor me!


## sigma.was

The Stefan-Boltzmann constant used in every practical radiation heat transfer can be computed from other fundamental physical constants. Of course, you may think of the Stefan-Boltzmann constant as being a fundamental constant and of any other constant as depending on $\sigma$. The way you understand the universe is up to you. This example shows how you can compute it without having to resort to special mathematical tricks in order to analytically integrate a weird function---at the cost of recurring to wasora, of course.

![Blackbody radiation](blackbody.svg)

The energy distribution of a black body radiation was proposed by Planck in his 1900 seminal paper. The power density of a black body irradiating energy of wavelenght $\lambda$ at an absolute temperature $T$ can be written as

$$ E(\lambda,T) = \frac{2 \pi h c^2}{\displaystyle \lambda^5 \left[\exp\left(\frac{hc}{\lambda k T}\right) - 1\right]} $$

where $h$ is Planck's constant, $c$ is the speed of light in vacuum and $k$ is Boltzmann's constant. The total power irradiated at temperature $T$ is thus

$$ E_b(T) = \int_0^{\infty} \frac{2 \pi h c^2}{\displaystyle \lambda^5 \left[ \exp\left( \frac{hc}{\lambda k T}\right) - 1 \right]} \, d\lambda $$

By defining a dummy variable $\xi$

$$ \xi = \frac{\lambda k T}{h c} $$

then (work out the math as an exercise)

$$ E_b(T) = \frac{2 \pi k^4}{h^3 c^2} \int_0^{\infty} \frac{1}{\xi^5 \left[\exp\left(\xi^{-1}\right) - 1\right]} \, d\xi \, \cdot \, T^4 = \sigma \cdot T^4 $$

from which an expression for the Stefan-Bolztmann constant $\sigma$ follows. An equivalent reasoning based on frequencies instead of wavelengths throws up a similar equation. The weird integral has an analytic solution in terms of $\pi$, but I am an engineer and I do not like elliptic integrals. To avoid overflows with the evaluation of the exponential in the integrand, the integration goes from $10^{-2}$ up to $\infty$. Compare the obtained results with what Google has to say about $\sigma$. 

```wasora
include(sigma.was)dnl
```

```bash
include(sigma.term)dnl
```

## resonance.was

The integral of resonance is an important parameter for computing nuclear reaction rates. It is defined as the integral over energy

$$ I_0 = \int_{E_c}^{E_0} \frac{\sigma(E)}{E} \, dE $$

where $\sigma(E)$ is the microscopic cross section of the reaction under consideration, $E_c$ is the epithermal cut-off energy (usually $E_c = 0.5 \text{eV}$) and $E_0$ is the maximum expected energy at which the reaction can occur.

The following example computes the integral of resonance for the reaction

$$ \,^{197}\text{Au} + n \rightarrow \,^{198}\text{Au} + \gamma $$

whose microscopic cross section dependence with the incident neutron energy can be obtained online from the ENDF VII library. One accepted result is

$$ I_0 = \left( 1550 \pm 28 \right) \text{barns} $$

which should be compared with the result $I_0 = 1570$ thrown by wasora almost instantaneously, even though there are more than forty thousand energies in the microscopic cross section data.

The microscopic cross section of the reaction $^{197}\text{Au}(n,\gamma)^{198}\text{Au}$ is read from text a file which was downloaded from the [Brookhaven National Laboratory webpage](http://www.nndc.bnl.gov). It is called `au117-ng-au118.dat` and contains two columns with the energy in eV in the first one and the cross section in barns in the second one. The terminal shows the actual number of lines present, although some of these lines correspond to text headers which are already commented using the hash character `#`, which wasora ignores perfectly well. The integral of resonance is computed by using the `integral` functional.


```wasora
include(resonance.was)dnl
```

```bash
include(resonance.term)dnl
```

![Integral of resonance in gold](au117-ng-au118.svg)

## lag-comparison.was

This example shows different ways to compute first-order lags. One of them is computing the convolution integral of the impulsive response and the time history of the signal. It uses a deliberately big time step so the differences are evident. Besides comparing numerical results with the analytical solution of the lag of a step function, the invocation of wasora is performed in such a way that the value of the variable `t0` is read from the commandline using the construction `$1`. Therefore, wasora expects two arguments instead of one when invoked: the input and the value for `t0`. The terminal shows that wasora is executed from a Bash script for four different values of `t0`. Alternatively, the parametric mode of wasora could have been used, but we defer that kind of computation for further examples to come.


```wasora
include(lag-comparison.was)dnl
```

```bash
include(lag-comparison.term)dnl
```

![](comparison1-0.svg)

![](comparison1-2.svg)

![](comparison1-5.svg)

![](comparison1-8.svg)


case_nav
