Strategic games
===============

Gambit has full support for constructing and manipulating arbitrary
N-player strategic (also known as normal form) games.

For extensive games, Gambit automatically computes the corresponding
reduced strategic game. To view the reduced strategic game
corresponding to an extensive game, select
:menuselection:`View --> Strategic game` or
click the strategic game table icon on the toolbar.



The strategic games computed by Gambit as the reduced strategic game
of an extensive game cannot be modified directly. Instead, edit the
original extensive game; Gambit automatically recomputes the strategic
game after any changes to the extensive game.

Strategies in a reduced strategic game are assigned numeric labels for
identification.  These labels are assigned via a deterministic algorithm for
constructing the reduced strategic game from an extensive game.
Hovering the mouse pointer over a strategy's row or column label briefly
displays a small window listing the action selected at each information set
where the reduced strategy specifies an action.

Strategic games may also be input directly. To create a new strategic
game, select :menuselection:`File --> New --> Strategic game`,
or click the new strategic game icon on the toolbar.


Displaying a strategic game
===========================

Gambit displays a strategic game as a table. Each cell represents one
strategy profile: one strategy choice for every player. The cell shows
the payoffs that result when the players choose the strategies named by
that cell's row and column labels.

For a two-player game, Gambit initially uses the familiar matrix
arrangement. One player's strategies label the rows, and the other
player's strategies label the columns.

.. image:: screens/pd1.*
   :width: 50%
   :alt: A two-player strategic game with Alice's strategies on the rows and Bob's strategies on the columns.
   :align: center
   :target: _images/pd1.png

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
--------------------------

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

.. image:: screens/pd2.*
   :width: 50%
   :alt: A strategic game with Alice and Bob arranged as two levels of hierarchical row labels.
   :align: center
   :target: _images/pd2.png

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
---------------------

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

.. image:: screens/pd3.*
   :width: 50%
   :alt: The same strategic game displayed with a different ordering of its hierarchical strategy labels.
   :align: center
   :target: _images/pd3.png

Changing the row and column hierarchies may also change the order in
which payoffs appear within each cell. The colour of each payoff
continues to identify the player to whom it belongs.

.. note::

   Rearranging the table changes only the presentation of the game. It
   does not change the players, strategies, outcomes, or payoffs.


Adding players
--------------

Players are added, removed, and reordered on the :guilabel:`Players` page of
the :guilabel:`Game properties` dialog (:menuselection:`Edit --> Game`); see
:doc:`gui.general`. A newly added player has one strategy, by default
labeled with the number :guilabel:`1`.


Editing strategies
-------------------

A player's strategies can be added, removed, reordered, and renamed by
clicking any cell in the strategic game table where one of that
player's strategy labels appears. This opens an :guilabel:`Edit
strategies` dialog for that player, titled with the player's own
label, listing a row for each of the player's strategies and showing
its label.

.. _editing-strategies:

Adding, removing, and reordering strategies
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Clicking :guilabel:`+`, below the list of strategies, adds a new
strategy with an automatically generated numeric label, which can be
relabeled like any other strategy. Each strategy's row has a
:guilabel:`✕` button to remove it, and :guilabel:`↑`/:guilabel:`↓`
buttons to move it earlier or later among the player's other
strategies.

Removing a strategy does not immediately discard the payoffs at the
contingencies that use it. Instead, the row is shown disabled, with
its label struck through, and its :guilabel:`✕` button is replaced by
a :guilabel:`↺` (restore) button, so that what is about to be
destroyed remains visible -- and reversible -- until the dialog is
confirmed. Clicking :guilabel:`↺` returns the strategy, and the
payoffs at every contingency involving it, exactly as they were.
Nothing is actually deleted until :guilabel:`OK` is clicked; clicking
:guilabel:`Cancel` leaves the player's strategies entirely unchanged.
At least one strategy must remain: the last remaining strategy cannot
be removed.

Renaming versus reordering
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Typing a new label into a strategy's row, and moving that row to a
different position with the :guilabel:`↑`/:guilabel:`↓` buttons, are
two distinct operations. They can be combined, but neither substitutes
for the other:

+ Changing the text in a row renames that strategy, wherever it ends
  up in the list. The payoffs at every contingency involving it travel
  with it: renaming a strategy never changes any payoff.
+ Moving a row up or down changes the strategy's position among the
  player's other strategies, permuting the payoff table to match. Each
  strategy keeps its own payoffs as it moves: reordering a strategy
  never changes what it is called.

Because these are separate operations, two strategies can have their
labels swapped in place -- for instance, relabeling :guilabel:`Cooperate`
to :guilabel:`Defect` and :guilabel:`Defect` to :guilabel:`Cooperate` at
the same time, since all the relabeling for a player happens
simultaneously when :guilabel:`OK` is clicked -- without touching the
payoffs at any contingency. This is different from using the
:guilabel:`↑`/:guilabel:`↓` buttons to swap the *positions* of
:guilabel:`Cooperate` and :guilabel:`Defect`, which exchanges the
payoffs at the contingencies where each is chosen, while leaving each
strategy's own label attached to it.

To help keep track of which is which while editing, a strategy whose
label has been changed from what it started as is shown in italic blue
text, and shows the original label in a tooltip on hover; a newly
added strategy is shown in bold green text.

Strategy labels must be nonempty, and unique among the strategies
currently kept for the player (a removed strategy's label is not
considered). Any field that currently violates one of these rules is
highlighted, and a description of the problem is shown below the list
of strategies; the :guilabel:`OK` button is disabled until all fields
are valid.


Editing payoffs
---------------

Payoffs for each player are specified individually for each
contingency, or collection of strategies, in the game. To edit any
payoff in the table, click that cell in the table and edit the payoff.
Pressing the Escape key (:kbd:`Esc`) cancels any editing of the payoff
and restores the previous value.

To speed entry of many payoffs, as is typical when creating a new
game, accepting a payoff entry via the :kbd:`Tab` key automatically moves
the edit control to the next cell to the right. If the payoff is the
last payoff listed in a row of the table, the edit control wraps
around to the first payoff in the next row; if the payoff is in the
last row, the edit control wraps around to the first payoff in the
first row. So a strategic game payoff table can be quickly entered by
clicking on the first payoff in the upper-left cell of the table,
inputting the payoff for the first (row) player, pressing the :kbd:`Tab`
key, inputting the payoff for the second (column) player, pressing the
:kbd:`Tab` key, and so forth, until all the payoff entries in the table
have been filled.
