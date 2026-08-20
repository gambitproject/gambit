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

There are two options for adding moves to a tree, both reached by
right-clicking the node where the move should go: a quick per-player
submenu, and the more flexible :guilabel:`Insert move` dialog.

#. Right-clicking any terminal node in the tree and choosing
   :guilabel:`Insert move for` opens a submenu listing every player
   (each shown with its assigned colour), plus the chance player. Choosing
   a player extends the tree with a new move for that player at the node,
   with two actions. Choosing the chance player adds a chance move instead,
   with its two actions each given a probability weight of one-half.

   The player who moves at an *existing* move can be changed the same
   way: right-clicking a nonterminal node offers the same submenu,
   labeled :guilabel:`Assign this move to`, listing the other players
   to which the move can be reassigned. A move cannot currently be
   changed between the chance player and a personal player this way --
   nor can it be, at present, using any other method in the interface.

.. image:: screens/insertmove.*
           :width: 33%
           :alt: insert move dialog
           :align: right
           :target: _images/insertmove.png

2. Right-click any terminal node in
   the tree, and choose :guilabel:`Insert move`
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

Both are reached with a right-click, and can be useful in different
contexts. The per-player submenu is a bit quicker to use, especially
when creating trees that have few actions at each move. The dialog
approach is a bit more flexible, in that a move can be added for a new,
as-yet-undefined player, a move can be added directly into an existing
information set, and a move can be immediately given more than two
actions.

A single action can also be added to, or removed from, an existing
move using the :guilabel:`Edit move` dialog described next.


.. _editing-moves:

Editing moves
-------------

The properties of an existing move -- the label of its information
set, the player to which it belongs, and the actions available at it
-- can be changed by right-clicking on any node belonging to the move
and choosing :guilabel:`Edit move`. This displays the
:guilabel:`Edit move` dialog, which lists a text field for the
information set's label, a dropdown for the player to which the move
belongs, and a row for each action, showing its label and, for moves
belonging to the chance player, its probability.

Adding, removing, and reordering actions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Clicking :guilabel:`+`, below the list of actions, adds a new action
with an automatically generated numeric label, which can be relabeled
like any other action. Each action's row has a :guilabel:`✕` button
to remove it, and :guilabel:`↑`/:guilabel:`↓` buttons to move it
earlier or later among the other actions at the move.

Removing an action does not immediately discard the subtree it leads
to. Instead, the row is shown disabled, with its label struck
through, and its :guilabel:`✕` button is replaced by a
:guilabel:`↺` (restore) button, so that what is about to be
destroyed remains visible -- and reversible -- until the dialog is
confirmed. Clicking :guilabel:`↺` returns the action, and its
subtree, exactly as they were. Nothing is actually deleted until
:guilabel:`OK` is clicked; clicking :guilabel:`Cancel` leaves the move
entirely unchanged. At least one action must remain at the move: the
last remaining action cannot be removed.

Renaming versus reordering
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Typing a new label into an action's row, and moving that row to a
different position with the :guilabel:`↑`/:guilabel:`↓`
buttons, are two distinct operations. They can be combined, but
neither substitutes for the other:

+ Changing the text in a row renames that action, wherever it ends up
  in the list. The subtree it leads to goes with it: renaming an
  action never changes what happens after it is taken.
+ Moving a row up or down changes the order in which actions are
  offered at the move. Each action keeps its own subtree as it moves:
  reordering an action never changes what it is called.

Because these are separate operations, two actions can have their
labels swapped in place -- for instance, relabeling :guilabel:`Left`
to :guilabel:`Right` and :guilabel:`Right` to :guilabel:`Left` at the
same time, since all the relabeling at a move happens simultaneously
when :guilabel:`OK` is clicked -- without touching the subtrees that
follow them. This is different from using the
:guilabel:`↑`/:guilabel:`↓` buttons to swap the *positions*
of :guilabel:`Left` and :guilabel:`Right`, which exchanges their
subtrees while leaving each action's own label attached to it.

