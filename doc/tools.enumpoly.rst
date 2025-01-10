:program:`gambit-enumpoly`: Compute equilibria of a game using polynomial systems of equations
==============================================================================================

:program:`gambit-enumpoly` reads a game on standard input and
computes Nash equilibria by solving systems of polynomial equations
and inequalities.

This program searches for all Nash equilibria in a strategic game
using a support enumeration approach. This approach computes all the
supports which could, in principle, be the support of a Nash
equilibrium.  For each candidate support, it attempts to compute
totally mixed equilibria on that support by successively subdividing
the space of mixed strategy profiles or mixed behavior profiles (as appropriate).
By using the fact that the equilibrium conditions imply a collection
of equations and inequalities which can be expressed as multilinear
polynomials, the subdivision constructed is such that each cell
contains either no equilibria or exactly one equilibrium.

For strategic games, the program searches supports in the order proposed
by Porter, Nudelman, and Shoham [PNS04]_.  For two-player games, this
prioritises supports for which both players have the same number of
strategies.  For games with three or more players, this prioritises
supports which have the fewest strategies in total.  For many classes
of games, this will tend to lower the average time until finding one equilibrium,
as well as finding the second equilibrium (if one exists).

When the verbose switch `-v` is used, the program outputs each support
as it is considered. The supports are presented as a comma-separated
list of binary strings, where each entry represents one player. The
digit 1 represents a strategy which is present in the support, and the
digit 0 represents a strategy which is not present. Each candidate
support is printed with the label "candidate,".

The approach of subdividing the space of totally mixed profiles assumes
solutions to the system of equations and inequalities are isolated
points.  In the case of degeneracies in the resulting system,
When the verbose switch `-v` is used, these supports are identified on
standard output with the label "singular,".   This will occur
if there is a positive-dimensional set of equilibria which all
share the listed support.  However, the converse is not true:
not all supports labeled as "singular" will necessarily be the
support of some set of equilibria.

.. program:: gambit-enumpoly

.. cmdoption:: -d

   Express all output using decimal representations with the specified
   number of digits.

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -H

   By default, the program uses an enumeration method designed to
   visit as few supports as possible in searching for all equilibria.
   With this switch,  This switch only has an
   effect when solving strategic games.

.. cmdoption:: -S

   By default, the program uses behavior strategies for extensive
   games; this switch instructs the program to use reduced strategic game
   strategies for extensive games. (This has no effect for strategic
   games, since a strategic game is its own reduced strategic game.)

.. cmdoption:: -m

   .. versionadded:: 16.3.0

   Specify the maximum regret criterion for acceptance as an approximate Nash equilibrium
   (default is 1e-4).  See :ref:`pygambit-nash-maxregret` for interpretation and guidance.

.. cmdoption:: -e EQA

   .. versionadded:: 16.3.0

   By default, the program will search all support profiles.
   This switch instructs the program to terminate when EQA equilibria have been found.

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.

.. cmdoption:: -v

   Sets verbose mode. In verbose mode, supports are printed on
   standard output with the label "candidate" as they are considered, and
   singular supports are identified with the label "singular." By
   default, no information about supports is printed.

Computing equilibria of the strategic game :download:`e01.nfg
<../contrib/games/e01.efg>`, the example in Figure 1 of Selten
(International Journal of Game Theory, 1975) sometimes called
"Selten's horse"::

   $ gambit-enumpoly e01.nfg
   Compute Nash equilibria by solving polynomial systems
   Gambit version 16.2.0, Copyright (C) 1994-2024, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1.000000,0.000000,1.000000,0.000000,0.000000,1.000000
   NE,0.000000,1.000000,1.000000,0.000000,1.000000,0.000000
   NE,0.000000,1.000000,0.333333,0.666667,1.000000,0.000000
   NE,1.000000,0.000000,1.000000,0.000000,0.250000,0.750000
