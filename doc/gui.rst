.. _section-gui:

***********************
The graphical interface
***********************

Gambit's graphical user interface provides an
"integrated development environment" to help visually construct
games and to investigate their main strategic features.

The graphical interface is largely intended for the interactive
construction and analysis of small to medium games. Repeating the
caution from the introduction of this manual, the computation time
required for the equilibrium analysis of games increases rapidly in
the size of the game. The graphical interface is ideal for students
learning about the fundamentals of game theory, or for practitioners
prototyping games of interest.

In graduating to larger applications,
users are encouraged to make use of the underlying Gambit libraries
and programs directly.  For greater control over computing Nash and
quantal response equilibria of a game, see the section on
:ref:`the command-line tools <command-line>`.
To build larger games or to explore parameter spaces of a game
systematically, it is recommended to use
:ref:`the Python package <pygambit>`.


.. toctree::
   :maxdepth: 2

   gui.general
   gui.efg
   gui.nfg
   gui.dominance
   gui.nash
   gui.export
