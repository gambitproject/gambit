The graphical interface
==================================

The most visible component of Gambit, especially for the new user, is
the graphical user interface. This program is an"integrated
development environment" to help visually construct games and to
investigate their strategic features.

The graphical interface is largely intended for the interactive
construction and analysis of small to medium games. Repeating the
caution from the introduction of this manual, the computation time
required for the equilibrium analysis of games increases rapidly in
the size of the game. The graphical interface is ideal for students
learning about the fundamentals of game theory, or for practitioners
prototyping games of interest. In graduating to larger applications,
users are encouraged to make use of the underlying Gambit libraries
and programs directly.  See :ref:`the chapter on command-line tools
<command-line>` for detailed documentation on these programs.



Getting started: Tutorials
-------------------------------

The documentation of the graphical interface focuses on detailing the
concepts and features of the program. To get started using the
program, a few tutorials are available:

* Building and analyzing a coordination game:

  - `Coordination game tutorial in PDF <_static/tutorial/coord.pdf>`_ 

  - `Coordination game tutorial in SWF <_static/tutorial/coord.swf>`_

* Building and analyzing a one-card poker game

  - `Poker game tutorial in PDF <_static/tutorial/poker.pdf>`_ 

  - `Poker game tutorial in SWF <_static/tutorial/poker.swf>`_

These tutorials illustrate
the use of the graphical interface in analyzing some common types of
games, and are suitable for users with any level of experience with
game theory. Students, in particular, may find some of the examples
helpful with coursework.

Once you're comfortable with some of the tutorials, this manual ought
to be helpful in filling in some of the gaps, and listing more
completely the features of the graphical interface.


General concepts
----------------

This section gives an overview of the graphical interface.



General layout of the main window
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. image::  screens/overview.*
            :width: 33%
 	    :alt: the default extensive game at launch
	    :align: right
	    :target: _images/overview.png

The frame presenting a game consists of two principal panels. The main
panel, to the right, displays the game graphically; in this case,
showing the game tree of a simple one-card poker game. To the left is
the player panel, which lists the players in the game; here, Fred and
Alice are the players. Note that where applicable, information is
color-coded to match the colors assigned to the players: Fred's moves
and payoffs are all presented in red, and Alice's in blue. The color
assigned to a player can be changed by clicking on the color icon
located to the left of the player's name on the player panel. Player
names are edited by clicking on the player's name, and editing the
name in the text control that appears.

Two additional panels are available. Selecting
:menuselection:`Tools --> Dominance` toggles
the display of an additional toolbar across the top of the window.
This toolbar controls the indication and elimination of actions or
strategies that are dominated. The use of this toolbar is discussed in
:ref:`dominated-strategies`.

Selecting :menuselection:`View --> Profiles`, 
or clicking the show profiles icon on the
toolbar, toggles the display of the list of computed strategy profiles
on the game. More on the way the interface handles the computation of
Nash equilibria and other kinds of strategy profiles is presented
in :ref:`computing-equilibria`.



Payoffs and probabilities in Gambit
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Gambit stores all payoffs in games in an arbitrary-precision format.
Payoffs may be entered as decimal numbers with arbitrarily many
decimal places. In addition, Gambit supports representing payoffs
using rational numbers. So, for example, in any place in which a
payoff may appear, either an outcome of an extensive game or a payoff
entry in a strategic game, the payoff one-tenth may be entered either
as .1 or 1/10.

The advantage of this format is that, in certain circumstances, Gambit
may be able to compute equilibria exactly. In addition, some methods
for computing equilibria construct good numerical approximations to
equilibrium points. For these methods, the computed equilibria are
stored in floating-point format. To increase the number of decimal
places shown for these profiles, click the increase decimals icon . To
decrease the number of decimal places shown, click the decrease
decimals icon .

Increasing or decreasing the number of decimals displayed in
computed strategy profiles will not have any effect on the display of
outcome payoffs in the game itself, since those are stored in
arbitrary precision.



A word about file formats
~~~~~~~~~~~~~~~~~~~~~~~~~

The graphical interface manipulates several different file formats for
representing games. This section gives a quick overview of those
formats.

