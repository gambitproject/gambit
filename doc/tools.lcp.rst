.. _gambit-lcp:

:program:`gambit-lcp`: Compute equilibria in a two-player game via linear complementarity
=========================================================================================


:ref:`Algorithm description <lcp>`

.. program:: gambit-lcp

.. cmdoption:: -d

   By default, this program computes using exact
   rational arithmetic. Since the extreme points computed by this method
   are guaranteed to be rational when the payoffs in the game are
   rational, this permits exact computation of the equilibrium set.
   Computation using rational arithmetic is in general slow, however. For
   most games, acceptable results can be obtained by computing using the
   computer's native floating-point arithmetic. Using this flag enables
   computation in floating-point, and expresses all output using decimal
   representations with the specified number of digits.

.. cmdoption:: -S

   By default, the program uses behavior strategies for extensive
   games; this switch instructs the program to use reduced strategic game
   strategies for extensive games. (This has no effect for strategic
   games, since a strategic game is its own reduced strategic game.)

.. cmdoption:: -D

   .. versionadded:: 14.0.2

   The default output format for computed equilibria is a
   comma-separated list of strategy or action probabilities, suitable
   for postprocessing by automated tools.  Specifying `-D` instead
   causes the program to output greater detail on each equilbrium
   profile computed.

.. cmdoption:: -e EQA

   By default, the program will find all equilibria accessible from
   the origin of the polytopes.  This switch instructs the program
   to terminate when EQA equilibria have been found.

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.


Computing an equilibrium of extensive game :download:`e02.efg
<../contrib/games/e02.efg>`, the example in Figure 2 of Selten
(International Journal of Game Theory, 1975)::

   $ gambit-lcp e02.efg
   Compute Nash equilibria by solving a linear complementarity program
   Gambit version 16.4.0, Copyright (C) 1994-2025, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1,0,1/2,1/2,1/2,1/2
