.. _gambit-enummixed:

:program:`gambit-enummixed`: Enumerate equilibria in a two-player game
======================================================================

:ref:`Algorithm description <enummixed>`

.. program:: gambit-enummixed

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

.. cmdoption:: -D

   Since all Nash equilibria involve only strategies which survive
   iterative elimination of strictly dominated strategies, the program
   carries out the elimination automatically prior to computation.
   This is recommended, since it almost always results in superior
   performance.
   Specifying `-D` skips the elimination step and performs the
   enumeration on the full game.

.. cmdoption:: -c

   The program outputs the extreme equilibria as it finds them,
   prefixed by the tag NE . If this option is specified, once all extreme
   equilbria are identified, the program computes the convex sets which
   make up the set of equilibria. The program then additionally outputs
   each convex set, prefixed by convex-N , where N indexes the set. The
   set of all equilibria, then, is the union of these convex sets.

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.

.. cmdoption:: -L

   Use `lrslib <http://cgm.cs.mcgill.ca/~avis/C/lrs.html>`_ by David Avis
   to carry out the enumeration process.  This is an experimental
   feature that has not been widely tested.

Computing the equilibria, in mixed strategies, of :download:`e02.nfg
<../contrib/games/e02.nfg>`, the reduced strategic form of the example
in Figure 2 of Selten (International Journal of Game Theory,
1975)

   $ gambit-enummixed e02.nfg
   Compute Nash equilibria by enumerating extreme points
   Gambit version |release|, Copyright (C) 1994-2026, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1,0,0,1,0
   NE,1,0,0,1/2,1/2

In fact, the game e02.nfg has a one-dimensional continuum of
equilibria.  This fact can be observed by examining the connectedness
information using the `-c` switch

   $ gambit-enummixed -c e02.nfg
   Compute Nash equilibria by enumerating extreme points
   Gambit version |release|, Copyright (C) 1994-2026, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1,0,0,1,0
   NE,1,0,0,1/2,1/2
   convex-1,1,0,0,1/2,1/2
   convex-1,1,0,0,1,0