Gambit has for many years supported two file formats for representing
games, one for extensive games (typically using the filename extension
.efg) and one for strategic games (typically using the filename
extension .nfg). These file formats are recognized by all Gambit
versions dating back to release 0.94 in 1995. (Users interested in the
details of these file formats can consult :ref:`file-formats`
for more information.)

Beginning with release 2005.12.xx, the graphical interface now reads
and writes a new file format, which is referred to as a"Gambit
workbook." This extended file format stores not only the
representation of the game, but also additional information, including
parameters for laying out the game tree, the colors assigned to
players, any equilibria or other analysis done on the game, and so
forth. So, for example, the workbook file can be used to store the
analysis of a game and then return to it. These files by convention
end in the extension .gbt.

The graphical interface will read files in all three formats: .gbt,
.efg, and .nfg. The "Save" and "Save as" commands, however, always
save in the Gambit workbook (.gbt) format. To save the game itself as
an extensive (.efg) or strategic (.nfg) game, use the items on the
"Export" submenu of the "File" menu. This is useful in interfacing
with older versions of Gambit, with other tools which read and write
those formats, and in using the underlying Gambit analysis command-
line tools directly, as those programs accept .efg or .nfg game files.
Users primarily interested in using Gambit solely via the graphical
interface are encouraged to use the workbook (.gbt) format.



As it is a new format, the Gambit workbook format is still under
development and may change in details. It is intended that newer
versions of the graphical interface will still be able to read
workbook files written in older formats. 



Extensive games
---------------

The graphical interface provides a flexible set of operations for
constructing and editing general extensive games. These are outlined
below.



Creating a new extensive game
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To create a new extensive game, select 
:menuselection:`File --> New --> Extensive game`, or
click on the new extensive game icon . The extensive game created is a
trivial game with two players, named by default 
:guilabel:`Player 1` and :guilabel:`Player 2`,
with one node, which is both the root and terminal node of the game.
In addition, extensive games have a special player labeled
:guilabel:`Chance`,
which is used to represent random events not controlled by any of the
strategic players in the game.


.. _adding-moves:

Adding moves
~~~~~~~~~~~~

There are two options for adding moves to a tree: drag-and-drop
and the :guilabel:`Insert move` dialog.

#. Moves can be added to the
   tree using a drag-and-drop idiom. From the player list window, drag
   the player icon located to the left of the player who will have the
   move to any terminal node in the game tree. The tree will be extended
   with a new move for that player, with two actions at the new move.
   Adding a move for the chance player is done the same way, except the
   dice icon appearing to the left of the chance player in the player
   list window is used instead of the player icon. For the chance player,
   the two actions created will each be given a probability weight of
   one-half. If the desired move has more than two actions, additional
   actions can be added by dragging the same player's icon to the move
   node; this will add one action to the move each time this is done.

.. image:: screens/insertmove.*
           :width: 33%
	   :alt: insert move dialog
	   :align: right
	   :target: _images/insertmove.png

2. Click on any terminal node in
   the tree, and select :menuselection:`Edit --> Insert move`
   to display the :guilabel:`insert move` dialog.
   The dialog is intended to read like a sentence:

   + The first control specifies the player who will make the move. The
     move can be assigned to a new player by specifying
     :guilabel:`Insert move for a new player here`.
   + The second control selects the information set to which to add the
     move. To create the move in a new information set, select 
     :guilabel:`at a new information set` for this control.
   + The third control sets the number of actions. This control is
     disabled unless the second control is set to 
     :guilabel:`at a new information set`. 
     Otherwise, it is set automatically to the number of actions at
     the selected information set.

The two methods can be useful in different contexts. 
The drag-and-drop approach
is a bit quicker to use, especially when creating trees that have few
actions at each move. The dialog approach is a bit more flexible, in
that a move can be added for a new, as-yet-undefined player, a move
can be added directly into an existing information set, and a move can
be immediately given more than two actions.

.. _copying-trees:

Copying and moving subtrees
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Many extensive games have structures that appear in multiple parts of
the tree. It is often efficient to create the structure once, and then
copy it as needed elsewhere.

Gambit provides a convenient idiom for this. Clicking on any
nonterminal node and dragging to any terminal node implements a move
operation, which moves the entire subtree rooted at the original,
nonterminal node to the terminal node.

