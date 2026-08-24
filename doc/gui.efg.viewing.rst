Formatting and labeling the tree
=================================

Gambit offers some options for customizing the display of game trees.


Labels on nodes and branches
-----------------------------

The information displayed at the nodes and on the branches of the tree
can be configured by selecting :menuselection:`Format --> Labels`,
which displays the :guilabel:`tree labels` dialog.

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
-----------------------------------

Gambit implements an automatic system for layout out game trees, which
provides generally good results for most games. These can be adjusted
by selecting :menuselection:`Format --> Layout`.
The layout parameters are organized on three tabs.

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
set.

These connections can be disabled by setting
:guilabel:`Connect members of information
sets` to :guilabel:`only when on the same level`.
In addition, drawing information
set indicators can be disabled entirely by setting this to invisibly
(don't draw indicators).


Selecting fonts and colors
----------------------------

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
