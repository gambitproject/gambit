.. _dominated-strategies:

Investigating dominated strategies and actions
==============================================

Selecting :menuselection:`Tools --> Dominance`
toggles the appearance of a toolbar which
can be used to investigate the structure of dominated strategies and
actions.



Dominated actions in extensive game
-----------------------------------

In extensive games, the dominance toolbar controls the elimination of actions which are conditionally dominated.

.. image::  screens/pokerdom1.*
            :width: 33%
            :alt: the poker game, with the dominance toolbar shown
            :align: right
            :target: _images/pokerdom1.png

Actions may be eliminated based on two criteria:

:guilabel:`Strict dominance`
  The action is always worse than another,
  regardless of beliefs at the information set;

:guilabel:`Strict or weak dominance`
  There is another action at the information
  set that is always at least as good as the action, and strictly better
  in some cases.

.. image::  screens/pokerdom2.*
            :width: 33%
            :alt: the poker game, with the dominated action eliminated
            :align: right
            :target: _images/pokerdom2.png

For example, in the poker game, it is strictly dominated for Fred to
choose Fold after Red. Clicking the next level icon
removes the dominated action from the game display.

The tree layout remains unchanged, including nodes which can only be
reached using actions which have been eliminated. To compress the tree
to remove the unreachable nodes, check the box labeled
:guilabel:`Show only
reachable nodes`.

For this game, no further actions can be eliminated. In general,
further steps of elimination can be done by again clicking the next
level icon. The toolbar keeps track of the number of levels of
elimination currently shown; the previous level icon moves up one
level of elimination.

.. image::  screens/pokerdom3.*
            :width: 33%
            :alt: the poker game, with only reachable actions shown
            :align: right
            :target: _images/pokerdom3.png

The elimination of multiple levels can be automated using the fast
forward icon , which iteratively eliminates dominated actions until no
further actions can be eliminated. The rewind icon restores the
display to the full game.



Dominated strategies in strategic games
---------------------------------------

The dominance toolbar operates in strategic games in the same way as
the in the extensive game. Strategies can be eliminated iteratively
based on whether they are strictly or weakly dominated.

.. image::  screens/pddom1.*
            :width: 33%
            :alt: the prisoner's dilemma example, with dominated
                  strategies indicated
            :align: right
            :target: _images/pddom1.png

When the dominance toolbar is shown, the strategic game table contains
indicators of strategies that are dominated.
In the prisoner's dilemma, the Cooperate strategy is strictly
dominated for both players. This strict dominance is indicated by the
solid "X" drawn across the corresponding strategy labels for both
players. In addition, the payoffs corresponding to the dominated
strategies are also drawn with a solid "X" across them. Thus, any
contingency in the table containing at least one "X" is a contingency
that can only be reached by at least one player playing a strategy
that is dominated.


Strategies that are weakly dominated are similarly indicated, except
the "X" shape is drawn using a thinner, dashed line instead of the
thick, solid line.

.. image::  screens/pddom2.*
            :width: 33%
            :alt: the prisoner's dilemma example, with dominated
                  strategies removed
            :align: right
            :target: _images/pddom2.png

Clicking the next level icon removes the strictly dominated strategies
from the display.
