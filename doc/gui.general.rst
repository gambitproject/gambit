General concepts
================

General layout of the main window
---------------------------------

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

