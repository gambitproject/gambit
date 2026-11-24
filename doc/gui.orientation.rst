General concepts
================

General layout of the main window
---------------------------------

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

Two additional panels are available. Selecting
:menuselection:`Tools --> Dominance` toggles
the display of an additional toolbar across the top of the window.
This toolbar controls the indication and elimination of strategies
that are dominated. The use of this toolbar is discussed in
:ref:`dominated-strategies`.

Selecting :menuselection:`View --> Profiles`,
or clicking the show profiles icon on the
toolbar, toggles the display of the list of computed strategy profiles
on the game. More on the way the interface handles the computation of
Nash equilibria and other kinds of strategy profiles is presented
in :ref:`computing-equilibria`.



.. _gui-catalog:

Opening a game from the catalog of games
----------------------------------------

Gambit comes with a catalog of games from textbooks and research articles.
To open one, click :guilabel:`Open from catalog` on the welcome screen, or
select :menuselection:`File --> Open from catalog`. The dialog lists each
game with its title, its number of players and its form, grouped under the
book or article it comes from. Typing in the search box narrows the list to
the games whose title, source or description contain every word typed; the
menus beneath it narrow it by source, form and number of players, and
:guilabel:`Sort by` lists the games by source or by title. Clicking a
source's heading hides or shows its games; holding :kbd:`Option` (:kbd:`Alt`
on Windows and Linux) while clicking hides or shows the games of every
source. The up and down arrow keys move through the list while typing.
Selecting a game shows a picture of it, with its full source and description.

A game opened from the catalog is not tied to a file. The title bar shows
its label in the catalog.  Editing the game clears the title and long description.


.. _payoffs-and-probabilities:

Payoffs and probabilities in Gambit
-----------------------------------

Gambit stores all payoffs in games in an arbitrary-precision format.
Payoffs may be entered as decimal numbers with arbitrarily many
decimal places. In addition, Gambit supports representing payoffs
using rational numbers. So, for example, in any place in which a
payoff may appear, either an outcome of an extensive game or a payoff
entry in a strategic game, the payoff one-tenth may be entered either
as .1 or 1/10. The advantage of this format is that, in certain
circumstances, Gambit may be able to compute equilibria exactly; see
:ref:`computing-equilibria` for how this interacts with the display of
computed profiles.



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