When this same operation is done while holding down the Control 
(:kbd:`Ctrl`) key on the keyboard, the operation is turned into a copy
operation. The entire subtree rooted at the original node is copied,
starting at the terminal node. In this copy operation, each node in
the copied image is placed in the same information set as the
corresponding node in the original subtree.


See the poker tutorial on the Gambit website
(`flash version 
<http://www.gambit-project.org/doc/tutorials/poker.swf>`_;
`PDF version
<http://www.gambit-project.org/doc/tutorials/poker.pdf>`_)
for how this can be used
to good effect in speeding up the building of games.


Copying a subtree to a terminal node in that subtree is also
supported. In this case, the copying operation is halted when reaching
the terminal node, to avoid an infinite loop. Thus, this feature
can also be helpful in constructing multiple-stage games.



Removing parts of a game tree
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Two deletion operations are supported on extensive games. To delete
the entire subtree rooted at a node, click on that node and select
:menuselection:`Edit --> Delete subtree`.

To delete an individual move from the game, click on one of the direct
children of that node, and select 
:menuselection:`Edit --> Delete parent`. This operation
deletes the parent node, as well as all the children of the parent
other than the selected node. The selected child node now takes the
place of the parent node in the tree.



Managing information sets
~~~~~~~~~~~~~~~~~~~~~~~~~


Gambit provides several methods to help manage the information
structure in an extensive game.

When building a tree, new moves can be placed in a given information
set using the :ref:`Insert move dialog <adding-moves>`.
Additionally, new moves can be
created using the drag-and-drop idiom by holding down the :kbd:`Shift`
key and dragging a node in the tree. During the drag operation, the
cursor changes to the move icon . Dropping the move icon on another
node places the target node in the same information set as the node
where the drag operation began.

.. image::  screens/editnode.*
            :width: 33%
            :alt: node properties dialog
  	    :align: right
     	    :target:  _images/editnode.png

The information set to which a node belongs can also be set by
selecting :menuselection:`Edit --> Node`.  This displays the 
:guilabel:`node properties` dialog.  
The :guilabel:`Information set` dropdown defaults
to the current information set to which the node belongs, and contains
a list of all other information sets in the game which are compatible
with the node, that is, which have the same number of
actions. Additionally, the node can be moved to a new, singleton
information set by setting this dropdown to the :guilabel:`New
information set` entry.



When building out a game tree using the :ref:`drag-and-drop approach
<copying-trees>` to copying portions of the tree,
the nodes created in
the copy of the subtree remain in the same information set as the
corresponding nodes in the original subtree. In many cases, though,
these trees differ in the information available to some or all of the
players. To help speed the process of adjusting information sets in
bulk, Gambit offers a "reveal" operation, which breaks information
sets based on the action taken at a particular node. Click on a node
at which the action taken is to be made known subsequently to other
players, and select :menuselection:`Edit --> Reveal`.  This displays a
dialog listing the players in the game. Check the boxes next to the
players who observe the outcome of the move at the node, and click
:guilabel:`OK`.  The information sets at nodes below the selected one
are adjusted based on the action selected at this node.


This is an operation that is easier to see than the explain. See the
poker tutorial
(`flash version 
<http://www.gambit-project.org/doc/tutorials/poker.swf>`_;
`PDF version
<http://www.gambit-project.org/doc/tutorials/poker.pdf>`_)
for an application of the
revelation operation in conjunction with the tree-copy operation.


.. note::

   The reveal operation only has an effect at the time it is done. In
   particular, it does not enforce the separation of information sets
   based on this information during subsequent editing of the game.




Outcomes and payoffs
~~~~~~~~~~~~~~~~~~~~

Gambit supports the specification of payoffs at any node in a game
tree, whether terminal or not. Each node is created with
no outcome attached; in this case, the payoff at each node is zero to
all players. These are indicated in the game tree by the presence of
a :guilabel:`(u)` in light grey to the right of a node.

To set the payoffs at a node, double-click on the
:guilabel:`(u)` to the right
of the node. This creates a new outcome at the node, with payoffs of
zero for all players, and displays an editor to set the payoff of the
first player.

The payoff to a player for an outcome can be edited by double-clicking
on the payoff entry. This action creates a text edit control in which
the payoff to that player can be modified. Edits to the payoff can be
accepted by pressing the :kbd:`Enter` key. In addition, accepting the
payoff by pressing the :kbd:`Tab` key both stores the changes to the
player's payoff, and advances the editor to the payoff for the next
player at that outcome.

