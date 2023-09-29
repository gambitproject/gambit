User guide
----------

Example: One-shot trust game with binary actions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

[Kre90]_ introduced a game commonly referred to as the **trust game**.
We will build a one-shot version of this game using ``pygambit``'s game transformation
operations.

There are two players, a **Buyer** and a **Seller**.
The Buyer moves first and has two actions, **Trust** or **Not trust**.
If the Buyer chooses **Not trust**, then the game ends, and both players
receive payoffs of 0.
If the Buyer chooses **Trust**, then the Seller has a choice with two actions,
**Honor** or **Abuse**.
If the Seller chooses **Honor**, both players receive payoffs of 1;
if the Seller chooses **Abuse**, the Buyer receives a payoff of -1 and the Seller
receives a payoff of 2.

We create a game with an extensive representation using :py:meth:`.Game.new_tree`::

  In [1]: import pygambit as gbt

  In [2]: g = gbt.Game.new_tree(players=["Buyer", "Seller"], title="One-shot trust game, after Kreps (1990)")

The tree of the game contains just a root node, with no children::

  In [3]: g.root
  Out[3]: Out[3]: <Node [0] '' in game 'One-shot trust game, after Kreps (1990)'>

  In [4]: g.root.children
  Out[4]: []

To extend a game from an existing terminal node, use :py:meth:`.Game.append_move`::

  In [5]: g.append_move(g.root, "Buyer", ["Trust", "Not trust"])

  In [6]: g.root.children
  Out[6]: [<Node [2] '' in game 'One-shot trust game, after Kreps (1990)'>, <Node [3] '' in game 'One-shot trust game, after Kreps (1990)'>]

We can then also add the Seller's move in the situation after the Buyer chooses Trust::

  In [7]: g.append_move(g.root.children[0], "Seller", ["Honor", "Abuse"])

Now that we have the moves of the game defined, we add payoffs.  Payoffs are associated with
:py:class:`.Outcome`s; each :py:class:`Outcome` has a vector of payoffs, one for each player,
and optionally an identifying text label.  First we add the outcome associated with the
Seller proving themselves trustworthy::

  In [8]: g.set_outcome(g.root.children[0].children[0], g.add_outcome([1, 1], label="Trustworthy"))

Next, the outcome associated with the scenario where the Buyer trusts but the Seller does
not return the trust::

  In [9]: g.set_outcome(g.root.children[0].children[1], g.add_outcome([-1, 2], label="Untrustworthy"))

And, finally the outcome associated with the Buyer opting out of the interaction::

  In [10]: g.set_outcome(g.root.children[1], g.add_outcome([0, 0], label="Opt-out"))

Nodes without an outcome attached are assumed to have payoffs of zero for all players.
Therefore, adding the outcome to this latter terminal node is not strictly necessary in Gambit,
but it is useful to be explicit for readability.

.. [Kre90] Kreps, D. (1990) "Corporate Culture and Economic Theory."
   In J. Alt and K. Shepsle, eds., *Perspectives on Positive Political Economy*,
   Cambridge University Press.


Example: A one-card poker game with private information
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To illustrate games in extensive form, [Mye91]_ presents a one-card poker game.
A version of this game also appears in [RUW08]_, as a classroom game under the
name "stripped-down poker".  This is perhaps the simplest interesting game
with imperfect information.

In our version of the game, there are two players, **Alice** and **Bob**.
There is a deck of cards, with equal numbers of **King** and **Queen** cards.
The game begins with each player putting $1 in the pot.
One card is dealt at random to Alice; Alice observes her card but Bob does not.
After Alice observes her card, she can choose either to **Raise** or to **Fold**.
If she chooses to Fold, Bob wins the pot and the game ends.
If she chooses to Raise, she adds another $1 to the pot.
Bob then chooses either to **Meet** or **Pass**.  If he chooses to Pass,
Alice wins the pot and the game ends.
If he chooses to Meet, he adds another $1 to the pot.
There is then a showdown, in which Alice reveals her card.  If she has a King,
then she wins the pot; if she has a Queen, then Bob wins the pot.

