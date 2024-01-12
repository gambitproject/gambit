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

Strategic games may also be input directly. To create a new strategic
game, select :menuselection:`File --> New --> Strategic game`,
or click the new strategic game icon on the toolbar.



Navigating a strategic game
---------------------------

Gambit displays a strategic game in table form. All players are
assigned to be either row players or column players, and the payoffs
for each entry in the strategic game table correspond to the payoffs
corresponding to the situation in which all the row players play the
strategy specified on that row for them, and all the column players
play the strategy specified on that column for them.

.. image::  screens/pd1.*
            :width: 33%
            :alt: a prisoner's dilemma game
            :align: right
            :target: _images/pd1.png

For games with two players, this presentation is by default configured
to be similar to the standard presenation of strategic games as
tables, in which one player is assigned to be the "row" player and the
other the "column" player. However, Gambit permits a more flexible
assignment, in which multiple players can be assigned to the rows and
multiple players to the columns. This is of particular use for games
with more than two players. In print, a three-player strategic game is
usually presented as a collection of tables, with one player choosing
the row, the second the column, and the third the table. Gambit
presents such games by hierarchially listing the strategies of one or
more players on both rows and columns.

The hierarchical presentation of the table is similar to that of a
contingency table in a spreadsheet application.
Here, Alice,
shown in red, has her strategies listed on the rows of the table, and
Bob, shown in blue, has his strategies listed on the columns of the
table.

The assignment of players to row and column roles is fully
customizable. To change the assignment of a player, drag the person
icon appearing to the left of the player's name on the player toolbar
to either of the areas in the payoff table displaying the strategy
labels.

.. image::  screens/pd2.*
            :width: 33%
            :alt: a prisoner's dilemma game, with contingencies in
                  list style
            :align: right
            :target: _images/pd2.png

For example, dragging the player icon from the left of Bob's name in
the list of players and dropping it on the right side of Alice's
strategy label column changes the display of the game as in
Here, the strategies are shown in a
hierarchical format, enumerating the outcomes of the game first by
Alice's (red) strategy choice, then by Bob's (blue) strategy choice.

Alternatively, the game can be displayed by listing the outcomes with
Bob's strategy choice first, then Alice's. Drag Bob's player icon and
drop it on the left side of Alice's strategy choices, and the game
display changes to organize the outcomes first by Bob's action, then
by Alice's.

The same dragging operation can be used to assign players to the
columns. Assigning multiple players to the columns gives the same
hierarchical presentation of those players' strategies. Dropping a
player above another player's strategy labels assigns him to a higher
level of the column player hierarchy; dropping a player below another
player's strategy labels assigns him to a lower level of the column
player hierarchy.

.. image::  screens/pd3.*
            :width: 33%
            :alt: another view of the same prisoner's dilemma game.
            :align: right
            :target: _images/pd3.png

As the assignment of players in the row and column
hierarchies changes, the ordering of the payoffs in each cell of the
table also changes. In all cases, the color-coding of the entries
identifies the player to whom each payoff corresponds. The ordering
convention is chosen so that for a two player game in which one player
is a row player and the other a column player, the row player's payoff
is shown first, followed by the column player, which is the most
common convention in print.



Adding players and strategies
-----------------------------

To add an additional player to the game, use the menu item
:menuselection:`Edit --> Add player`,
or the corresponding toolbar icon . The newly created player
has one strategy, by default labeled with the number :guilabel:`1`.

Gambit supports arbitrary numbers of strategies for each player. To
add a new strategy for a player, click the new strategy icon located
to the left of that player's name.

To edit the names of strategies, click on any cell in the strategic
game table where the strategy label appears, and edit the label using
the edit control.



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
