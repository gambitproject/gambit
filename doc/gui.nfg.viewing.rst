Displaying a strategic game
============================

Gambit displays a strategic game as a table. Each cell represents one
strategy profile: one strategy choice for every player. The cell shows
the payoffs that result when the players choose the strategies named by
that cell's row and column labels.

For a two-player game, Gambit initially uses the familiar matrix
arrangement. One player's strategies label the rows, and the other
player's strategies label the columns.

Each row-and-column combination identifies one strategy profile. For
example, a cell in the row labelled ``Cooperate`` and the column
labelled ``Defect`` represents the outcome in which the row player
chooses ``Cooperate`` and the column player chooses ``Defect``.

The payoffs in each cell are colour-coded by player. In the standard
two-player arrangement, Gambit displays the row player's payoff first
and the column player's payoff second.

Whenever a profile has been computed and selected, hovering the mouse
pointer over any strategy's row or column label also shows that
strategy's probability under the current profile, the payoff its player
would get by playing that strategy alone against the others' current
play, and that player's overall payoff under the current profile.
Comparing this payoff-if-played figure across a player's strategies is
how to check whether the current profile is actually a best response for
that player.


Row and column hierarchies
----------------------------

Games with more than two players require more than one strategy label
to identify each row or column. Gambit handles this by allowing several
players to be assigned to the rows, several players to be assigned to
the columns, or both.

When several players are assigned to the same side of the table, their
strategy labels form a hierarchy. Each level groups together the
strategy combinations belonging to the players below it. This is
similar to the hierarchical row and column labels used in a pivot table
or a table with a multi-level index.

For example, suppose Alice and Bob are both assigned to the rows. If
Alice is above Bob in the row hierarchy, Gambit first groups the rows
by Alice's strategy and then lists Bob's strategies within each group.

Reversing their order groups the rows first by Bob's strategy and then
by Alice's. The strategy profiles and payoffs do not change; only their
arrangement in the table changes.

The same principle applies to the columns. A player placed at a higher
level of the column hierarchy forms the outer grouping, while players
at lower levels form groups within it.

This arrangement provides a single table view of games that are often
printed as a collection of separate payoff matrices. For example, a
three-player game can be displayed with one player on the rows and two
players in a column hierarchy, rather than as a separate matrix for
each strategy of the third player.


Rearranging the table
------------------------

To change the table arrangement, right-click any row or column label and
choose a player from the menu that appears. If that side of the table
already has one or more players assigned to it, the menu offers
:guilabel:`Place player before` and :guilabel:`Place player after`
submenus, each listing every player; if not, it offers a single
:guilabel:`Use as row player` (or :guilabel:`Use as column player`)
submenu instead. Gambit updates the display without changing the game
itself.

The available positions depend on the current arrangement. Choosing a
player from :guilabel:`Place player before` makes that player a higher
level in the hierarchy than the row or column that was right-clicked;
choosing one from :guilabel:`Place player after` makes it a lower level.

Changing the row and column hierarchies may also change the order in
which payoffs appear within each cell. The colour of each payoff
continues to identify the player to whom it belongs.

.. note::

   Rearranging the table changes only the presentation of the game. It
   does not change the players, strategies, outcomes, or payoffs.
