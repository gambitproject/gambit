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
:guilabel:`Compute one Nash equilibrium`, :guilabel:`Compute some Nash equilibria`,
and, for two-player games, :guilabel:`Compute all Nash equilibria`.
For some games, there exist algorithms which will
compute many equilibria (relatively) efficiently, but are not
guaranteed to find all equilibria.

To simplify this process of choosing the method to compute equilibria,
the second drop-down offers :guilabel:`Gambit's recommended method`.
The recommendation is determined by the requested number of equilibria
and the structure of the game as follows.

=======================  =============================================  ==================
Requested output         Game                                           Method
=======================  =============================================  ==================
One equilibrium          Two players and constant sum                   :ref:`lp`
One equilibrium          Any other supported game                       :ref:`logit`
Some equilibria          Two players                                    :ref:`lcp`
Some equilibria          Three or more players                          :ref:`simpdiv`
All equilibria           Two players                                    :ref:`enummixed`
=======================  =============================================  ==================

The :guilabel:`all equilibria` choice is therefore offered only for
two-player games.  The recommendation selects an algorithm; it does not
infer properties such as uniqueness.  In particular, "some" means that
the selected method searches for multiple equilibria where possible, not
that a particular number of distinct equilibria is guaranteed.

For more control, the user can select one of the other methods shown in
the second drop-down.  The list is filtered using the requested output
category and the methods' applicability.  Methods which only operate on
strategic games automatically select and lock the strategic
representation.  Otherwise an extensive game may be solved using either
its behavior representation or its reduced strategic representation.
See :ref:`algorithms` for descriptions and limitations of the methods.

The current dialog uses the following solver parameters.  They are
recorded with each computation so that the interface can expose them for
editing in a future revision.

================  ============================================================================
Method            Parameters used by the graphical interface
================  ============================================================================
``enumpure``      Enumerate all pure-strategy equilibria.
``enummixed``     Enumerate all mixed-strategy extreme equilibria.
``enumpoly``      Stop after the first equilibrium when "one" is requested; otherwise search all
                  supports; maximum regret ``1e-4``.
``lp``            No method-specific parameters.
``lcp``           Search all accessible equilibria; unlimited recursion depth.
``liap``          10 random starting points; 1,000 iterations per point; maximum regret ``1e-4``.
``logit``         Maximum regret ``1e-8``; initial step ``0.03``; maximum acceleration ``1.1``.
``simpdiv``       One random starting point when "one" is requested, otherwise 20; denominator
                  100; grid resize factor 2; maximum regret ``1/10000000``.
``ipa``           One random perturbation.
``gnm``           One random perturbation; ending lambda ``-10``; 100 steps per support cell;
                  local Newton refinement every 3 steps, with at most 10 iterations.
================  ============================================================================

.. image::  screens/computing.*
            :width: 33%
            :alt: dialog for monitoring computation of equilibria
            :align: right
            :target: _images/computing.png

For extensive games, there is an option of whether to use the
extensive or strategic game for computation when the selected method
supports both. In general, computation
using the extensive game is preferred, since it is often a
significantly more compact representation of the strategic
characteristics of the game than the reduced strategic game is.

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
equilibrium correspondence for extensive games :cite:p:`McKPal98`
and strategic games :cite:p:`McKPal95`,
using the tracing method of :cite:p:`Tur05`.

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
