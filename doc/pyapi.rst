Python interface to Gambit library
==================================

A tutorial introduction
-----------------------

Building an extensive game
~~~~~~~~~~~~~~~~~~~~~~~~~~

The function :func:`gambit.new_tree` creates a new, trivial extensive game,
with no players, and only a root node::

  $ gambit-shell

  In [1]: g = gambit.new_tree()

  In [2]: len(g.players)
  Out[2]: 0

The game also has no title.  The :py:attr:`title` attribute provides
access to a game's title::

  In [3]: str(g)
  Out[3]: "<Game ''>"

  In [4]: g.title = "A simple poker example"

  In [5]: g.title
  Out[5]: 'A simple poker example'

  In [6]: str(g)
  Out[6]: "<Game 'A simple poker example'>"

The :py:attr:`players` attribute of a game is a collection of the
players.  As seen above, calling :py:meth:`len` on the set of players
gives the number of players in the game.  Adding a player is done
with the :py:meth:`add` member of :py:attr:`players`::

  In [7]: p = g.players.add("Alice")

  In [8]: p
  Out[8]: <Player [0] 'Alice' in game 'A simple poker example'>

Each player has a text string stored in the :py:attr:`label` attribute,
which is useful for human identification of players::

  In [9]: p.label
  Out[9]: 'Alice'

The :py:attr:`players` can be accessed like a Python list::

  In [10]: len(g.players)
  Out[10]: 1

  In [11]: g.players[0]
  Out[11]: <Player [0] 'Alice' in game 'A simple poker example'>

  In [12]: g.players
  Out[12]: [<Player [0] 'Alice' in game 'A simple poker example'>]


Building a strategic game
~~~~~~~~~~~~~~~~~~~~~~~~~

Games in strategic form are created using :func:`gambit.new_table`, which
takes a list of integers specifying the number of strategies for
each player::

  In [1]: g = gambit.new_table([2,2])

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

The :attr:`strategies` collection for a player lists all the
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



Reading a game from a file
~~~~~~~~~~~~~~~~~~~~~~~~~~

Games stored in existing Gambit savefiles in either the .efg or .nfg
formats can be loaded using :func:`gambit.read_game`::

  In [1]: g = gambit.read_game("e02.nfg")

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
The property :attr:`gambit.Gambit.contingencies` is the collection of
all such combinations.  Iterating over the contingencies collection
visits each pure strategy profile possible in the game::

   In [1]: g = gambit.read_game("e02.nfg")

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








Mixed strategies
~~~~~~~~~~~~~~~~

A mixed strategy object, which represents a probability distribution
over the pure strategies of a player, can be obtained using the
:meth:`gambit.Game.mixed_profile` method on a :class:`gambit.Game`
object.  Mixed strategies are initialized to uniform randomization
over all strategies for all players.

Mixed strategies can be indexed in three ways. Specifying a strategy
returns the probability of that strategy being played in the profile.
Specifying a player returns a list of probabilities, one for each
strategy available to the player.  Finally, mixed strategies can be
treated as a list indexed from 0 up to the number of total strategies
in the game minus one.  This latter behavior allows :py:func:`list` to
work as expected on a mixed strategy object::

  In [1]: g = gambit.read_game("e02.nfg")

  In [2]: p = g.mixed_profile()

  In [3]: list(p)
  Out[3]: [0.33333333333333331, 0.33333333333333331, 0.33333333333333331, 0.5, 0.5]

  In [4]: p[g.players[0]]
  Out[4]: [0.33333333333333331, 0.33333333333333331, 0.33333333333333331]

  In [5]: p[g.players[1].strategies[0]]
  Out[5]: 0.5

The expected payoff to a player is obtained using
:meth:`gambit.MixedProfile.payoff`::

  In [6]: p.payoff(g.players[0])
  Out[6]: 0.66666666666666663

The standalone expected payoff to playing a given strategy, assuming
all other players play according to the profile, is obtained using
:meth:`gambit.MixedProfile.strategy_value`::

  In [7]: p.strategy_value(g.players[0].strategies[2])
  Out[7]: 1.0


Computing Nash equilibria
-------------------------

Interfaces to algorithms for computing Nash equilibria are collected
in the module :mod:`gambit.nash`.  Each algorithm is encapsulated in
its own class.

Algorithms with the word "External" in the class name operate by
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

For example, consider the game e02.nfg from the set of standard
Gambit examples.  This game has a continuum of equilibria, in which
the first player plays his first strategty with probability one,
and the second player plays a mixed strategy, placing at least
probability one-half on her first strategy::

  In [1]: g = gambit.read_game("e02.nfg")

  In [2]: solver = gambit.nash.ExternalEnumPureSolver()

  In [3]: solver.solve(g)
  Out[3]: [[1.0, 0.0, 0.0, 1.0, 0.0]]

  In [4]: solver = gambit.nash.ExternalEnumMixedSolver()

  In [5]: solver.solve(g)
  Out[5]: [[1.0, 0.0, 0.0, 1.0, 0.0], [1.0, 0.0, 0.0, 0.5, 0.5]]

  In [6]: solver = gambit.nash.ExternalLogitSolver()

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




