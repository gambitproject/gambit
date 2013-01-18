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

When a game's representation is in extensive form, these solvers
default to using the version of the algorithm which operates on the
extensive game, where available, and returns a list of
:py:class:`gambit.BehavProfile` objects.  This can be overridden when
calling :py:meth:`solve` via the ``use_strategic`` parameter::

  In [1]: g = gambit.read_game("e02.efg")

  In [2]: solver = gambit.nash.ExternalLCPSolver()

  In [3]: solver.solve(g)
  Out[3]: [<NashProfile for 'Selten (IJGT, 75), Figure 2': [1.0, 0.0, 0.5, 0.5, 0.5, 0.5]>]

  In [4]: solver.solve(g, use_strategic=True)
  Out[4]: [<NashProfile for 'Selten (IJGT, 75), Figure 2': [1.0, 0.0, 0.0, 1.0, 0.0]>]

As this game is in extensive form, in the first call, the returned
profile is a :py:class:`gambit.BehavProfile`, while in the second, it
is a :py:class:`gambit.MixedProfile`.  While the set of equilibria is
not affected by whether behavior or mixed strategies are used, the
equilibria returned by specific solution methods may differ, when
using a call which does not necessarily return all equilibria.

It is also possible to convert between mixed and behavior strategic
profiles using :meth:`gambit.MixedProfile.as_behav` and 
:meth:`gambit.BehavProfile.as_mixed`.


Hashing and game objects
------------------------

Games, and objects representing elements within games, have a hash
method defined and are therefore hashable, usable as keys in
dictionaries and in Python sets.  The hash value is generated based on
the memory address at which the underlying C++ object is stored.  This
meets the requirements of a Python hash value, as distinct objects
will generate distinct hash values.  However, the hash value generated
will vary in different runs of a program.  As such, operations which
depend on the sequence of the has value may generate different output
in different runs of the program, most notably popping from a set, or
iterating over the keys in a dictionary where the keys are game objects.



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

   .. py:attribute:: comment

      Accesses the text string of the game's comment.

   .. py:attribute:: actions

      Returns a :py:class:`gambit.GameActions` collection object
      representing the actions defined in the game.

      :raises: :py:class:`gambit.UndefinedOperationError` if the game does not have a tree representation.

   .. py:attribute:: infosets

      Returns a :py:class:`gambit.GameInfosets` collection object
      representing the information sets defined in the game.
      
      :raises: :py:class:`gambit.UndefinedOperationError` if the game does not have a tree representation.

   .. py:attribute:: players
 
      Returns a :py:class:`gambit.Players` collection object
      representing the players defined in the game.

   .. py:attribute:: strategies

      Returns a :py:class:`gambit.GameStrategies` collection object
      representing the strategies defined in the game.

   .. py:attribute:: contingencies

      Returns a collection object representing the collection of all
      possible pure strategy profiles in the game.

   .. py:attribute:: root

      Returns the :py:class:`gambit.Node` representing the root
      node of the game.

      :raises: :py:class:`gambit.UndefinedOperationError` if the game does not have a tree representation.

   .. py:attribute:: is_const_sum

      Returns ``True`` if the game is constant sum.

   .. py:attribute:: is_perfect_recall

      Returns ``True`` if the game is of perfect recall.

   .. py:attribute:: min_payoff

      Returns the smallest payoff in any outcome of the game.

   .. py:attribute:: max_payoff

      Returns the largest payoff in any outcome of the game.

   .. py:method:: __getitem__(profile)

      Returns the :py:class:`gambit.Outcome` associated with a
      profile of pure strategies.  :literal:`profile` is a list
      of integers specifying the strategy number each player plays
      in the profile.

   .. py:method:: mixed_profile(rational=False)

      Returns a mixed strategy profile :py:class:`gambit.MixedProfile`
      over the game, initialized to uniform randomization for each
      player over his strategies.  If the game has a tree
      representation, the mixed strategy profile is defined over the
      reduced strategic form representation.
      
      :param rational: If :literal:`True`, probabilities are represented using rational numbers; otherwise double-precision floating point numbers are used.  

   .. py:method:: behav_profile(rational=False)

      Returns a behavior strategy profile
      :py:class:`gambit.BehavProfile` over the game, initialized to
      uniform randomization for each player over his actions at each
      information set. 

      :param rational: If :literal:`True`, probabilities are represented using rational numbers; otherwise double-precision floating point numbers are used.  
      :raises: :py:class:`gambit.UndefinedOperationError` if the game does not have a tree representation.

