General concepts
================

General layout of the main window
---------------------------------

.. image::  screens/overview.*
            :width: 33%
            :alt: the default extensive game at launch
            :align: right
            :target: _images/overview.png

The frame presenting a game consists of a single main panel, which displays
the game graphically; in this case, showing the game tree of a simple
one-card poker game. Note that where applicable, information is color-coded
to match the colors assigned to the players: Fred's moves and payoffs are
presented in red, and Alice's in blue. Players are added, removed, reordered,
renamed, and recolored on the :guilabel:`Players` page of the
:guilabel:`Game properties` dialog (:menuselection:`Edit --> Game`), in the
same way as a player's strategies are edited on the :guilabel:`Edit
strategies` dialog (see :ref:`Adding, removing, and reordering strategies
<editing-strategies>`): each player has a text field for its name, a color
swatch beside it that opens a color picker when clicked, and
:guilabel:`↑`/:guilabel:`↓`/:guilabel:`✕` buttons to reorder or remove it.
A player can't be removed if it has decisions in the game, or more than one
strategy in the game's strategic representation; its :guilabel:`✕` button is
disabled, with a tooltip explaining why.

Hovering the mouse pointer over a node in the tree briefly displays a small
window showing, for each player, that player's expected payoff from that
point in the game onward, under the currently-selected profile. For a node
that is not a terminal node, this window also shows the probability of
reaching that node and its information set, the information set's value to
the player who moves there, and that player's belief about being at this
particular node within the information set.

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
-----------------------------------

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
-------------------------

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

The graphical interface reads and writes an extended format, referred to
as a "Gambit workspace".
This stores not only the
representation of the game, but also additional information, including
parameters for laying out the game tree, the colors assigned to
players, any equilibria or other analysis done on the game, and so
forth. So, for example, the workspace file can be used to store the
analysis of a game and then return to it. These files by convention
end in the extension .gbt.
The graphical interface will read files in all three formats: .gbt,
.efg, and .nfg.