We can build this game using the following script::

        g = gbt.Game.new_tree(players=["Alice", "Bob"],
                              title="One card poker game, after Myerson (1991)")
        g.append_move(g.root, g.players.chance, ["King", "Queen"])
        for node in g.root.children:
            g.append_move(node, "Alice", ["Raise", "Fold"])
        g.append_move(g.root.children[0].children[0], "Bob", ["Meet", "Pass"])
        g.append_infoset(g.root.children[1].children[0],
                         g.root.children[0].children[0].infoset)
        alice_winsbig = g.add_outcome([2, -2], label="Alice wins big")
        alice_wins = g.add_outcome([1, -1], label="Alice wins")
        bob_winsbig = g.add_outcome([-2, 2], label="Bob wins big")
        bob_wins = g.add_outcome([-1, 1], label="Bob wins")
        g.set_outcome(g.root.children[0].children[0].children[0], alice_winsbig)
        g.set_outcome(g.root.children[0].children[0].children[1], alice_wins)
        g.set_outcome(g.root.children[0].children[1], bob_wins)
        g.set_outcome(g.root.children[1].children[0].children[0], bob_winsbig)
        g.set_outcome(g.root.children[1].children[0].children[1], alice_wins)
        g.set_outcome(g.root.children[1].children[1], bob_wins)

All extensive games have a chance (or nature) player, accessible as
``.Game.players.chance``.  Moves belonging to the chance player can be added in the same
way as to personal players.  At any new move created for the chance player, the action
probabilities default to uniform randomization over the actions at the move.

In this game, information structure is important.  Alice knows her card, so the two nodes
at which she has the move are part of different information sets.  The loop::

        for node in g.root.children:
            g.append_move(node, "Alice", ["Raise", "Fold"])

causes each of the newly-appended moves to be in new information sets.  In contrast, Bob
does not know Alice's card, and therefore cannot distinguish between the two nodes at which
he has the decision.   This is implemented in the following lines::

        g.append_move(g.root.children[0].children[0], "Bob", ["Meet", "Pass"])
        g.append_infoset(g.root.children[1].children[0],
                         g.root.children[0].children[0].infoset)

The call :py:meth:`.Game.append_infoset` adds a move at a terminal node as part of
an existing information set (represented in `pygambit` as an :py:class:`.Infoset`).


.. [Mye91] Myerson, Roger B. (1991) *Game Theory: Analysis of Conflict*.
   Cambridge: Harvard University Press.

.. [RUW08] Reiley, David H., Michael B. Urbancic and Mark Walker. (2008)
   "Stripped-down poker: A classroom game with signaling and bluffing."
   *The Journal of Economic Education* 39(4): 323-341.



Building a strategic game
~~~~~~~~~~~~~~~~~~~~~~~~~

Games in strategic form, also referred to as normal form, are represented solely
by a collection of payoff tables, one per player.  The most direct way to create
a strategic game is via :py:meth:`.Game.from_arrays`.  This function takes one
n-dimensional array per player, where n is the number of players in the game.
The arrays can be any object that can be indexed like an n-times-nested Python list;
so, for example, NumPy arrays can be used directly.

For example, to create a standard prisoner's dilemma game in which the cooperative
payoff is 8, the betrayal payoff is 10, the sucker payoff is 2, and the noncooperative
payoff is 5::

  In [1]: import numpy as np

  In [2]: m = np.array([[8, 2], [10, 5]])

  In [3]: g = gbt.Game.from_arrays(m, np.transpose(m))

  In [4]: g
  Out[4]:
  NFG 1 R "Untitled strategic game" { "1" "2" }

  { { "1" "2" }
  { "1" "2" }
  }
  ""

  {
  { "" 8, 8 }
  { "" 10, 2 }
  { "" 2, 10 }
  { "" 5, 5 }
  }
  1 2 3 4

The arrays passed to :py:meth:`.Game.from_arrays` are all indexed in the same sense, that is,
the top level index is the choice of the first player, the second level index of the second player,
and so on.  Therefore, to create a two-player symmetric game, as in this example, the payoff matrix
for the second player is transposed before passing to :py:meth:`.Game.from_arrays`.


Representation of numerical data of a game
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Payoffs to players and probabilities of actions at chance information sets are specified
as numbers.  Gambit represents the numerical values in a game in exact precision,
using either decimal or rational representations.

To illustrate, we consider a trivial game which just has one move for the chance player::

  In  [1]: import pygambit as gbt

  In  [2]: g = gbt.Game.new_tree()

  In  [3]: g.append_move(g.root, g.players.chance, 3)

  In  [4]: [act.prob for act in g.root.infoset.actions]
  Out [4]: [Rational(1, 3), Rational(1, 3), Rational(1, 3)]

The default when creating a new move for chance is that all actions are chosen with
equal probability.  These probabilities are represented as rational numbers,
using `pygambit`'s :py:class:`.Rational` class, which is derived from Python's
`fractions.Fraction`.  Numerical data can be set as rational numbers::

  In  [5]: g.set_chance_probs(g.root.infoset,
      ...: [gbt.Rational(1, 4), gbt.Rational(1, 2), gbt.Rational(1, 4)])

  In  [6]: [act.prob for act in g.root.infoset.actions]
  Out [6]: [Rational(1, 4), Rational(1, 2), Rational(1, 4)]

