.. _gambit-liap:

:program:`gambit-liap`: Compute Nash equilibria using function minimization
===========================================================================

:program:`gambit-liap` reads a game on standard input and computes
approximate Nash equilibria using a function minimization approach.

This procedure searches for equilibria by generating random starting
points and using conjugate gradient descent to minimize the Lyapunov
function of the game. This function is a nonnegative function which is
zero exactly at strategy profiles which are Nash equilibria.

Note that this procedure is not globally convergent. That is, it is
not guaranteed to find all, or even any, Nash equilibria.


.. program:: gambit-liap

.. cmdoption:: -d

   Express all output using decimal representations with the
   specified number of digits.

.. cmdoption:: -n

   Specify the number of starting points to randomly generate.

.. cmdoption:: -i

   .. versionadded:: 16.1.0

   Specify the maximum number of iterations in function minimization (default is 100).

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.

.. cmdoption:: -s

   Specifies a file containing a list of starting points
   for the algorithm. The format of the file is comma-separated values,
   one mixed strategy profile per line, in the same format used for
   output of equilibria (excluding the initial NE tag).

.. cmdoption:: -S

   By default, the program uses behavior strategies for extensive
   games; this switch instructs the program to use reduced strategic game
   strategies for extensive games. (This has no effect for strategic
   games, since a strategic game is its own reduced strategic game.)

.. cmdoption:: -v

   Sets verbose mode. In verbose mode, initial points, as well as
   points at which the minimization fails at a constrained local minimum
   that is not a Nash equilibrium, are all output, in addition to any
   equilibria found.

Computing an equilibrium in mixed strategies of :download:`e02.efg
<../contrib/games/e02.efg>`, the example in Figure 2 of Selten
(International Journal of Game Theory, 1975)::

   $ gambit-liap e02.nfg
   Compute Nash equilibria by minimizing the Lyapunov function
   Gambit version 16.1.0, Copyright (C) 1994-2023, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE, 0.998701, 0.000229, 0.001070, 0.618833, 0.381167
