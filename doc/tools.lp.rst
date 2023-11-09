.. _gambit-lp:

:program:`gambit-lp`: Compute equilibria in a two-player constant-sum game via linear programming
=================================================================================================

:program:`gambit-lp` reads a two-player constant-sum game on standard input
and computes a Nash equilibrium by solving a linear program. The
program uses the sequence form formulation of Koller, Megiddo, and von
Stengel [KolMegSte94]_ for extensive games.

While the set of equilibria in a two-player constant-sum strategic
game is convex, this method will only identify one of the extreme
points of that set.


.. program:: gambit-lp

.. cmdoption:: -d

   By default, this program computes using exact
   rational arithmetic. Since the extreme points computed by this method
   are guaranteed to be rational when the payoffs in the game are
   rational, this permits exact computation of an equilibrium.
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

   .. versionadded:: 14.0.3

   The default output format for computed equilibria is a
   comma-separated list of strategy or action probabilities, suitable
   for postprocessing by automated tools.  Specifying `-D` instead
   causes the program to output greater detail on each equilbrium
   profile computed.

.. cmdoption:: -P

   By default, the program computes Nash equilibria in an extensive
   game. This switch instructs the program to find only equilibria
   which are subgame perfect.  (This has no effect for strategic
   games, since there are no proper subgames of a strategic game.)

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.

Computing an equilibrium of the game :download:`2x2const.nfg
<../contrib/games/2x2const.nfg>`, a game with two players with two
strategies each, with a unique equilibrium in mixed strategies::

   $ gambit-lp 2x2const.nfg
   Compute Nash equilibria by solving a linear program
   Gambit version 16.1.0, Copyright (C) 1994-2023, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1/3,2/3,1/3,2/3