.. py:class:: GameActions
   
   A collection object representing the actions of a game.

   .. py:method:: len()

      Returns the number of actions in the game.

   .. py:method:: __getitem__(i)

      Returns action number ``i`` in the game.  Actions are numbered
      starting with ``0``.

.. py:class:: GameInfosets
   
   A collection object representing the information sets of a game.

   .. py:method:: len()

      Returns the number of information sets in the game.

   .. py:method:: __getitem__(i)

      Returns information set number ``i`` in the game.  Information sets
      are numbered starting with ``0``.

.. py:class:: GameStrategies
   
   A collection object representing the strategies of a game.

   .. py:method:: len()

      Returns the number of strategies in the game.

   .. py:method:: __getitem__(i)

      Returns strategy ``i`` in the game.  Strategies are numbered
      starting with ``0``.

.. py:class:: Infoset

   An information set for an extensive form game.

   .. py:method:: precedes(node)

      Returns ``True`` or ``False`` depending on whether the specified node
      precedes the information set in the extensive game. 

   .. py:method:: reveal(player)

      Reveals the information set to a player.

   .. py:attribute:: actions

      Returns a :py:class:`gambit.Actions` collection object representing 
      the actions defined in this information set.

   .. py:attribute:: label

      A text label used to identify the information set.

   .. py:attribute:: is_chance

      Returns ``True`` or ``False`` depending on whether this information set is
      associated to the chance player.

   .. py:attribute:: members

      Returns the set of nodes associated with this information set.

   .. py:attribute:: player

      Returns the player object associated with this information set.

.. py:class:: Infosets
   
   A collection object representing the information sets available to a
   player in a game.

   .. py:method:: len()

      Returns the number of information sets for the player.

   .. py:method:: __getitem__(i)

      Returns information set number ``i``.  Information sets are numbered
      starting with ``0``.

.. py:class:: Action

   An action associated with an information set.

   .. py:method:: delete()

      Deletes this action from the game.

      :raises: :py:class:`gambit.UndefinedOperationError` when the action is the last one of its infoset.

   .. py:method:: precedes(node)

      Returns ``True`` or ``False`` depending on whether the specified node
      precedes the action in the extensive game. 

   .. py:attribute:: label

      A text label used to identify the action.

   .. py:attribute:: infoset

      Returns the information to which this action is associated.

   .. py:attribute:: prob

      A settable property that represents the probability associated 
      with the action. It can be a value stored as an int, 
      decimal.Decimal, or Fraction.fraction. 

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

   .. py:attribute:: game

      Returns the :py:class:`gambit.Game` in which the player is.

   .. py:attribute:: label

      A text label useful for identification of the player.

   .. py:attribute:: number

      Returns the number of the player in the :py:class:`gambit.Game`.
      Players are numbered starting with ``0``.

   .. py:attribute:: is_chance

      Returns ``True`` or ``False`` on whether the player represents the chance 
      moves or not.

   .. py:attribute:: infosets

      Returns a :py:class:`gambit.Infosets` collection object
      representing the information sets of the player.

   .. py:attribute:: strategies

      Returns a :py:class:`gambit.Strategies` collection object
      representing the strategies of the player.

   .. py:attribute:: min_payoff

      Returns the smallest payoff for the player in any outcome of the game.

   .. py:attribute:: max_payoff

      Returns the largest payoff for the player in any outcome of the game.

