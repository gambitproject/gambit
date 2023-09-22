.. _python-api:

.. py:module:: pygambit-manual


``pygambit`` Python package
===========================

Gambit provides a Python package, ``pygambit``, which provides access to
Gambit's features.  ``pygambit`` is available on PyPI
(https://pypi.org/project/pygambit/), and can be installed via ``pip``.

User guide
----------

Building an extensive game
~~~~~~~~~~~~~~~~~~~~~~~~~~

The function :py:meth:`.Game.new_tree` creates a new, trivial
extensive game, with no players, and only a root node::

  In [1]: import pygambit as gbt

  In [2]: g = gbt.Game.new_tree()

  In [3]: len(g.players)
  Out[3]: 0

The :py:attr:`~.Game.title` attribute on a :py:class:`.Game` provides
access to a game's title::

  In [4]: g.title
  Out[4]: 'Untitled extensive game'

  In [5]: g.title = "A simple poker example"

  In [6]: g.title
  Out[6]: 'A simple poker example'


The :py:attr:`~.Game.players` attribute of a game is a collection of
the players.  Calling :py:meth:`~.Players.__len__` on the set of
players gives the number of players in the game.  Adding a
:py:class:`~.Player` to the game is done with the
:py:meth:`~.GamePlayers.add` member
of :py:attr:`~.Game.players`::

  In [8]: p = g.players.add("Alice")

  In [9]: p
  Out[9]: <Player [0] 'Alice' in game 'A simple poker example'>

Each :py:class:`~pygambit.gambit.Player` has a text string stored in the
:py:attr:`~pygambit.gambit.Player.label` attribute, which is useful for human
identification of players::

  In [10]: p.label
  Out[10]: 'Alice'

:py:attr:`~pygambit.gambit.Game.players` can be accessed like a Python list::

  In [11]: len(g.players)
  Out[11]: 1

  In [12]: g.players[0]
  Out[12]: <Player [0] 'Alice' in game 'A simple poker example'>

  In [13]: g.players
  Out[13]: [<Player [0] 'Alice' in game 'A simple poker example'>]


Building a strategic game
~~~~~~~~~~~~~~~~~~~~~~~~~

Games in strategic form are created using :py:meth:`.Game.new_table`, which
takes a list of integers specifying the number of strategies for
each player::

  In [1]: g = gbt.Game.new_table([2,2])

  In [2]: g.title = "A prisoner's dilemma game"

  In [3]: g.players[0].label = "Alphonse"

  In [4]: g.players[1].label = "Gaston"

  In [5]: g
  Out[5]: 
  NFG 1 R "A prisoner's dilemma game" { "Alphonse" "Gaston" }

  { { "1" "2" }
  { "1" "2" }
  }
  ""

  {
  }
  0 0 0 0 

The :py:attr:`.Player.strategies` collection for a
:py:class:`.Player` lists all the
strategies available for that player::

  In [6]: g.players[0].strategies
  Out[6]: [<Strategy [0] '1' for player 'Alphonse' in game 'A
  prisoner's dilemma game'>, 
           <Strategy [1] '2' for player 'Alphonse' in game 'A prisoner's dilemma game'>]

  In [7]: len(g.players[0].strategies)
  Out[7]: 2

  In [8]: g.players[0].strategies[0].label = "Cooperate"

  In [9]: g.players[0].strategies[1].label = "Defect"

  In [10]: g.players[0].strategies
  Out[10]: [<Strategy [0] 'Cooperate' for player 'Alphonse' in game 'A
  prisoner's dilemma game'>,
            <Strategy [1] 'Defect' for player 'Alphonse' in game 'A prisoner's dilemma game'>]

The outcome associated with a particular combination of strategies is
accessed by treating the game like an array. For a game :literal:`g`,
:literal:`g[i,j]` is the outcome where the first player plays his
:literal:`i` th strategy, and the second player plays his
:literal:`j` th strategy.  Payoffs associated with an outcome are set
or obtained by indexing the outcome by the player number.  For a
prisoner's dilemma game where the cooperative payoff is 8, the
betrayal payoff is 10, the sucker payoff is 2, and the noncooperative
(equilibrium) payoff is 5::

  In [11]: g[0,0][0] = 8

  In [12]: g[0,0][1] = 8

  In [13]: g[0,1][0] = 2

  In [14]: g[0,1][1] = 10

  In [15]: g[1,0][0] = 10

  In [16]: g[1,1][1] = 2

  In [17]: g[1,0][1] = 2

  In [18]: g[1,1][0] = 5

  In [19]: g[1,1][1] = 5

Alternatively, one can use :py:meth:`.Game.from_arrays` in conjunction
with numpy arrays to construct a game with desired payoff matrices
more directly, as in::

  In [20]: m = numpy.array([[8, 2], [10, 5]])
 
  In [21]: g = gbt.Game.from_arrays(m, numpy.transpose(m))


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

However, rounding can cause difficulties when attempting to use `float`s to
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

Iterating the pure strategy profiles in a game
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Each entry in a strategic game corresponds to the outcome arising from
a particular combination fo pure strategies played by the players.
The property :py:attr:`.Game.contingencies` is the collection of
all such combinations.  Iterating over the contingencies collection
visits each pure strategy profile possible in the game::

   In [1]: g = gbt.Game.read_game("e02.nfg")

   In [2]: list(g.contingencies)
   Out[2]: [[0, 0], [0, 1], [1, 0], [1, 1], [2, 0], [2, 1]]

Each pure strategy profile can then be used to access individual
outcomes and payoffs in the game::

   In [3]: for profile in g.contingencies:
      ...:     print profile, g[profile][0], g[profile][1]
      ...:     
   [0, 0] 1 1
   [0, 1] 1 1
   [1, 0] 0 2
   [1, 1] 0 3
   [2, 0] 0 2
   [2, 1] 2 0


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

  In [3]: list(p)
  Out[3]: [0.33333333333333331, 0.33333333333333331, 0.33333333333333331, 0.5, 0.5]

  In [4]: p[g.players[0]]
  Out[4]: [0.33333333333333331, 0.33333333333333331, 0.33333333333333331]

  In [5]: p[g.players[1].strategies[0]]
  Out[5]: 0.5

The expected payoff to a player is obtained using
:py:meth:`.MixedStrategyProfile.payoff`::

  In [6]: p.payoff(g.players[0])
  Out[6]: 0.66666666666666663

The standalone expected payoff to playing a given strategy, assuming
all other players play according to the profile, is obtained using
:py:meth:`.MixedStrategyProfile.strategy_value`::

  In [7]: p.strategy_value(g.players[0].strategies[2])
  Out[7]: 1.0

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

  In [3]: list(p)
  Out[3]: [0.5, 0.5, 0.5, 0.5, 0.5, 0.5]

  In [5]: p[g.players[0]]
  Out[5]: [[0.5, 0.5], [0.5, 0.5]]

  In [6]: p[g.players[0].infosets[0]]
  Out[6]: [0.5, 0.5]

  In [7]: p[g.players[0].infosets[0].actions[0]]
  Out[7]: 0.5

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




API documentation
-----------------

.. currentmodule:: pygambit

Representation of games
~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: pygambit.gambit
   :members:
   :undoc-members:

Computation on supports
~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: pygambit.supports
   :members:


Computation of Nash equilibria
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: pygambit.nash
   :members:
   :exclude-members: ExternalEnumMixedSolver, ExternalEnumPolySolver,
                     ExternalEnumPureSolver, ExternalGlobalNewtonSolver,
                     ExternalIteratedPolymatrixSolver, ExternalLCPSolver,
                     ExternalLPSolver, ExternalLogitSolver,
                     ExternalLyapunovSolver, ExternalSimpdivSolver,
                     ExternalSolver


Computation of quantal response equilibria
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: pygambit.qre
   :members:
   :exclude-members: ExternalStrategicQREPathTracer,
                     sym_compute_jac, sym_compute_lhs

