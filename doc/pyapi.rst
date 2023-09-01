.. _python-api:

.. py:module:: pygambit-manual


Python interface to Gambit library
==================================

Gambit provides a Python interface for programmatic manipulation of
games.  This section documents this interface, which is under active
development.  Refer to the :ref:`instructions for building the Python
interface <build-python>` to compile and install the Python extension.

.. note::

   Prior to 16.0.1, the Python extension was named `gambit`.
   It is now known as `pygambit` to avoid a name clash with an
   unrelated (and abandoned) project on PyPI.
   

A tutorial introduction
-----------------------

Building an extensive game
~~~~~~~~~~~~~~~~~~~~~~~~~~

The function :py:func:`Game.new_tree` creates a new, trivial
extensive game, with no players, and only a root node::

  In [1]: import pygambit

  In [2]: g = pygambit.Game.new_tree()

  In [3]: len(g.players)
  Out[3]: 0

The game also has no title.  The :py:attr:`~Game.title` attribute provides
access to a game's title::

  In [4]: str(g)
  Out[4]: "<Game ''>"

  In [5]: g.title = "A simple poker example"

  In [6]: g.title
  Out[6]: 'A simple poker example'

  In [7]: str(g)
  Out[7]: "<Game 'A simple poker example'>"

The :py:attr:`~Game.players` attribute of a game is a collection of
the players.  As seen above, calling :py:meth:`len` on the set of
players gives the number of players in the game.  Adding a
:py:class:`Player` to the game is done with the :py:meth:`add` member
of :py:attr:`~Game.players`::

  In [8]: p = g.players.add("Alice")

  In [9]: p
  Out[9]: <Player [0] 'Alice' in game 'A simple poker example'>

Each :py:class:`Player` has a text string stored in the
:py:attr:`~Player.label` attribute, which is useful for human
identification of players::

  In [10]: p.label
  Out[10]: 'Alice'

:py:attr:`Game.players` can be accessed like a Python list::

  In [11]: len(g.players)
  Out[11]: 1

  In [12]: g.players[0]
  Out[12]: <Player [0] 'Alice' in game 'A simple poker example'>

  In [13]: g.players
  Out[13]: [<Player [0] 'Alice' in game 'A simple poker example'>]


Building a strategic game
~~~~~~~~~~~~~~~~~~~~~~~~~

Games in strategic form are created using :py:func:`Game.new_table`, which
takes a list of integers specifying the number of strategies for
each player::

  In [1]: g = pygambit.Game.new_table([2,2])

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

The :py:attr:`~Player.strategies` collection for a :py:class:`Player` lists all the
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

Alternatively, one can use :py:func:`Game.from_arrays` in conjunction
with numpy arrays to construct a game with desired payoff matrices
more directly, as in::

  In [20]: m = numpy.array([ [ 8, 2 ], [ 10, 5 ] ], dtype=pygambit.Rational)
 
  In [21]: g = pygambit.Game.from_arrays(m, numpy.transpose(m))


Reading a game from a file
~~~~~~~~~~~~~~~~~~~~~~~~~~

Games stored in existing Gambit savefiles in either the .efg or .nfg
formats can be loaded using :py:func:`Game.read_game`::

  In [1]: g = pygambit.Game.read_game("e02.nfg")

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
The property :py:attr:`Game.contingencies` is the collection of
all such combinations.  Iterating over the contingencies collection
visits each pure strategy profile possible in the game::

   In [1]: g = pygambit.Game.read_game("e02.nfg")

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

A :py:class:`MixedStrategyProfile` object, which represents a probability
distribution over the pure strategies of each player, is constructed
using :py:meth:`Game.mixed_strategy_profile`.  Mixed strategy
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

  In [1]: g = pygambit.Game.read_game("e02.nfg")

  In [2]: p = g.mixed_strategy_profile()

  In [3]: list(p)
  Out[3]: [0.33333333333333331, 0.33333333333333331, 0.33333333333333331, 0.5, 0.5]

  In [4]: p[g.players[0]]
  Out[4]: [0.33333333333333331, 0.33333333333333331, 0.33333333333333331]

  In [5]: p[g.players[1].strategies[0]]
  Out[5]: 0.5

The expected payoff to a player is obtained using
:py:meth:`MixedStrategyProfile.payoff`::

  In [6]: p.payoff(g.players[0])
  Out[6]: 0.66666666666666663

The standalone expected payoff to playing a given strategy, assuming
all other players play according to the profile, is obtained using
:py:meth:`MixedStrategyProfile.strategy_value`::

  In [7]: p.strategy_value(g.players[0].strategies[2])
  Out[7]: 1.0

