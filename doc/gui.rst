.. _section-gui:

***
GUI
***

See the :ref:`installation instructions <install-cli-gui>`.

Gambit's graphical user interface provides an
"integrated development environment" to help visually construct
games and to investigate their main strategic features.

The graphical interface is intended for the interactive construction
and analysis of games. It handles building and viewing games with
thousands of nodes or contingencies, but the computation time required for
the equilibrium analysis of games can increase rapidly in the size of the
game.  Showing the strategic form of a large tree can likewise take a
long time.

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

   gui.orientation
   gui.efg.building
   gui.nfg.building
   gui.efg.viewing
   gui.nfg.viewing
   gui.dominance
   gui.nash
   gui.export