API documentation
-----------------

.. py:module:: gambit

.. py:function:: new_tree()

   Creates a new :py:class:`gambit.Game`
   consisting of a trivial game tree, with one
   node, which is both root and terminal, and no players.

.. py:function:: new_table(dim)

   Creates a new :py:class:`gambit.Game` with a strategic
   representation.  The parameter `dim` is a list of the number of
   strategies for each player.

.. py:function:: read_game(fn)

   Creates a new :py:class:`gambit.Game` by reading in the
   contents of the file named `fn`.

.. py:class:: Game

   An object representing a game, in extensive or strategic form.

   .. py:attribute:: is_tree

      Returns ``True`` if the game has a tree representation.

   .. py:attribute:: title

      Accesses the text string of the game's title.

   .. py:attribute:: players
 
      Returns a :py:class:`gambit.Players` collection object
      representing the players defined in the game.

   .. py:attribute:: contingencies

      Returns a collection object representing the collection of all
      possible pure strategy profiles in the game.

   .. py:attribute:: root

      Returns the :py:class:`gambit.Node` representing the root
      node of the game.

   .. py:method:: __getitem__(profile)

      Returns the :py:class:`gambit.Outcome` associated with a
      profile of pure strategies.  :literal:`profile` is a list
      of integers specifying the strategy number each player plays
      in the profile.

   .. py:method:: mixed_profile()

      Returns a mixed strategy profile :py:class:`gambit.MixedProfile`
      over the game, initialized to
      uniform randomization for each player over his strategies.

.. py:class:: Players
   
   A collection object representing the players in a game.

   .. py:method:: len()

      Returns the number of players in the game.

   .. py:method:: __getitem__(i)

      Returns player number ``i`` in the game.  Players are numbered
      starting with ``0``.

   .. py:attribute:: chance

      Returns the player representing all chance moves in the game.

   .. py:method:: add([label=""])

      Add a :py:class:`gambit.Player` to the game.  If label
      is specified, sets the text label for the player. In the case
      of extensive games this will create a new player with no 
      moves. In the case of strategic form games it creates a player
      with one strategy. If the provided player label is shared by
	  another player a warning will be returned.

.. py:class:: Player

   Represents a player in a :py:class:`gambit.Game`.

   .. py:attribute:: label

      A text label useful for identification of the player.

.. py:class:: Node

   Represents a node in a :py:class:`gambit.Game`.

   .. py:attribute:: label

      A text label useful for identification of the node.


.. py:class:: Strategies
   
   A collection object representing the strategies available to a
   player in a game.

   .. py:method:: len()

      Returns the number of strategies for the player.

   .. py:method:: __getitem__(i)

      Returns strategy number ``i``.  Strategies are numbered
      starting with ``0``.

.. py:class:: Strategy

   Represents a strategy available to a :py:class:`gambit.Player`.

   .. py:attribute:: label

      A text label useful for identification of the strategy.

.. py:class:: Node

   Represents a node in a :py:class:`gambit.Game`.

   .. py:attribute:: label

      A text label useful for identification of the node.

.. py:class:: Outcome

   Represents an outcome in a :py:class:`gambit.Game`.

   .. py:attribute:: label

      A text label useful for identification of the outcome.

   .. py:method:: __getitem__(pl)

      Returns the payoff to the ``pl`` th player at the outcome.

   .. py:method:: __setitem__(pl, payoff)

      Sets the payoff to the ``pl`` th player at the outcome to the
      specified ``payoff``.  Payoffs may be specified as integers
      or instances of ``decimal.Decimal`` or ``fractions.Fraction``.
      

.. py:class:: MixedProfile

   Represents a mixed strategy profile over a :py:class:`gambit.Game`.

   .. py:method:: __getitem__(index)

      Returns a slice of the profile based on the parameter
      ``index``.  If ``index`` is a :py:class:`gambit.Strategy`,
      returns the probability with which that strategy is played in
      the profile.  If ``index`` is a :py:class:`gambit.Player`,
      returns a list of probabilities, one for each strategy belonging
      to that player.  If ``index`` is an integer, returns the
      ``index`` th entry in the profile, treating the profile as a
      flat list of probabilities.

   .. py:method:: __setitem__(strategy, prob)

      Sets the probability ``strategy`` is played in the profile to ``prob``. 
         
   .. py:method:: payoff(player)

      Returns the expected payoff to ``player`` if all players play
      according to the profile.

   .. py:method:: strategy_value(strategy)

      Returns the expected payoff to choosing ``strategy`` if all
      other players play according to the profile.

   .. py:method:: liap_value()

      Returns the Lyapunov value (see [McK91]_) of the strategy profile.  The
      Lyapunov value is a non-negative number which is zero exactly at
      Nash equilibria.
