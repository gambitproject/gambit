.. _computing-equilibria:

Computing Nash equilibria
=========================

Gambit offers broad support for computing Nash equilibria in both
extensive and strategic games. To access the provided algorithms for
computing equilibria, select :menuselection:`Tools --> Equilibrium`,
or click on the
calculate icon on the toolbar.


Choosing how to compute equilibria
----------------------------------

The :guilabel:`Compute equilibria` dialog helps you choose a method for
computing Nash equilibria.  The choice is made in two steps: you state a
**Goal**, and Gambit recommends a **variant** of one of its **methods**
that suits the Goal and the game.  You can accept the recommendation or
choose any other variant in the list.  The process is the same for
extensive and strategic games.  For strategic games the variants that
need a game tree are not listed, and the features specific to extensive
games, such as displaying the profiles on the game tree, do not apply.

The terms are used as follows.

Goal
   What you want from the computation, in terms of how many equilibria
   are returned: :guilabel:`One equilibrium`, :guilabel:`Several
   equilibria`, or :guilabel:`All equilibria`.  The Goal determines the
   recommended variant and the default parameters.  It never removes
   variants from the list.

Method
   An algorithm family, named as in :ref:`algorithms` (for example
   ``lcp`` or ``simpdiv``).

Variant
   A method applied in a particular representation of the game.  Each
   variant has its own coverage, reliability and solution concept.

The Goal
~~~~~~~~

Some algorithms are adapted to finding a single equilibrium, while others
attempt to compute the whole equilibrium set, and some compute many
equilibria (relatively) efficiently without guaranteeing all of them.
:guilabel:`All equilibria` is available only for games with two players,
because Gambit has no method that guarantees all equilibria for games
with more players; for such games the option is disabled, and a note
beside it gives the reason.

The variant list
~~~~~~~~~~~~~~~~

The list shows every variant that can be applied to the game's
representations: variants that need a game tree are listed only for
extensive games.  A variant that cannot be run on the game, for example
because it is restricted to two-player games, is greyed out and cannot
be selected; hovering over it shows the reason.  A variant that can be
run but does not meet your Goal can still be selected, and the detail
pane under the list then shows a warning saying what to expect.  A star
marks the variant Gambit recommends for the current Goal.  The list can
be sorted by clicking a column heading.  The columns are:

:guilabel:`Method`
   A description of the variant and the representation it works on.

:guilabel:`Coverage`
   Which equilibria the variant can be relied on to return.
   :guilabel:`One` means a single equilibrium; :guilabel:`Some` means
   possibly several equilibria, with no promise of finding all of them;
   :guilabel:`All pure` means all equilibria in pure strategies, of
   which there may be none; and :guilabel:`All` means every equilibrium.
   :guilabel:`All` is claimed only where it is delivered in practice,
   and only ``enummixed`` qualifies: it reports every extreme
   equilibrium and describes each connected component of equilibria by
   its extreme points.  Two methods that might be expected to qualify do
   not:

   * ``enumpoly`` can characterise all equilibria in theory, but it finds
     isolated equilibria only.  It cannot represent a continuum of
     equilibria on a support, and supports it cannot resolve are skipped.
   * ``lcp`` finds only the equilibria reachable by the Lemke-Howson path
     on the strategic form (the accessible equilibria), and returns just
     one on the sequence form.

:guilabel:`Reliability`
   Whether the variant is guaranteed to return a result on a given game.
   :guilabel:`Always` means it is.  :guilabel:`Numerical failure
   possible` means it converges in theory but can fail in floating-point
   arithmetic.  :guilabel:`No guarantee` means it is a local or
   heuristic search that is not known to converge on every game.

:guilabel:`Solves for`
   The solution concept: :guilabel:`Nash`, or :guilabel:`Agent Nash`
   (see :ref:`agent-nash-equilibria`).

Selecting a row shows a description in the detail pane: the method name,
whether the arithmetic is exact or floating point, the solution concept,
any warning about your Goal, and notes on the variant.  For variants
that work on the reduced strategic form of an extensive game, the pane
also shows the number of contingencies in that form, which gives an idea
of how large the computation will be.

The recommendation
~~~~~~~~~~~~~~~~~~

The recommendation depends on the Goal and the structure of the game.
It selects a method; it does not infer properties such as uniqueness.
In particular, "several" means that the selected method searches for
multiple equilibria where possible, not that a particular number of
distinct equilibria is guaranteed.