.. py:class:: Node

   Represents a node in a :py:class:`gambit.Game`.

   .. py:method:: is_successor_of(node)

      Returns ``True`` if the current node is a successor of the
      node provided in the argument list.

   .. py:method:: is_subgame_root(node)

      Returns ``True`` if the current node is a marked subgame root.

   .. py:attribute:: label

      A text label useful for identification of the node.

   .. py:attribute:: is_terminal

      Returns ``True`` if the node is a terminal node in the game tree.
      Returns ``False`` otherwise.

   .. py:attribute:: children

      Returns a collection of the current node's children.

   .. py:attribute:: game

      Returns the :py:class:`gambit.Game` associated with the 
      current node.

   .. py:attribute:: infoset

      Returns the :py:class:`gambit.Infoset` associated with the 
      current node.

   .. py:attribute:: player

      Returns the :py:class:`gambit.Player` associated with the 
      current node.

   .. py:attribute:: parent

      Returns the :py:class:`gambit.Node` that is the parent of 
      the current node.

   .. py:attribute:: prior_action

      Returns the action prior to the current node.

   .. py:attribute:: prior_sibling

      Returns the :py:class:`gambit.Node` that is prior to the 
      current node at the same level of the game tree.

   .. py:attribute:: next_sibling

      Returns the :py:class:`gambit.Node` that is prior to the 
      current node at the same level of the game tree.

   .. py:attribute:: outcome

      Returns the :py:class:`gambit.Outcome` that is associated 
      with the current node.

   .. py:method:: append_move(infoset[ , actions])

      Add a move to a terminal node, at the :py:class:`gambit.Infoset`
      ``infoset``.  Alternatively, a :py:class:`gambit.Player` can be
      passed as the information set, in which case the move is placed
      in a new information set for that player; in this instance, the
      number of ``actions`` at the new information set must be specified.

      :raises: :py:class:`gambit.UndefinedOperationError` when called on a non-terminal node.
      :raises: :py:class:`gambit.UndefinedOperationError` when called with a :py:class:`gambit.Player` object and no actions, or actions < 1.
      :raises: :py:class:`gambit.UndefinedOperationError` when called with a :py:class:`gambit.Infoset` object and with actions.
      :raises: :py:class:`gambit.MismatchError` when called with objects from different games.

   .. py:method:: insert_move(infoset[ , actions])

      Insert a move at a node, at the :py:class:`gambit.Infoset`
      ``infoset``.  Alternatively, a :py:class:`gambit.Player` can be
      passed as the information set, in which case the move is placed
      in a new information set for that player; in this instance, the
      number of ``actions`` at the new information set must be specified.
      The newly-inserted node takes the place of the node in the game
      tree, and the existing node becomes the first child of the new node.

      :raises: :py:class:`gambit.UndefinedOperationError` when called with a :py:class:`gambit.Player` object and no actions, or actions < 1.
      :raises: :py:class:`gambit.UndefinedOperationError` when called with a :py:class:`gambit.Infoset` object and with actions.
      :raises: :py:class:`gambit.MismatchError` when called with objects from different games.

   .. py:method:: leave_infoset()

      Removes this node from its information set. If this node is the last
      of its information set, this method does nothing.

   .. py:method:: delete_parent()

      Deletes the parent node and its subtrees other than the one 
      which contains this node and moves this node into its former 
      parent's place.

   .. py:method:: delete_tree()

      Deletes the whole subtree which has this node as a root, except 
      the actual node.

   .. py:method:: copy_tree(node)

      Copies the tree of this node to ``node``.

      :raises: :literal:`MismatchError` if both objects aren't in the same game.

   .. py:method:: move_tree(node)

      Move the tree of this node to ``node``.

      :raises: :literal:`MismatchError` if both objects aren't in the same game.

.. py:class:: Actions
   
   A collection object representing the actions available at an
   information set in a game.

   .. py:method:: len()

      Returns the number of actions for the player.

   .. py:method:: __getitem__(i)

      Returns action number ``i``.  Actions are numbered
      starting with ``0``.

   .. py:method:: add([action=None])

      Add a :py:class:`gambit.Action` to the list of actions of an 
      information set.

