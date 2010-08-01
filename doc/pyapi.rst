Python interface to Gambit library
==================================

A tutorial introduction
-----------------------

Building an extensive game
~~~~~~~~~~~~~~~~~~~~~~~~~~

The function :func:`new_tree` creates a new, trivial extensive game,
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

Games in strategic form are created using :func:`new_table`, which
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




API documentation
-----------------

.. py:module:: gambit

.. py:function:: new_tree()

   Creates a new :py:class:`gambit.Game`
   consisting of a trivial game tree, with one
   node, which is both root and terminal, and no players.

.. py:function:: new_table(dim)

   Creates a new :py:class:`gambit.Game` with a strategic
   representation.  The parameter dim is a list of the number of
   strategies for each player.


.. py:class:: Game

   An object representing a game, in extensive or strategic form.

   .. py:attribute:: is_tree

      Returns ``True`` if the game has a tree representation.

   .. py:attribute:: title

      Accesses the text string of the game's title.

   .. py:attribute:: players
 
      Returns a :py:class:`gambit.Players` collection object
      representing the players defined in the game.

   .. py:attribute:: root

      Returns the :py:class:`gambit.Node` representing the root
      node of the game.

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
      is specified, sets the text label for the player.

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


      
