.. _dominated-strategies:

Investigating dominated strategies
==================================

Selecting :menuselection:`Tools --> Dominance`
toggles the appearance of a toolbar which
can be used to investigate the structure of dominated strategies.
A :guilabel:`Hide strategies which are` dropdown at the left of this
toolbar selects whether strategies are considered dominated when they
are :guilabel:`strictly` dominated, or when they are
:guilabel:`strictly or weakly` dominated; this defaults to
:guilabel:`strictly`. Strategies can then be eliminated iteratively
based on whichever of these two notions is selected.

When the dominance toolbar is shown, the strategic game table contains
indicators of strategies that are dominated according to the selected
notion.
In the prisoner's dilemma, with the dropdown set to
:guilabel:`strictly`, the Cooperate strategy is strictly
dominated for both players. This strict dominance is indicated by the
solid "X" drawn across the corresponding strategy labels for both
players. In addition, the payoffs corresponding to the dominated
strategies are also drawn with a solid "X" across them. Thus, any
contingency in the table containing at least one "X" is a contingency
that can only be reached by at least one player playing a strategy
that is dominated.

Setting the dropdown to :guilabel:`strictly or weakly` additionally
indicates weakly dominated strategies, using the same "X" shape drawn
with a thinner, dashed line instead of the thick, solid line used for
strict dominance.

To the right of the dropdown, a status readout (for example,
:guilabel:`All strategies shown` or :guilabel:`Eliminated 1 level`)
shows how many rounds of elimination are currently applied, and four
buttons step through rounds of elimination one at a time or all at
once. Clicking :guilabel:`Next round of elimination` removes the
currently-indicated dominated strategies from the display, advancing
one round; :guilabel:`Previous round of elimination` undoes the most
recent round, restoring the strategies it removed. The elimination of
multiple rounds can be automated using :guilabel:`Eliminate
iteratively`, which repeats elimination until no further strategies of
the selected notion can be eliminated. :guilabel:`Show all strategies`
restores the display to the full game.