A :py:class:`MixedBehaviorProfile` object, which represents a probability
distribution over the actions at each information set, is constructed
using :py:meth:`Game.mixed_behavior_profile`.  Behavior profiles are
initialized to uniform randomization over all actions at each
information set.

Mixed behavior profiles are indexed similarly to mixed strategy
profiles, except that indexing by a player returns a list of lists of
probabilities, containing one list for each information set controlled
by that player::

  In [1]: g = pygambit.Game.read_game("e02.efg")

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
:py:class:`MixedStrategyProfile` can be converted to its equivalent
:py:class:`MixedBehaviorProfile` by calling
:py:func:`MixedStrategyProfile.as_behavior`. Equally, a
:py:class:`MixedBehaviorProfile` can be converted to an equivalent
:py:class:`MixedStrategyProfile` using :py:func:`MixedBehaviorProfile.as_strategy`.


Computing Nash equilibria
~~~~~~~~~~~~~~~~~~~~~~~~~

Interfaces to algorithms for computing Nash equilibria are collected
in the module :py:mod:`pygambit.nash`.  There are two choices for
calling these algorithms: directly within Python, or via the
corresponding Gambit :ref:`command-line tool <command-line>`.

Calling an algorithm directly within Python has less overhead, which
makes this approach well-suited to the analysis of smaller games,
where the expected running time is small.  In addition, these
interfaces may offer more fine-grained control of the behavior
of some algorithms.  

Calling the Gambit command-line tool launches the algorithm as a
separate process.  This makes it easier to abort during the run of the
algorithm (preserving where possible the equilibria which have already
been found), and also makes the program more robust to any internal
errors which may arise in the calculation.

Calling command-line tools
^^^^^^^^^^^^^^^^^^^^^^^^^^

The interface to each command-line tool is encapsulated in a class
with the word "External" in the name.  These operate by
creating a subprocess, which calls the corresponding Gambit
:ref:`command-line tool <command-line>`.  Therefore, a working
Gambit installation needs to be in place, with the command-line tools
located in the executable search path.

======================    ========================
Method                    Python class
======================    ========================
gambit-enumpure           ExternalEnumPureSolver
gambit-enummixed          ExternalEnumMixedSolver
gambit-lp                 ExternalLPSolver
gambit-lcp                ExternalLCPSolver
gambit-simpdiv            ExternalSimpdivSolver
gambit-gnm                ExternalGlobalNewtonSolver
gambit-enumpoly           ExternalEnumPolySolver
gambit-liap               ExternalLyapunovSolver
gambit-ipa                ExternalIteratedPolymatrixSolver
gambit-logit              ExternalLogitSolver
======================    ========================

For example, consider the game :file:`e02.nfg` from the set of standard
Gambit examples.  This game has a continuum of equilibria, in which
the first player plays his first strategty with probability one,
and the second player plays a mixed strategy, placing at least
probability one-half on her first strategy::

  In [1]: g = pygambit.Game.read_game("e02.nfg")

  In [2]: solver = pygambit.nash.ExternalEnumPureSolver()

  In [3]: solver.solve(g)
  Out[3]: [[1.0, 0.0, 0.0, 1.0, 0.0]]

  In [4]: solver = pygambit.nash.ExternalEnumMixedSolver()

  In [5]: solver.solve(g)
  Out[5]: [[1.0, 0.0, 0.0, 1.0, 0.0], [1.0, 0.0, 0.0, 0.5, 0.5]]

  In [6]: solver = pygambit.nash.ExternalLogitSolver()

  In [7]: solver.solve(g)
  Out[7]: [[0.99999999997881173, 0.0, 2.1188267679986399e-11, 0.50001141005647654, 0.49998858994352352]]

In this example, the pure strategy solver returns the unique
equilibrium in pure strategies.  Solving using
:program:`gambit-enummixed` gives two equilibria, which are the
extreme points of the set of equilibria.  Solving by tracing the
quantal response equilibrium correspondence produces a close numerical
approximation to one equilibrium; in fact, the equilibrium which is
the limit of the principal branch is the one in which the second
player randomizes with equal probability on both strategies.