Outcomes may also be moved or copied using a drag-and-drop idiom.
Left-clicking and dragging an outcome to another node moves the
outcome from the original node to the target node. Copying an outcome
may be accomplished by doing this same action while holding down the
Control (:kbd:`Ctrl`) key on the keyboard.



When using the copy idiom described above, the action assigns the same
outcome to both the involved nodes. Therefore, if subsequently the
payoffs of the outcome are edited, the payoffs at both nodes will be
modified. To copy the outcome in such a way that the outcome at the
target node is a different outcome from the one at the source, but
with the same payoffs, hold down the :kbd:`Shift` key instead of the 
:kbd:`Control` key while dragging.

To remove an outcome from a node, click on the node, and
select :menuselection:`Edit --> Remove outcome`.



Formatting and labeling the tree
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Gambit offers some options for customizing the display of game trees.


Beginning in version 0.2005.12.12, all formatting information,
including colors, fonts, layout, labeling, and so forth, are stored
with the game in the Gambit workbook ( .gbt ) file format. This
contrasts with previous behavior of the graphical interface, in which
the settings were stored on a per-user basis, and games were displayed
with the current settings only.



Labels on nodes and branches
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The information displayed at the nodes and on the branches of the tree
can be configured by selecting :menuselection:`Format --> Labels`, 
which displays the :guilabel:`tree labels` dialog.

.. image::  screens/labels.*
            :width: 33%
	    :alt: tree labels dialog
	    :align: right
	    :target: _images/labels.png

Above and below each node, the following information can be displayed:


:guilabel:`No label`
  The space is left blank.

:guilabel:`The node's label`
  The text label assigned to the node. (This is the
  default labeling above each node.)

:guilabel:`The player's name`
  The name of the player making the move at the node.

:guilabel:`The information set's label`
  The name of the information set to
  which the node belongs.

:guilabel:`The information set's number`
  A unique identifier of the information
  set, in the form player number:information set number. (This is the
  default labeling below each node.)

:guilabel:`The realization probability`
  The probability the node is reached.
  (Only displayed when a behavior strategy is selected to be displayed
  on the tree.)

:guilabel:`The belief probability`
  The probability a player assigns to being at
  the node, conditional on reaching the information set. (Only displayed
  when a behavior strategy is selected to be displayed on the tree.)

:guilabel:`The payoff of reaching the node`
  The expected payoff to the player
  making the choice at the node, conditional on reaching the node. (Only
  displayed when a behavior strategy is selected to be displayed on the
  tree.)


Above and below each branch, the following information can be
displayed:


:guilabel:`No label`
  The space is left blank.

:guilabel:`The name of the action`
  The name of the action taken on the branch.
  (This it the default labeling above the branch.)

:guilabel:`The probability the action is played`
  For chance actions, the
  probability the branch is taken; this is always displayed. For player
  actions, the probability the action is taken in the selected profile
  (only displayed when a behavior strategy is selected to be displayed
  on the tree). In some cases, behavior strategies do not fully specify
  behavior sufficiently far off the equilibrium path; in such cases, an
  asterisk is shown for such action probabilities. (This is the default
  labeling below each branch.)

:guilabel:`The value of the action`
  The expected payoff to the player of taking
  the action, conditional on reaching the information set. (Only
  displayed when a behavior strategy is selected to be displayed on the
  tree.)



.. _gui-tree-layout:

Controlling the layout of the tree
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Gambit implements an automatic system for layout out game trees, which
provides generally good results for most games. These can be adjusted
by selecting :menuselection:`Format --> Layout`. 
The layout parameters are organized on three tabs.

.. image::  screens/layoutnodes.*
            :width: 33%
	    :alt: layout options dialog, nodes tab
	    :align: right
	    :target: _images/layoutnodes.png

The first tab,
labeled :guilabel:`Nodes`, controls the size, location, and
rendering of nodes in the tree.
Nodes can be indicated using one
of five tokens: a horizontal line (the "traditional" Gambit style from
previous versions), a box, a diamond, an unfilled circle, and a filled
circle). These can be set independently to distinguish chance and
terminal nodes from player nodes.

