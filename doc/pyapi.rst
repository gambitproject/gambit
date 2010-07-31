Python interface to Gambit library
==================================

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

  In [7]: p = g.players.add()

  In [8]: p
  Out[8]: <Player [0] '' in game 'A simple poker example'>

Players can be given meaningful text labels using the :py:attr:`label` attribute::

  In [9]: p.label = "Alice"

  In [10]: p
  Out[10]: <Player [0] 'Alice' in game 'A simple poker example'>




