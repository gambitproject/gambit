.. _dominated-strategies:

Investigating dominated strategies
==================================

Selecting :menuselection:`Tools --> Dominance`
toggles the appearance of a toolbar which
can be used to investigate the structure of dominated strategies.
Strategies can be eliminated iteratively
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

The elimination of multiple levels can be automated using the fast
forward icon, which iteratively eliminates dominated actions until no
further actions can be eliminated. The rewind icon restores the
display to the full game.