To help keep track of which is which while editing, an action whose
label has been changed from what it started as is shown in italic
blue text, and shows the original label in a tooltip on hover; a
newly added action is shown in bold green text.

Action labels must be nonempty, and unique among the actions
currently kept at the move (a removed action's label is not
considered); the information set's label, if not left blank, must
similarly be unique among the information sets belonging to the same
player. Any field that currently violates one of these rules is
highlighted, and a description of the problem is shown below the list
of actions; the :guilabel:`OK` button is disabled until all fields
are valid. For moves belonging to the chance player, the
probabilities of the actions kept at the move must also be
nonnegative numbers summing to exactly one.


.. _copying-trees:

Copying and moving subtrees
---------------------------

Many extensive games have structures that appear in multiple parts of
the tree. It is often efficient to create the structure once, and then
copy it as needed elsewhere.

Gambit provides a convenient idiom for this. Clicking on any
nonterminal node and dragging it to another node results in a
context-aware popup menu. While dragging, the node under the pointer is
outlined in green if dropping there is possible, or in red if not.
Depending on the destination node, this menu offers the option of
copying the subtree rooted at the original node, moving it entirely, or
placing the destination node in the same information set as the source
node. Moving a subtree onto one of its own descendants is never
offered (though shown, disabled), since the destination would cease to
exist partway through the move; copying onto a descendant remains
available, since the copy is made before anything is removed.


Removing parts of a game tree
-----------------------------

Two deletion operations are supported on extensive games. To delete
the entire subtree rooted at a node, right-click that node and choose
:guilabel:`Delete subtree`.

To delete an individual move from the game, right-click one of the
direct children of that node, and choose
:guilabel:`Delete parent`. This operation
deletes the parent node, as well as all the children of the parent
other than the clicked node. The clicked child node now takes the
place of the parent node in the tree.



Managing information sets
-------------------------


Gambit provides several methods to help manage the information
structure in an extensive game.

When building a tree, new moves can be placed in a given information
set using the :ref:`Insert move dialog <adding-moves>`. Additionally, a
node can be placed in the same information set as another using the
drag-and-drop idiom described in :ref:`copying-trees`: dragging one
node onto another and choosing :guilabel:`Put node in same information
set` (or, when dropping on a terminal node, :guilabel:`Insert move
using same information set`) from the popup menu.

.. image::  screens/editnode.*
            :width: 33%
            :alt: node properties dialog
            :align: right
            :target:  _images/editnode.png

The information set to which a node belongs can also be set by
right-clicking the node and choosing :guilabel:`Node properties`
(or by double-clicking the node). This displays the
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
sets based on the action taken at a particular node. Right-click the
node at which the action taken is to be made known subsequently to
other players, and choose :guilabel:`Reveal`.  This displays a
dialog listing the players in the game. Check the boxes next to the
players who observe the outcome of the move at the node, and click
:guilabel:`OK`.  The information sets at nodes below the clicked one
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
zero for all players, and pops up an editing panel for that outcome.

For nodes with existing outcomes, clicking on any of the displayed
payoffs pops up an editing panel for that outcome.

Outcomes may also be moved, or shared between nodes, using drag-and-drop.
Left-clicking and dragging an outcome to another node (highlighted, as with
moving and copying subtrees, in green or red as a valid or invalid drop
target) pops up a context-aware menu which allows the outcome to be moved to
the new node, or for the new node to use the same outcome as the original --
so that editing the payoffs at either node updates both.

To remove an outcome from a node, right-click the node, and
choose :guilabel:`Remove outcome`.



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
available fonts vary across systems, when opening a workspace on a
system different from the system on which it was saved, Gambit tries
to match the font style as closely as possible when the original font
is not available.

The color-coding for each player (and for chance) can be changed on the
:guilabel:`Players` page of the :guilabel:`Game properties` dialog
(:menuselection:`Edit --> Game`), by clicking the color swatch next to the
player's name.