The sizing of nodes can be configured for best results. Gambit styling
from previous versions used the horizontal line tokens with relatively
long lines; when using the other tokens, smaller node sizes often look
better.

.. image::  screens/layoutbranches.*
            :width: 33%
	    :alt: layout options dialog, branches tab
	    :align: right
	    :target: _images/layoutbranches.png

The layout algorithm is based upon identifying the location of
terminal nodes. The vertical spacing between these nodes can be set;
making this value larger will tend to give the tree a larger vertical
extent.

The second tab,
labeled :guilabel:`Branches`, controls the display of the branches
of the tree.
The traditional Gambit way of drawing branches is a "fork-tine"
approach, in which there is a flat part at the end of each branch at
which labels are displayed. Alternatively, branches can be drawn
directly between nodes by setting :guilabel:`Draw branches` 
to using straight
lines between nodes. With this setting, labels are now displayed at
points along the (usually) diagonal branches. Labels are usually shown
horizontally; however, they can be drawn rotated parallel to the
branches by setting :guilabel:`Draw labels` to rotated.

The rotated label drawing is experimental, and does not always look
good on screen.

.. image::  screens/layoutinfosets.*
            :width: 33%
	    :alt: layout options dialog, information sets tab
	    :align: right
	    :target: _images/layoutinfosets.png
 
The length used for branches and their tines, if drawn, can be
configured. Longer branch and tine lengths give more space for longer
labels to be drawn, at the cost of giving the tree a larger horizontal
extent.

Finally, display of the information sets in the game is configured
under the tab labeled :guilabel:`Information sets`.
Members of information sets are
by default connected using a "bubble" similar to that drawn in
textbook diagrams of games. The can be modified to use a single line
to connect nodes in the same information set. In conjunction with
using lines for nodes, this can sometimes lead to a more compact
representation of a tree where there are many information sets at the
same horizontal location.

The layout of the tree may be such that members of the same
information set appear at different horizontal locations in the tree.
In such a case, by default, Gambit draws a horizontal arrow pointing
rightward or leftward to indicate the continuation of the information
set, as illustrated in the diagram nearby.

.. image::  screens/connectinfoset.*
            :width: 33%
	    :alt: information sets spanning multiple levels
	    :align: right
	    :target: _images/connectinfoset.png
 
