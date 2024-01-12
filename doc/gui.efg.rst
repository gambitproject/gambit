Extensive games
===============

The graphical interface provides a flexible set of operations for
constructing and editing general extensive games. These are outlined
below.



Creating a new extensive game
-----------------------------

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
------------

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
---------------------------

Many extensive games have structures that appear in multiple parts of
the tree. It is often efficient to create the structure once, and then
copy it as needed elsewhere.

Gambit provides a convenient idiom for this. Clicking on any
nonterminal node and dragging to any terminal node implements a move
operation, which moves the entire subtree rooted at the original,
nonterminal node to the terminal node.

To turn the operation into a copy operation:

+ On Windows and Linux systems, hold down the :kbd:`Ctrl` key during
  the operation.
+ On OS X, hold down the :kbd:`Cmd` key when starting the
  drag operation, then release prior to dropping.

The entire subtree rooted at the original node is copied,
starting at the terminal node. In this copy operation, each node in
the copied image is placed in the same information set as the
corresponding node in the original subtree.

Copying a subtree to a terminal node in that subtree is also
supported. In this case, the copying operation is halted when reaching
the terminal node, to avoid an infinite loop. Thus, this feature
can also be helpful in constructing multiple-stage games.



Removing parts of a game tree
-----------------------------

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
-------------------------


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

.. note::

   The reveal operation only has an effect at the time it is done. In
   particular, it does not enforce the separation of information sets
   based on this information during subsequent editing of the game.


Outcomes and payoffs
--------------------

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
--------------------------------

Gambit offers some options for customizing the display of game trees.


Labels on nodes and branches
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
^^^^^^^^^^^^^^^^^^^^^^^^^^

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
