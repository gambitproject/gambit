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

  In [1]: g = pygambit.Game.read_game("e02.efg")

  In [2]: pygambit.nash.lcp_solve(g)
  Out[2]: [[1.0, 0.0, 0.5, 0.5, 0.5, 0.5]]

  In [3]: pygambit.nash.lcp_solve(g, rational=True)
  Out[3]: [[Fraction(1, 1), Fraction(0, 1), Fraction(1, 2), Fraction(1, 2), Fraction(1, 2), Fraction(1, 2)]]

  In [4]: pygambit.nash.lcp_solve(g, use_strategic=True)
  Out[4]: [[1.0, 0.0, 0.0, 1.0, 0.0]]

  In [5]: pygambit.nash.lcp_solve(g, use_strategic=True, rational=True)
  Out[5]: [[Fraction(1, 1), Fraction(0, 1), Fraction(0, 1), Fraction(1, 1), Fraction(0, 1)]]