.. list-table::
   :header-rows: 1
   :widths: 20 30 15 35

   * - Goal
     - Game
     - Method
     - Variant for an extensive game
   * - One equilibrium
     - Two players and constant sum
     - :ref:`lp`
     - Sequence form
   * - One equilibrium
     - Any other game
     - :ref:`logit`
     - Agent form
   * - Several equilibria
     - Two players
     - :ref:`lcp`
     - Strategic form (Lemke-Howson)
   * - Several equilibria
     - Three or more players
     - :ref:`simpdiv`
     - Strategic form
   * - All equilibria
     - Two players
     - :ref:`enummixed`
     - Strategic form

The recommended variant is the one of the recommended method that is
applicable to the game and meets the Goal, preferring the variant that
works on the extensive game.  Hence for :guilabel:`Several equilibria`
on a two-player extensive game the recommendation is Lemke-Howson on the
strategic form, because the sequence-form variant of ``lcp`` returns
only one equilibrium.  ``simpdiv`` is recommended over ``gnm`` for three
or more players because it is reliable, whereas ``gnm`` is less
thoroughly tested and can fail numerically.  For strategic games every
recommendation uses the strategic form.

The dialog remembers your Goal and variant for each game while it is
open: when you open the dialog again on the same game it starts with
the choices you last made, provided that variant can still be run.  This
memory is not saved with the game file.

.. _method-variants:

Why the representation is part of the choice
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

An extensive game can be analysed directly, through its behavior
strategies and sequence form, or through its reduced strategic form.  It
is tempting to regard the choice of representation as a setting that can
be applied to any method.  It is not: for many methods, applying the
method to a different representation produces a different algorithm
with different guarantees, so each combination is treated as a
separate variant.  The differences are as follows.

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Method
     - What changes between representations
   * - ``lp``
     - The algorithms are theoretically equivalent, but the single
       equilibrium returned can differ.
   * - ``lcp``
     - Theoretically different algorithms: Lemke on the sequence form
       and Lemke-Howson on the strategic form.  The sequence-form
       implementation returns one equilibrium; the strategic-form one
       returns the accessible equilibria.
   * - ``enumpoly``
     - In theory the sequence form suffices.  In practice the sequence
       form does not yet use dominance elimination, and the per-support
       solvers mean some equilibria may be found in one representation
       and not the other.
   * - ``liap``, ``enumpure``
     - Different solution concept: on the extensive game they compute
       agent Nash equilibria, not Nash equilibria.
   * - ``logit``
     - Traces the agent quantal response equilibrium on the extensive
       game, but its limit is a Nash equilibrium.

For an extensive game, computation on the extensive representation is
generally preferred when it is available, since it is often a
significantly more compact representation of the strategic
characteristics of the game than the reduced strategic game.  See
:ref:`algorithms` for descriptions and limitations of the methods.

.. _agent-nash-equilibria:

Agent Nash equilibria
~~~~~~~~~~~~~~~~~~~~~

An *agent Nash equilibrium* is a behavior profile of an extensive game at
which no agent can gain by deviating at its own information set alone.
It is not necessarily a Nash equilibrium of the game: a profile may be
stable against deviations at one information set at a time while a
player could gain by changing play at several information sets
together.