.. py:class:: Strategies
   
   A collection object representing the strategies available to a
   player in a game.

   .. py:method:: len()

      Returns the number of strategies for the player.

   .. py:method:: __getitem__(i)

      Returns strategy number ``i``.  Strategies are numbered
      starting with ``0``.

   .. py:method:: add([label=""])

      Add a :py:class:`gambit.Strategy` to the player's list of strategies.
      This method is only applicable to games in a strategic form. When
      this method is applied to a player in an extensive form it will raise
      a type error.

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

   .. py:method:: delete()

      Deletes the outcome from the game.

   .. py:attribute:: label

      A text label useful for identification of the outcome.

   .. py:method:: __getitem__(pl)

      Returns the payoff to the ``pl`` th player at the outcome.

   .. py:method:: __setitem__(pl, payoff)

      Sets the payoff to the ``pl`` th player at the outcome to the
      specified ``payoff``.  Payoffs may be specified as integers
      or instances of ``decimal.Decimal`` or ``fractions.Fraction``.

.. py:class:: Outcomes
   
   A collection object representing the outcomes of a game.

   .. py:method:: len()

      Returns the number of outcomes in the game.

   .. py:method:: __getitem__(i)

      Returns outcome ``i`` in the game.  Outcomes are numbered
      starting with ``0``.

   .. py:method:: add([label=""])

      Add a :py:class:`gambit.Outcome` to the game.  If label
      is specified, sets the text label for the outcome. If the 
      provided outcome label is shared by another outcome a warning 
      will be returned.
      

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

   .. py:method:: as_behav()

      Returns a behavior strategy profile :py:class:`BehavProfile` associated
      to the profile.

      :raises: :py:class:`gambit.UndefinedOperationError` if the game does not have a tree representation.
         
   .. py:method:: copy()

      Creates a copy of the mixed strategy profile.

   .. py:method:: payoff(player)

      Returns the expected payoff to ``player`` if all players play
      according to the profile.

   .. py:method:: strategy_value(strategy)

      Returns the expected payoff to choosing ``strategy`` if all
      other players play according to the profile.

   .. py:method:: strategy_values(player)

      Returns the expected payoffs for a player's set of strategies 
      to choosing ``strategy`` if all other players play according to 
      the profile.

   .. py:method:: liap_value()

      Returns the Lyapunov value (see [McK91]_) of the strategy profile.  The
      Lyapunov value is a non-negative number which is zero exactly at
      Nash equilibria.

.. py:class:: BehavProfile

   Represents a behavior strategy profile over a :py:class:`gambit.Game`.

   .. py:method:: __getitem__(index)

      Returns a slice of the profile based on the parameter
      ``index``.  If ``index`` is a :py:class:`gambit.Action`,
      returns the probability with which that action is played in
      the profile. 
      If ``index`` is an :py:class:`gambit.Infoset`,
      returns a list of probabilities, one for each action belonging
      to that information set.  If ``index`` is an integer, returns the
      ``index`` th entry in the profile, treating the profile as a
      flat list of probabilities.

   .. py:method:: __setitem__(action, prob)

      Sets the probability ``action`` is played in the profile to ``prob``. 

   .. py:method:: as_mixed()

      Returns a behavior strategy profile as a :py:class:`BehavProfile` 
      object associated to the profile.
         
   .. py:method:: copy()

      Creates a copy of the behavior strategy profile.

   .. py:method:: payoff(player)

      Returns the expected payoff to ``player`` if all players play
      according to the profile.

   .. py:method:: action_value(action)

      Returns the expected payoff to choosing ``action``, conditional
      on having reached the information set, if all
      other players play according to the profile.

   .. py:method:: infoset_prob(infoset)

      Returns the probability with which information set ``infoset``
      is reached, if all players play according to the profile.

   .. py:method:: infoset_value(infoset)

      Returns the expected payoff to the player who has the move at
      ``infoset``, conditional on the information set being reached,
      if all players play according to the profile.

   .. py:method:: liap_value()

      Returns the Lyapunov value (see [McK91]_) of the strategy profile.  The
      Lyapunov value is a non-negative number which is zero exactly at
      Nash equilibria.

.. py:class:: MismatchError

   An ``Exception`` which is raised on an operation between objects from 
   different games.
   Subclasses from ``ValueError``.

.. py:class:: UndefinedOperationError

   An ``Exception`` which is raised when an undefined operation is 
   attempted.
   Subclasses from ``ValueError``.