When a game's representation is in extensive form, these solvers
default to using the version of the algorithm which operates on the
extensive game, where available, and returns a list of
:py:class:`pygambit.MixedBehaviorProfile` objects.  This can be overridden when
calling :py:meth:`solve` via the ``use_strategic`` parameter::

  In [1]: g = pygambit.Game.read_game("e02.efg")

  In [2]: solver = pygambit.nash.ExternalLCPSolver()

  In [3]: solver.solve(g)
  Out[3]: [<NashProfile for 'Selten (IJGT, 75), Figure 2': [1.0, 0.0, 0.5, 0.5, 0.5, 0.5]>]

  In [4]: solver.solve(g, use_strategic=True)
  Out[4]: [<NashProfile for 'Selten (IJGT, 75), Figure 2': [1.0, 0.0, 0.0, 1.0, 0.0]>]

As this game is in extensive form, in the first call, the returned
profile is a :py:class:`MixedBehaviorProfile`, while in the second, it
is a :py:class:`MixedStrategyProfile`.  While the set of equilibria is
not affected by whether behavior or mixed strategies are used, the
equilibria returned by specific solution methods may differ, when
using a call which does not necessarily return all equilibria.

Calling internally-linked libraries
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Where available, versions of algorithms which have been linked
internally into the Python library are generally called via
convenience functions.  The following table lists the algorithms
available via this approach.

========================================  ========================
Method                                    Python function
========================================  ========================
:ref:`gambit-enumpure <gambit-enumpure>`  :py:func:`gambit.nash.enumpure_solve`
:ref:`gambit-lp <gambit-lp>`              :py:func:`gambit.nash.lp_solve`
:ref:`gambit-lcp <gambit-lcp>`            :py:func:`gambit.nash.lcp_solve`
========================================  ========================

Parameters are available to modify the operation of the algorithm.
The most common ones are ``use_strategic``, to indicate the use of a
strategic form version of an algorithm where both extensive and
strategic versions are available, and ``rational``, to indicate
computation using rational arithmetic, where this is an option to the
algorithm.

For example, taking again the game :file:`e02.efg` as an example::

  In [1]: g = pygambit.Game.read_game("e02.efg")

  In [2]: pygambit.nash.lcp_solve(g)
  Out[2]: [[1.0, 0.0, 0.5, 0.5, 0.5, 0.5]]

  In [3]: pygambit.nash.lcp_solve(g, rational=True)
  Out[3]: [[Fraction(1, 1), Fraction(0, 1), Fraction(1, 2), Fraction(1, 2), Fraction(1, 2), Fraction(1, 2)]]

  In [4]: pygambit.nash.lcp_solve(g, use_strategic=True)
  Out[4]: [[1.0, 0.0, 0.0, 1.0, 0.0]]

  In [5]: pygambit.nash.lcp_solve(g, use_strategic=True, rational=True)
  Out[5]: [[Fraction(1, 1), Fraction(0, 1), Fraction(0, 1), Fraction(1, 1), Fraction(0, 1)]]



API documentation
-----------------



Analysis of quantal response equilibria
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~---------

.. py:module:: pygambit.qre

.. py:function:: fit_fixedpoint(data)

   .. versionadded:: 16.1.0

   Use maximum likelihood estimation to find the point on the
   principal branch of strategic form game closest to `data`.
   The `data` are expressed as an instance of a mixed strategy
   profile on the game.  The `data` should typically be expressed
   as the counts of observations of each strategy.

   :param data: The observed data to use in fitting.
   :type data: :py:class:`MixedStrategyProfile`
   :rtype: :py:class:`LogitQREMixedStrategyFitResult`

.. py:function:: fit_empirical(data)

   .. versionadded:: 16.1.0

   Use maximum likelihood estimation to estimate a quantal
   response equilibrium using the empirical payoff method.
   The `data` are expressed as an instance of a mixed strategy
   profile on the game.  The `data` should typically be expressed
   as the counts of observations of each strategy.

   :param data: The observed data to use in fitting.
   :type data: :py:class:`MixedStrategyProfile`
   :rtype: :py:class:`LogitQREMixedStrategyFitResult`

.. py:class:: LogitQREMixedStrategyFitResult

   .. versionadded:: 16.1.0

   The result of estimating a quantal response equilibrium
   using given data on a game.

   .. py:attribute:: method

   A text string indicating the estimation method used.
   This can be "fixedpoint" or "empirical".

   .. py:attribute:: profile

   The estimated :py:class:`MixedStrategyProfile`.

   .. py:attribute:: lam

   The estimated value of the precision parameter lambda.

   .. py:attribute:: data

   The data used in the estimation, represented as a
   :py:class:`MixedStrategyProfile`.

   .. py:attribute:: log_like

   The log of the likelihood function at the estimated profile.
