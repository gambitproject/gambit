.. _gambit-lcp:

:program:`gambit-lcp`: Compute equilibria in a two-player game via linear complementarity
=========================================================================================


:program:`gambit-lcp` reads a two-player game on standard input and
computes Nash equilibria by finding solutions to a linear
complementarity problem.

For extensive games, the program uses the
sequence form representation of the extensive game, as defined by
Koller, Megiddo, and von Stengel [KolMegSte94]_, and applies the
algorithm developed by Lemke. In that case, the method will find
one Nash equilibrium.

For strategic games, the program uses the method of Lemke and Howson
[LemHow64]_. In this case, the method will find all "accessible"
equilibria, i.e., those that can be found as concatenations of Lemke-Howson
paths that start at the artificial equilibrium.
There exist strategic-form games for which some equilibria cannot be found
by this method, i.e., some equilibria are inaccessible; see Shapley [Sha74]_.

In a two-player strategic game, the set of Nash equilibria can be expressed
as the union of convex sets. This program will find extreme points
of those convex sets.  See :ref:`gambit-enummixed` for a method
which is guaranteed to find all the extreme points for a strategic
game.

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

   By default, when working with the reduced strategic game, the program
   will find all equilibria accessible from the origin of the polytopes.
   This switch instructs the program to terminate when EQA equilibria
   have been found. This has no effect when using the extensive representation
   of a game, in which case the method always only returns one equilibrium.

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.


Computing an equilibrium of extensive game :download:`e02.efg
<../contrib/games/e02.efg>`, the example in Figure 2 of Selten
(International Journal of Game Theory, 1975)

   $ gambit-lcp e02.efg
   Compute Nash equilibria by solving a linear complementarity program
   Gambit version |release|, Copyright (C) 1994-2026, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1,0,1/2,1/2,1/2,1/2