They can also be explicitly specified as decimal numbers::

  In  [7]: g.set_chance_probs(g.root.infoset,
      ...: [gbt.Decimal(".25"), gbt.Decimal(".50"), gbt.Decimal(".25")]

  In  [8]: [act.prob for act in g.root.infoset.actions]
  Out [8]: [Decimal('0.25'), Decimal('0.50'), Decimal('0.25')]

Although the two representations above are mathematically equivalent, `pygambit`
remembers the format in which the values were specified.

Expressing rational or decimal numbers as above is verbose and tedious.
`pygambit` offers a more concise way to express numerical data in games:
when setting numerical game data, `pygambit` will attempt to convert text strings to
their rational or decimal representation.  The above can therefore be written
more compactly using string representations::

  In  [9]: g.set_chance_probs(g.root.infoset, ["1/4", "1/2", "1/4"])

  In  [10]: [act.prob for act in g.root.infoset.actions]
  Out [10]: [Rational(1, 4), Rational(1, 2), Rational(1, 4)]

  In  [11]: g.set_chance_probs(g.root.infoset, [".25", ".50", ".25"])

  In  [12]: [act.prob for act in g.root.infoset.actions]
  Out [12]: [Decimal('0.25'), Decimal('0.50'), Decimal('0.25')]

As a further convenience, `pygambit` will accept Python `int` and `float` values.
`int` values are always interpreted as :py:class:`.Rational` values.
`pygambit` attempts to render `float` values in an appropriate :py:class:`.Decimal`
equivalent.  In the majority of cases, this creates no problems.
For example,::

  In  [13]: g.set_chance_probs(g.root.infoset, [.25, .50, .25])

  In  [14]: [act.prob for act in g.root.infoset.actions]
  Out [14]: [Decimal('0.25'), Decimal('0.5'), Decimal('0.25')]

However, rounding can cause difficulties when attempting to use `float` values to
represent values which do not have an exact decimal representation::

  In  [15]: g.set_chance_probs(g.root.infoset, [1/3, 1/3, 1/3])
  ValueError: set_chance_probs(): must specify non-negative probabilities that sum to one

This behavior can be slightly surprising, especially in light of the fact that
in Python,::

  In  [16]: 1/3 + 1/3 + 1/3
  Out [16]: 1.0

In checking whether these probabilities sum to one, `pygambit` first converts each
of the probabilities to a :py:class:`.Decimal` representation, via the following method::

  In  [17]: gbt.Decimal(str(1/3))
  Out [17]: Decimal('0.3333333333333333')

and the sum-to-one check then fails because::

  In  [18]: gbt.Decimal(str(1/3)) + gbt.Decimal(str(1/3)) + gbt.Decimal(str(1/3))
  Out [18]: Decimal('0.9999999999999999')

Setting payoffs for players also follows the same rules.  Representing probabilities
and payoffs exactly is essential, because `pygambit` offers (in particular for two-player
games) the possibility of computation of equilibria exactly, because the Nash equilibria
of any two-player game with rational payoffs and chance probabilities can be expressed exactly
in terms of rational numbers.

It is therefore advisable always to specify the numerical data of games either in terms
of :py:class:`.Decimal` or :py:class:`.Rational` values, or their string equivalents.
It is safe to use `int` values, but `float` values should be used with some care to ensure
the values are recorded as intended.


Reading a game from a file
~~~~~~~~~~~~~~~~~~~~~~~~~~

Games stored in existing Gambit savefiles in either the .efg or .nfg
formats can be loaded using :meth:`.Game.read_game`::

  In [1]: g = gbt.Game.read_game("e02.nfg")

  In [2]: g
  Out[2]:
  NFG 1 R "Selten (IJGT, 75), Figure 2, normal form" { "Player 1" "Player 2" }

  { { "1" "2" "3" }
  { "1" "2" }
  }
  ""

  {
  { "" 1, 1 }
  { "" 0, 2 }
  { "" 0, 2 }
  { "" 1, 1 }
  { "" 0, 3 }
  { "" 2, 0 }
  }
  1 2 3 4 5 6


Mixed strategy and behavior profiles
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A :py:class:`~.MixedStrategyProfile` represents a probability
distribution over the pure strategies of each player.  This is constructed
using :py:meth:`.Game.mixed_strategy_profile`.  Mixed strategy
profiles are initialized to uniform randomization over all strategies
for all players.

Mixed strategy profiles can be indexed in three ways.

#. Specifying a strategy returns the probability of that strategy
   being played in the profile.
#. Specifying a player returns a list of probabilities, one for each
   strategy available to the player.
#. Profiles can be treated as a list indexed from 0 up to the number
   of total strategies in the game minus one.

This sample illustrates the three methods::

  In [1]: g = gbt.Game.read_game("e02.nfg")

  In [2]: p = g.mixed_strategy_profile()

  In [3]: p[g.players[0]]
  Out[3]: [0.3333333333333333, 0.3333333333333333, 0.3333333333333333]

  In [4]: p[g.players[1].strategies[0]]
  Out[4]: 0.5

The expected payoff to a player is obtained using
:py:meth:`.MixedStrategyProfile.payoff`::

  In [5]: p.payoff(g.players[0])
  Out[5]: 0.6666666666666666

The standalone expected payoff to playing a given strategy, assuming
all other players play according to the profile, is obtained using
:py:meth:`.MixedStrategyProfile.strategy_value`::

  In [6]: p.strategy_value(g.players[0].strategies[2])
  Out[6]: 1.0

A :py:class:`.MixedBehaviorProfile` object, which represents a probability
distribution over the actions at each information set, is constructed
using :py:meth:`.Game.mixed_behavior_profile`.  Behavior profiles are
initialized to uniform randomization over all actions at each
information set.

Mixed behavior profiles are indexed similarly to mixed strategy
profiles, except that indexing by a player returns a list of lists of
probabilities, containing one list for each information set controlled
by that player::

  In [1]: g = gbt.Game.read_game("e02.efg")

  In [2]: p = g.mixed_behavior_profile()

  In [3]: p[g.players[0]]
  Out[3]: [[0.5, 0.5], [0.5, 0.5]]

  In [4]: p[g.players[0].infosets[0]]
  Out[4]: [0.5, 0.5]

  In [5]: p[g.players[0].infosets[0].actions[0]]
  Out[5]: 0.5

For games with a tree representation, a
:py:class:`.MixedStrategyProfile` can be converted to its equivalent
:py:class:`.MixedBehaviorProfile` by calling
:py:meth:`.MixedStrategyProfile.as_behavior`. Equally, a
:py:class:`.MixedBehaviorProfile` can be converted to an equivalent
:py:class:`.MixedStrategyProfile` using :py:meth:`.MixedBehaviorProfile.as_strategy`.


Computing Nash equilibria
~~~~~~~~~~~~~~~~~~~~~~~~~

Interfaces to algorithms for computing Nash equilibria are collected
in the module :py:mod:`pygambit.nash`.


==========================================    ========================================
Method                                        Python function
==========================================    ========================================
:ref:`gambit-enumpure <gambit-enumpure>`      :py:func:`pygambit.nash.enumpure_solve`
:ref:`gambit-enummixed <gambit-enummixed>`    :py:func:`pygambit.nash.enummixed_solve`
:ref:`gambit-lp <gambit-lp>`                  :py:func:`pygambit.nash.lp_solve`
:ref:`gambit-lcp <gambit-lcp>`                :py:func:`pygambit.nash.lcp_solve`
:ref:`gambit-liap <gambit-liap>`              :py:func:`pygambit.nash.liap_solve`
:ref:`gambit-logit <gambit-logit>`            :py:func:`pygambit.nash.logit_solve`
:ref:`gambit-simpdiv <gambit-simpdiv>`        :py:func:`pygambit.nash.simpdiv_solve`
:ref:`gambit-ipa <gambit-ipa>`                :py:func:`pygambit.nash.ipa_solve`
:ref:`gambit-gnm <gambit-gnm>`                :py:func:`pygambit.nash.gnm_solve`
==========================================    ========================================


For example, taking the game :file:`e02.efg` as an example::

  In [1]: g = gbt.Game.read_game("e02.efg")

  In [2]: gbt.nash.lcp_solve(g)
  Out[2]: [[[[Rational(1, 1), Rational(0, 1)], [Rational(1, 2), Rational(1, 2)]], [[Rational(1, 2), Rational(1, 2)]]]]

  In [3]: gbt.nash.lcp_solve(g, rational=False)
  Out[3]: [[[[1.0, 0.0], [0.5, 0.5]], [[0.5, 0.5]]]]

  In [4]: gbt.nash.lcp_solve(g, use_strategic=True)
  Out[4]: [[[Rational(1, 1), Rational(0, 1), Rational(0, 1)], [Rational(1, 1), Rational(0, 1)]]]

  In [5]: gbt.nash.lcp_solve(g, use_strategic=True, rational=True)
  Out[5]: [[[1.0, 0.0, 0.0], [1.0, 0.0]]]


