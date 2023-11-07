.. _gambit-logit:

:program:`gambit-logit`: Compute quantal response equilbria
===========================================================

:program:`gambit-logit` reads a game on standard input and computes the
principal branch of the (logit) quantal response correspondence.

The method is based on the procedure described in Turocy [Tur05]_ for
strategic games and Turocy [Tur10]_ for extensive games.
It uses standard path-following methods (as
described in Allgower and Georg's "Numerical Continuation Methods") to
adaptively trace the principal branch of the correspondence
efficiently and securely.

The method used is a predictor-corrector method, which first generates
a prediction using the differential equations describing the branch of
the correspondence, followed by a corrector step which refines the
prediction using Newton's method for finding a zero of a function. Two
parameters control the operation of this tracing. The option `-s` sets
the initial step size for the predictor phase of the tracing. This
step size is then dynamically adjusted based on the rate of
convergence of Newton's method in the corrector step. If the
convergence is fast, the step size is adjusted upward (accelerated);
if it is slow, the step size is decreased (decelerated). The option
`-a` sets the maximum acceleration (or deceleration). As described in
Turocy [Tur05]_, this acceleration helps to
efficiently trace the correspondence when it reaches its asymptotic
phase for large values of the precision parameter lambda.

In extensive games, logit quantal response equilibria are not well-defined
if an information set is not reached due to being the successor of chance
moves with zero probability.  In such games, the implementation treats
the beliefs at such information sets as being uniform across all member nodes.

.. program:: gambit-logit

.. cmdoption:: -d

   Express all output using decimal representations with the specified
   number of digits.  The default is `-d 6`.

.. cmdoption:: -s

   Sets the initial step size for the predictor phase of
   the tracing procedure. The default value is .03. The step size is
   specified in terms of the arclength along the branch of the
   correspondence, and not the size of the step measured in terms of
   lambda. So, for example, if the step size is currently .03, but the
   position of the strategy profile on the branch is changing rapidly
   with lambda, then lambda will change by much less then .03 between
   points reported by the program.

.. cmdoption:: -a

   Sets the maximum acceleration of the step size during
   the tracing procedure. This is interpreted as a multiplier. The
   default is 1.1, which means the step size is increased or decreased by
   no more than ten percent of its current value at every step. A value
   close to one would keep the step size (almost) constant at every step.

.. cmdoption:: -m

   Stop when reaching the specified value of the
   parameter lambda. By default, the tracing stops when lambda reaches
   1,000,000, which is usually suitable for computing a good
   approximation to a Nash equilibrium. For applications, such as to
   laboratory experiments, where the behavior of the correspondence for
   small values of lambda is of interest and the asymptotic behavior is
   not relevant, setting MAXLAMBDA to a much smaller value may be
   indicated.

.. cmdoption:: -l

   While tracing, compute the logit equilibrium points
   with parameter LAMBDA accurately.

.. cmdoption:: -S

   By default, the program uses behavior strategies for extensive
   games; this switch instructs the program to use reduced strategic game
   strategies for extensive games. (This has no effect for strategic
   games, since a strategic game is its own reduced strategic game.)

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -e

   By default, all points computed are output by the program. If
   this switch is specified, only the approximation to the Nash
   equilibrium at the end of the branch is output.

Computing the principal branch, in mixed strategies, of :download:`e02.nfg
<../contrib/games/e02.nfg>`, the reduced strategic form of the example
in Figure 2 of Selten (International Journal of Game Theory,
1975)::

   $ gambit-logit e02.nfg
   Compute a branch of the logit equilibrium correspondence
   Gambit version 16.1.0, Copyright (C) 1994-2023, The Gambit Project
   This is free software, distributed under the GNU GPL

   0.000000,0.333333,0.333333,0.333333,0.5,0.5
   0.022853,0.335873,0.328284,0.335843,0.501962,0.498038
   0.047978,0.338668,0.322803,0.33853,0.504249,0.495751
   0.075600,0.341747,0.316863,0.34139,0.506915,0.493085
   0.105965,0.345145,0.310443,0.344413,0.510023,0.489977
   0.139346,0.348902,0.303519,0.347578,0.51364,0.48636

   ...

   735614.794714,1,0,4.40659e-11,0.500016,0.499984
   809176.283787,1,0,3.66976e-11,0.500015,0.499985
   890093.921767,1,0,3.05596e-11,0.500014,0.499986
   979103.323545,1,0,2.54469e-11,0.500012,0.499988
   1077013.665501,1,0,2.11883e-11,0.500011,0.499989

