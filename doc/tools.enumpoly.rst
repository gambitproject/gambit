:program:`gambit-enumpoly`: Compute equilibria of a game using polynomial systems of equations
==============================================================================================

:program:`gambit-enumpoly` reads a game on standard input and
computes Nash equilibria by solving systems of polynomial equations
and inequalities.

This program searches for all Nash equilibria in a strategic game
using a support enumeration approach. This approach computes all the
supports which could, in principle, be the support of a Nash
equilibrium, and then searches for a totally mixed equilibrium on that
support by solving a system of polynomial equalities and inequalities
formed by the Nash equilibrium conditions. The ordering of the
supports is done in such a way as to maximize use of previously
computed information, making it suited to the computation of all Nash
equilibria.

When the verbose switch `-v` is used, the program outputs each support
as it is considered. The supports are presented as a comma-separated
list of binary strings, where each entry represents one player. The
digit 1 represents a strategy which is present in the support, and the
digit 0 represents a strategy which is not present. Each candidate
support is printed with the label "candidate,".

Note that the subroutine to compute a solution to the system of
polynomial equations and inequalities will fail in degenerate cases.
When the verbose switch `-v` is used, these supports are identified on
standard output with the label "singular,". It is possible that there
exist equilibria, often a connected component of equilibria, on these
singular supports.


.. program:: gambit-enumpoly

.. cmdoption:: -d

   Express all output using decimal representations with the specified
   number of digits.

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -H

   By default, the program uses an enumeration method designed to
   visit as few supports as possible in searching for all equilibria.
   With this switch, the program uses a heuristic search method based on
   Porter, Nudelman, and Shoham [PNS04]_, which is designed to minimize the
   time until the first equilibrium is found. This switch only has an
   effect when solving strategic games.

.. cmdoption:: -S

   By default, the program uses behavior strategies for extensive
   games; this switch instructs the program to use reduced strategic game
   strategies for extensive games. (This has no effect for strategic
   games, since a strategic game is its own reduced strategic game.)

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.

.. cmdoption:: -v

   Sets verbose mode. In verbose mode, supports are printed on
   standard output with the label "candidate" as they are considered, and
   singular supports are identified with the label "singular." By
   default, no information about supports is printed.

Computing equilibria of the extensive game :download:`e01.efg
<../contrib/games/e01.efg>`, the example in Figure 1 of Selten
(International Journal of Game Theory, 1975) sometimes called
"Selten's horse"::

   $ gambit-enumpoly e01.efg
   Compute Nash equilibria by solving polynomial systems
   Gambit version 16.1.0, Copyright (C) 1994-2023, The Gambit Project
   Heuristic search implementation Copyright (C) 2006, Litao Wei
   This is free software, distributed under the GNU GPL

   NE,0.000000,1.000000,0.333333,0.666667,1.000000,0.000000
   NE,1.000000,0.000000,1.000000,0.000000,0.250000,0.750000
   NE,1.000000,0.000000,1.000000,0.000000,0.000000,0.000000
   NE,0.000000,1.000000,0.000000,0.000000,1.000000,0.000000

