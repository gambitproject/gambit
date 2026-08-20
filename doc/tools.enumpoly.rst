.. _gambit-enumpoly:

:program:`gambit-enumpoly`
==========================

Compute equilibria of a game using polynomial systems of equations
See the :ref:`algorithm description <enumpoly>` for full details.

When the verbose switch `-V` is used, the program outputs each support
as it is considered. The supports are presented as a comma-separated
list of binary strings, where each entry represents one player. The
digit 1 represents a strategy which is present in the support, and the
digit 0 represents a strategy which is not present. Each candidate
support is printed with the label "candidate,".

The approach of subdividing the space of totally mixed profiles assumes
solutions to the system of equations and inequalities are isolated
points.  In the case of degeneracies in the resulting system,
When the verbose switch `-V` is used, these supports are identified on
standard output with the label "singular,".   This will occur
if there is a positive-dimensional set of equilibria which all
share the listed support.  However, the converse is not true:
not all supports labeled as "singular" will necessarily be the
support of some set of equilibria.

Independently of degeneracy, the search on a single support can require
examining an impractically large number of subdivided cells before it
can either confirm or rule out an equilibrium.  To bound the time spent
on any one support, the number of cells examined is capped (see
:option:`gambit-enumpoly -r`).  When this limit is reached, the search for that support
is abandoned, any equilibria already confirmed on it are still reported,
and (in verbose mode) the support is identified with the label
"budget-exceeded,".

.. program:: gambit-enumpoly

.. cmdoption:: -d

   Express all output using decimal representations with the specified
   number of digits.

.. cmdoption:: -h

   Prints a help message listing the available options.

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

.. cmdoption:: -r RECTANGLES

   .. versionadded:: 17.0.0

   Specify the maximum number of cells to examine when searching for equilibria
   on a single support, before giving up on that support (default 20000).  See
   the :ref:`algorithm description <enumpoly>` for why this is necessary.

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.

.. cmdoption:: -V, --verbose

   Sets verbose mode. In verbose mode, supports are printed on
   standard output with the label "candidate" as they are considered, and
   singular supports are identified with the label "singular." By
   default, no information about supports is printed.

.. cmdoption:: -v, --version

   Prints version information and exits.

Computing equilibria of the example in Figure 1 of :cite:p:`Sel75`, sometimes called
"Selten's horse"::

   $ gambit-enumpoly -S catalog/journals/ijgt/selten1975/fig1.efg
   Compute Nash equilibria by solving polynomial systems
   Gambit version |release|, Copyright (C) 1994-2026, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1.000000,0.000000,1.000000,0.000000,0.000000,1.000000
   NE,0.000000,1.000000,1.000000,0.000000,1.000000,0.000000
   NE,0.000000,1.000000,0.333333,0.666667,1.000000,0.000000
   NE,1.000000,0.000000,1.000000,0.000000,0.250000,0.750000