The agent-form variants of ``enumpure`` and ``liap`` compute agent Nash
equilibria, and their :guilabel:`Solves for` entry in the variant list
reads :guilabel:`Agent Nash`.  The profiles they return are labelled as
agent Nash equilibria in the description across the top of the profiles
panel (for example, "One agent Nash equilibrium in pure strategies in
extensive game"), so that they are not mistaken for Nash equilibria.  If
Nash equilibria are what you need, use the strategic-form variants, or
check the profiles returned.

Parameters
~~~~~~~~~~

Below the detail pane, a collapsible :guilabel:`Parameters` section shows
the parameters the selected variant will use.  They are shown for
information and cannot be edited in the dialog.  They depend on the Goal
only for ``enumpoly`` and ``simpdiv``.

================  ============================================================================
Method            Parameters used by the graphical interface
================  ============================================================================
``enumpure``      No method-specific parameters.
``enummixed``     No method-specific parameters.
``enumpoly``      Stop after the first equilibrium when :guilabel:`One equilibrium` is the Goal;
                  otherwise search all supports; maximum regret ``1e-4``.
``lp``            No method-specific parameters.
``lcp``           All accessible equilibria; unlimited recursion depth.
``liap``          10 random starting points; at most 1,000 iterations per point; maximum
                  regret ``1e-4``.
``logit``         Maximum regret ``1e-8``; initial step ``0.03``; maximum acceleration ``1.1``.
``simpdiv``       One random starting point when :guilabel:`One equilibrium` is the Goal,
                  otherwise 20; denominator 100; grid resize factor 2; maximum regret
                  ``1/10000000``.
``ipa``           One random perturbation.
``gnm``           One random perturbation; ending lambda ``-10``; 100 steps per support cell;
                  local Newton refinement every 3 steps, with at most 10 iterations.
``hp``            10 random priors; maximum regret ``1e-8``.
================  ============================================================================

Running the computation
~~~~~~~~~~~~~~~~~~~~~~~

For even moderate sized games, computation of equilibrium can be a
time-intensive process. Gambit runs all computations in the
background, and displays a dialog
showing all equilibria computed so
far. The computation can be cancelled at any time by clicking on the
cancel icon, which terminates the computation but keeps any
equilibria computed.



Viewing computed profiles in the game
-------------------------------------

After computing equilibria, a panel showing the list of equilibria
computed is displayed automatically. The display of this panel can be
toggled by selecting :menuselection:`View --> Profiles`,
or clicking on the playing card
icon on the toolbar.

This game has a unique equilibrium in which Fred raises after Red with
probability one, and raises with probability one-third after Black.
Alice, at her only information set, plays meet with probability two-
thirds and raise with probability one-third.

This equilibrium is displayed in a table in the profiles panel. If
more than one equilibrium is found, this panel lists all equilibria
found. Equilibria computed are grouped by separate computational runs;
computing equilibria using a different method (or different settings)
will add a second list of profiles. The list of profiles displayed is
selected using the drop-down at the top left of the profiles panel;
since only one computation has been run in this example, it shows
:guilabel:`Profiles 1`. A
brief description of the method used to compute the equilibria is
listed across the top of the profiles panel.

By default the equilibria are listed in the order in which they were
computed. Clicking on the label of a column in the profiles listing
sorts the equilibria by the probability with which that action or
strategy is played; clicking on the same column label again reverses
the order. The label of the column sorted on is highlighted and
carries an arrow, :guilabel:`▲` or :guilabel:`▼`, showing the
direction of the sort. Equilibria which agree on that
column are ordered by
comparing their profiles entry-by-entry from the leftmost column
onwards, so sorting on the first column lists the equilibria in
lexicographic order by profile. Clicking on the :guilabel:`#` label in
the top left corner of the listing restores the order in which the
equilibria were computed. Sorting only changes the order in which the
equilibria are listed: the number shown at the left of each row always
identifies the equilibrium itself, so it does not change when the list
is re-sorted.

Some methods for computing equilibria construct good numerical
approximations to equilibrium points rather than exact values; for
these methods, the computed equilibria are stored in floating-point
format. When such a profile is displayed in the profiles panel, a
:guilabel:`Decimals` control appears next to the profile list chooser,
letting you increase or decrease the number of decimal places shown.
This control only appears for profiles computed in floating-point
precision; it is hidden when the displayed profiles are exact
(rational-number) equilibria, since changing the number of decimals
would have no effect on them. Increasing or decreasing the number of
decimals displayed in computed strategy profiles will not have any
effect on the display of outcome payoffs in the game itself, since
those are always stored in arbitrary precision (see
:ref:`payoffs-and-probabilities`).

The currently selected equilibrium is shown in bold in the profiles
listing, and information about this equilibrium is displayed in the
extensive game. For this equilibrium, the probabilities of selecting each
action are displayed below each branch of the tree. (This is the
default Gambit setting; see :ref:`gui-tree-layout`
for configuring the labeling of trees.)
Each branch of the tree also shows a black line, the length of which
is proportional to the probability with which the action is played.

Hovering the mouse pointer over any node in the tree briefly displays a
popup listing, for each player, that player's expected payoff from that
point in the game onward under the currently-selected profile, color-coded
to match the player. For a node that is not a terminal node, the popup
also names the player who moves there and shows the probability of
reaching the node, the probability of reaching its information set, that
player's belief about being at this particular node conditional on having
reached the information set, and the information set's value to the
player who moves there. For a node belonging to the chance player, the
popup refers to its event instead of an information set throughout.

The computed profiles can also be viewed in the reduced strategic
game. Clicking on the strategic game icon changes the view to the
reduced strategic form of the game, and shows the equilibrium profiles
converted to mixed strategies in the strategic game. Hovering the mouse
pointer over a strategy's row or column label there shows the analogous
information for that strategy: its probability under the current
profile, its payoff if played against the others' current play, and its
player's overall payoff under the current profile.



Computing quantal response equilibria
-------------------------------------

Gambit provides methods for computing the logit quantal response
equilibrium correspondence for extensive games :cite:p:`McKPal98`
and strategic games :cite:p:`McKPal95`,
using the tracing method of :cite:p:`Tur05`.

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