These connections can be disabled by setting 
:guilabel:`Connect members of information
sets` to :guilabel:`only when on the same level`. 
In addition, drawing information
set indicators can be disabled entirely by setting this to invisibly
(don't draw indicators).


Selecting fonts and colors
~~~~~~~~~~~~~~~~~~~~~~~~~~

To select the font used to draw the labels in the tree, select
:menuselection:`Format --> Font`. 
The standard font selection dialog for the operating
system is displayed, showing the fonts available on the system. Since
available fonts vary across systems, when opening a workbook on a
system different from the system on which it was saved, Gambit tries
to match the font style as closely as possible when the original font
is not available.

The color-coding for each player can be changed by clicking on the
color icon to the left of the corresponding player.


Strategic games
---------------

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
~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~

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


.. _dominated-strategies:

Investigating dominated strategies and actions
----------------------------------------------

Selecting :menuselection:`Tools --> Dominance`
toggles the appearance of a toolbar which
can be used to investigate the structure of dominated strategies and
actions.



Dominated actions in extensive game
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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


.. _computing-equilibria:

Computing Nash equilibria
-------------------------

Gambit offers broad support for computing Nash equilibria in both
extensive and strategic games. To access the provided algorithms for
computing equilibria, select :menuselection:`Tools --> Equilibrium`, 
or click on the
calculate icon on the toolbar.


Selecting the method of computing equilibria
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The process of computing Nash equilibria in extensive and strategic
games is similar. This section focuses on the case of extensive games;
the process for strategic games is analogous, except the extensive
game-specific features, such as displaying the profiles on the game
tree, are not applicable.

Gambit provides guidance on the options for computing Nash equilibria
in a dialog.
The methods applicable to a particular game depend on three criteria:
the number of equilibria to compute, whether the computation is to be
done on the extensive or strategic games, and on details of the game,
such as whether the game has two players or more, and whether the game
is constant-sum.

.. image::  screens/nash.*
            :width: 33%
            :alt: dialog for computing Nash equilibria
            :align: right
            :target: _images/nash.png

The first step in finding equilibria is to specify how many equilibria
are to be found. Some algorithms for computing equilibria are adapted
to finding a single equilibrium, while others attempt to compute the
whole equilibrium set. The first drop-down in the dialog specifies how
many equilibria to compute. In this drop-down there are options for
:guilabel:`as many equilibria as possible` and, for two-player games,
:guilabel:`all equilibria`.  For some games, there exist algorithms which will
compute many equilibria (relatively) efficiently, but are not
guaranteed to find all equilibria.

To simplify this process of choosing the method to compute equilibria
in the second drop-down, Gambit provides for any game "recommended"
methods for computing one, some, and all Nash equilibria,
respectively. These methods are selected based on experience as to the
efficiency and reliability of the methods, and should generally work
well on most games. For more control over the process, the user can
select from the second drop-down in the dialog one of the appropriate
methods for computing equilibria. This list only shows the methods
which are appropriate for the game, given the selection of how many
equilibria to compute. More details on these methods are contained
in :ref:`command-line`.

.. image::  screens/computing.*
            :width: 33%
            :alt: dialog for monitoring computation of equilibria
            :align: right
            :target: _images/computing.png

Finally, for extensive games, there is an option of whether to use the
extensive or strategic game for computation. In general, computation
using the extensive game is preferred, since it is often a
significantly more compact representation of the strategic
characeteristics of the game than the reduced strategic game is.

For even moderate sized games, computation of equilibrium can be a
time-intensive process. Gambit runs all computations in the
background, and displays a dialog
showing all equilibria computed so
far. The computation can be cancelled at any time by clicking on the
cancel icon , which terminates the computation but keeps any
equilibria computed.



Viewing computed profiles in the game
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

After computing equilibria, a panel showing the list of equilibria
computed is displayed automatically. The display of this panel can be
toggled by selecting :menuselection:`View --> Profiles`,
or clicking on the playing card
icon on the toolbar. 

.. image::  screens/profiles.*
            :width: 33%
            :alt: poker game with the unique equilibrium displayed
            :align: right
            :target: _images/profiles.png

This game has a unique equilibrium in which Fred raises after Red with
probability one, and raises with probability one-third after Black.
Alice, at her only information set, plays meet with probability two-
thirds and raise with probability one-third.

This equilibrium is displayed in a table in the profiles panel. If
more than one equilibrium is found, this panel lists all equilibria
found. Equilibria computed are grouped by separate computational runs;
computing equilibria using a different method (or different settings)
will add a second list of profiles. The list of profiles displayed is
selected using the drop-down at the top left of the profiles panel; in
the screenshot, it is set to
:guilabel:`Profiles 1`. A
brief description of the method used to compute the equilibria is
listed across the top of the profiles panel.

The currently selected equilibrium is shown in bold in the profiles
listing, and information about this equilibrium is displayed in the
extensive game. In the figure, the probabilities of selecting each
action are displayed below each branch of the tree. (This is the
default Gambit setting; see :ref:`gui-tree-layout`
for configuring the labeling of trees.)
Each branch of the tree also shows a black line, the length of which
is proportional to the probability with which the action is played.

.. image::  screens/beliefs.*
            :width: 33%
            :alt: poker game with the beliefs at Alice's top node
            :align: right
            :target: _images/beliefs.png

Clicking on any node in the tree displays additional information about
the profile at that node. 
The player panel displays
information relevant to the selected node, including the payoff to all players
conditional on reaching the node, as well as information about Alice's
beliefs at the node.

The computed profiles can also be viewed in the reduced strategic
game. Clicking on the strategic game icon changes the view to the
reduced strategic form of the game, and shows the equilibrium profiles
converted to mixed strategies in the strategic game.



Computing quantal response equilibria
-------------------------------------

Gambit provides methods for computing the logit quantal response
equilibrium correspondence for extensive games [McKPal98]_
and strategic games [McKPal95]_, 
using the tracing method of [Tur05]_.

.. image::  screens/qre.*
            :width: 33%
            :alt: quantal response equilibria
            :align: right
            :target: _images/qre.png


To compute the correspondence, select :menuselection:`Tools --> Qre`.
If viewing an
extensive game, the agent quantal response equilibrium correspondence
is computed; if viewing a strategic game (including the reduced
strategic game derived from an extensive game), the correspondence is
computed in mixed strategies.

The computed correspondence values can be saved to a CSV (comma-
separated values) file by clicking the button labeled
:guilabel:`Save correspondence to .csv file`. 
This format is suitable for reading by a
spreadsheet or graphing application.



Quantal response equilibria in strategic games (experimental)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There is an experimental graphing interface for quantal response
equilibria in strategic games. 
The graph by default plots the probabilities of all strategies, color-
coded by player, as a function of the lambda parameter. The lambda
values on the horizontal axis are plotted using a sigmoid
transformation; the Graph scaling value controls the shape of this
transformation. Lower values of the scaling give more graph space to
lower values of lambda; higher values of the scaling give more space
to higher values of lambda.

.. image::  screens/logit.*
            :width: 33%
            :alt: quantal response equilibria graphing
            :align: right
            :target: _images/logit.png

The strategies graphed are indicated in the panel at the left of the
window. Clicking on the checkbox next to a strategy toggles whether it
is displayed in the graph.

The data points computed in the correspondence can be viewed (as in
the extensive game example above) by clicking on the show data icon on
the toolbar. The data points can be saved to a CSV file by clicking on
the .

To zoom in on a portion of the graph of interest, hold down the left
mouse button and drag a rectangle on the graph. The plot window zooms
in on the portion of the graph selected by that rectangle. To restore
the graph view to the full graph, click on the zoom to fit icon .

To print the graph as shown, click on the print icon . Note that this
is very experimental, and the output may not be very satisfactory yet.


Printing and exporting games
----------------------------

Gambit supports (almost) WYSIWYG (what you see is what you get) output
of both extensive and strategic games, both to a printer and to
several graphical formats. For all of these operations, the game is
drawn exactly as currently displayed on the screen, including whether
the extensive or strategic representation is used, the layout, colors
for players, dominance and probability indicators, and so forth.



Printing a game
~~~~~~~~~~~~~~~

To print the game, press :kbd:`Ctrl`-:kbd:`P`, select 
:menuselection:`File --> Print`, or click
the printer icon on the toolbar. The game is scaled so that the
printout fits on one page, while maintaining the same ratio of
horizontal to vertical size; that is, the scaling factor is the same
in both horizontal and vertical dimensions.

Note that especially for extensive games, one dimension of the tree is
much larger than the other. Typically, the extent of the tree
vertically is much greater than its horizontal extent. Because the
printout is scaled to fit on one page, printing such a tree will
generally result in what appears to be a thin line running vertically
down the center of the page. This is in fact the tree, shrunk so the
large vertical dimension fits on the page, meaning that the horizontal
dimension, scaled at the same ratio, becomes very tiny.



Saving to a graphics file
~~~~~~~~~~~~~~~~~~~~~~~~~

Gambit supports export to five graphical file formats:


+ Windows bitmaps ( .bmp )
+ JPEG, a lossy compressed format ( .jpg , .jpeg )
+ PNG, a lossless compressed format ( .png ); these are similar to
  GIFs
+ Encapsulated PostScript ( .ps )
+ Scalable vector graphics ( .svg )

To export a game to one of these formats, select
:menuselection:`File --> Export`, and
select the corresponding menu entry.

The Windows bitmap and PNG formats are generally recommended for
export, as they both are lossless formats, which will reproduce the
game image exactly as in Gambit. PNG files use a lossless compression
algorithm, so they are typically much smaller than the Windows bitmap
for the same game. Not all image viewing and manipulation tools handle
PNG files; in those cases, use the Windows bitmap output instead. JPEG
files use a compression algorithm that only approximates the original
version, which often makes it ill-suited for use in saving game
images, since it often leads to "blocking" in the image file.

For all three of these formats, the dimensions of the exported graphic
are determined by the dimensions of the game as drawn on screen. Image
export is only supported for games which are less than about 65000
pixels in either the horizontal or vertical dimensions. This is
unlikely to be a practical problem, since such games are so large they
usually cannot be drawn in such a way that a human can make sense of
them.

Encapsulated PostScript output is generally useful for inclusion in
LaTeX and other scientific document preparation systems. This is a
vector-based output, and thus can be rescaled much more effectively
than the other output formats.


