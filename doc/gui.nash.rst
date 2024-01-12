.. _computing-equilibria:

Computing Nash equilibria
=========================

Gambit offers broad support for computing Nash equilibria in both
extensive and strategic games. To access the provided algorithms for
computing equilibria, select :menuselection:`Tools --> Equilibrium`,
or click on the
calculate icon on the toolbar.


Selecting the method of computing equilibria
--------------------------------------------

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
-------------------------------------

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
-------------------------------------------------------------

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
