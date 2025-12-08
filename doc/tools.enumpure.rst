.. _gambit-enumpure:

:program:`gambit-enumpure`: Enumerate pure-strategy equilibria of a game
========================================================================

:ref:`Algorithm description <enumpure>`

.. versionchanged:: 14.0.2
   The effect of the `-S` switch is now purely cosmetic, determining
   how the equilibria computed are represented in the
   output. Previously, `-S` computed using the strategic game; if this
   was not specified for an extensive game, the agent form equilibria
   were returned.

.. program:: gambit-enumpure

.. cmdoption:: -S

   Report equilibria in reduced strategic form strategies, even if the
   game is an extensive game.  By default, if passed an extensive
   game, the output will be in behavior strategies.  Specifying this switch
   does not imply any change in operation internally, as pure-strategy
   equilibria are defined in terms of reduced strategic form
   strategies.

.. cmdoption:: -D

   .. versionadded:: 14.0.2

   The default output format for computed equilibria is a
   comma-separated list of strategy or action probabilities, suitable
   for postprocessing by automated tools.  Specifying `-D` instead
   causes the program to output greater detail on each equilbrium
   profile computed.

.. cmdoption:: -A

   .. versionadded:: 14.0.2

   Report agent form equilibria, that is, equilibria which consider
   only deviations at one information set.  Only has an effect for
   extensive games, as strategic games have only one information set
   per player.

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.


Computing the pure-strategy equilibria of extensive game :download:`e02.efg
<../contrib/games/e02.efg>`, the example in Figure 2 of Selten
(International Journal of Game Theory, 1975)::

   $ gambit-enumpure e02.efg
   Search for Nash equilibria in pure strategies
   Gambit version 16.4.0, Copyright (C) 1994-2025, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1,0,0,0,1,0

With the `-S` switch, the set of equilibria returned is the same,
except expressed in strategic game strategies rather than behavior
strategies::

   $ gambit-enumpure -S e02.efg
   Search for Nash equilibria in pure strategies
   Gambit version 16.4.0, Copyright (C) 1994-2025, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1,0,0,1,0

The `-A` switch considers only behavior strategy profiles where there
is no way for a player to improve his payoff by changing action at
only one information set; therefore the set of solutions is larger::

   $ gambit-enumpure -A e02.efg
   Search for Nash equilibria in pure strategies
   Gambit version 16.4.0, Copyright (C) 1994-2025, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1,0,1,0,1,0
   NE,1,0,1,0,0,1
   NE,1,0,0,1,1,0
