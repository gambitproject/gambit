.. _python-api:

.. py:module:: pygambit


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

Game representations
~~~~~~~~~~~~~~~~~~~~

.. py:class:: Game

   An object representing a game, in extensive or strategic form.

   .. py:classmethod:: new_tree(title=None)

      Creates a new :py:class:`Game`
      consisting of a trivial game tree, with one
      node, which is both root and terminal, and no players.

      :param title: The title of the game.  If no title is specified,
		    "Untitled extensive game" is used.
      :type title: str

      .. versionadded:: 16.1.0
	 Added the *title* parameter

   .. py:classmethod:: new_table(dim, title=None)
 
      Creates a new :py:class:`Game` with a strategic
      representation. 

      :param dim: A list specifying the number of strategies for each player.
      :param title: The title of the game.  If no title is specified,
		    "Untitled strategic game" is used.
      :type title: str

      .. versionadded:: 16.1.0
	 Added the *title* parameter

   .. py:classmethod:: from_arrays(*arrays, title=None)

      Creates a new :py:class:`Game` with a strategic representation.
      Each entry in arrays is a numpy array giving the payoff matrix for the
      corresponding player.  The arrays must all have the same shape,
      and have the same number of dimensions as the total number of players.
      
      :param title: The title of the game.  If no title is specified,
		    "Untitled strategic game" is used.
      :type title: str

      .. versionadded:: 16.1.0
	 Added the *title* parameter

   .. py:classmethod:: read_game(fn)

      Constructs a game from its serialized representation in a file.
      See :ref:`file-formats` for details on recognized formats.

      :param file fn: The path to the file to open
      :raises IOError: if the file cannot be opened, or does not contain
	   	       a valid game representation

   .. py:classmethod:: parse_game(s)

      Constructs a game from its seralized representation in a string.	
      See :ref:`file-formats` for details on recognized formats.

      :param str s: The string containing the serialized representation
      :raises IOError: if the string does not contain a valid game
		       representation

   .. py:attribute:: is_tree

      Returns ``True`` if the game has a tree representation.

   .. py:attribute:: title

      Accesses the text string of the game's title.

   .. py:attribute:: comment

      Accesses the text string of the game's comment.

   .. py:attribute:: actions

      Returns a list-like object representing the actions defined in the game.

      :raises pygambit.UndefinedOperationError: if the game does not have a tree representation.

   .. py:attribute:: infosets

      Returns a list-like object representing the information sets defined in the game.
      
      :raises pygambit.UndefinedOperationError: if the game does not have a tree representation.

   .. py:attribute:: players
 
      Returns a :py:class:`Players` collection object
      representing the players defined in the game.

   .. py:attribute:: strategies

      Returns a list-like object representing the strategies defined in the game.

   .. py:attribute:: contingencies

      Returns a collection object representing the collection of all
      possible pure strategy profiles in the game.

   .. py:attribute:: root

      Returns the :py:class:`Node` representing the root
      node of the game.

      :raises: :py:class:`UndefinedOperationError` if the game does not have a tree representation.

   .. py:attribute:: is_const_sum

      Returns ``True`` if the game is constant sum.

   .. py:attribute:: is_perfect_recall

      Returns ``True`` if the game is of perfect recall.

   .. py:attribute:: min_payoff

      Returns the smallest payoff in any outcome of the game.

   .. py:attribute:: max_payoff

      Returns the largest payoff in any outcome of the game.

   .. py:method:: __getitem__(profile)

      Returns the :py:class:`Outcome` associated with a
      profile of pure strategies. 

      :param profile: A list of integers specifying the strategy
                      number each player plays in the profile.

   .. py:method:: mixed_strategy_profile(rational=False)

      Returns a mixed strategy profile :py:class:`MixedStrategyProfile`
      over the game, initialized to uniform randomization for each
      player over his strategies.  If the game has a tree
      representation, the mixed strategy profile is defined over the
      reduced strategic form representation.
      
      :param rational: If :literal:`True`, probabilities are
                       represented using rational numbers; otherwise
                       double-precision floating point numbers are
                       used.

   .. py:method:: mixed_behavior_profile(rational=False)

      Returns a behavior strategy profile
      :py:class:`MixedBehaviorProfile` over the game, initialized to
      uniform randomization for each player over his actions at each
      information set. 

      :param rational: If :literal:`True`, probabilities are
                       represented using rational numbers; otherwise
                       double-precision floating point numbers are
                       used.
      :raises UndefinedOperationError: if the game
				       does not have a tree representation.

   .. py:method:: write(format='native')

      Returns a serialization of the game.  Several output formats are
      supported, depending on the representation of the game.

      * `efg`: A representation of the game in
        :ref:`the .efg extensive game file format <file-formats-efg>`.
        Not available for games in strategic representation.
      * `nfg`: A representation of the game in
        :ref:`the .nfg strategic game file format <file-formats-nfg>`.
        For an extensive game, this uses the reduced strategic form
        representation.
      * `gte`: The XML representation used by the Game Theory Explorer
        tool.   Only available for extensive games.
      * `native`: The format most appropriate to the
        underlying representation of the game, i.e., `efg` or `nfg`.

      This method also supports exporting to other output formats
      (which cannot be used directly to re-load the game later, but
      are suitable for human consumption, inclusion in papers, and so
      on):

      * `html`: A rendering of the strategic form of the game as a
	collection of HTML tables.  The first player is the row
	chooser; the second player the column chooser.  For games with
	more than two players, a collection of tables is generated,
	one for each possible strategy combination of players 3 and higher.
      * `sgame`: A rendering of the strategic form of the game in
	LaTeX, suitable for use with `Martin Osborne's sgame style
	<https://www.economics.utoronto.ca/osborne/latex/>`_.
	The first player is the row
	chooser; the second player the column chooser.  For games with
	more than two players, a collection of tables is generated,
	one for each possible strategy combination of players 3 and higher.
	
	
.. py:class:: StrategicRestriction

   A read-only view on a :py:class:`Game`, defined by a subset
   of the strategies on the original game.

   In addition to the members described here, a StrategicRestriction
   implements the interface of a :py:class:`Game`, although
   operations which change the content of the game will raise an
   exception.

   .. py:method:: unrestrict()

      Returns the :py:class:`Game` object on which the
      restriction was based.


Representations of play of games
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The main responsibility of these classes is to capture information
about a plan of play of a game, by one or more players.

.. py:class:: StrategySupportProfile

   A set-like object representing a subset of the strategies in a
   game.  It incorporates the restriction that each player must have
   at least one strategy.

   .. py:attribute:: game

      Returns the :py:class:`Game` on which the support
      profile is defined.

   .. py:method:: issubset(other)

      Returns :literal:`True` if this profile is a subset of
      `other`.

      :param StrategySupportProfile other: another support profile

   .. py:method:: issuperset(other)

      Returns :literal:`True` if this profile is a superset of
      `other`.

      :param StrategySupportProfile other: another support profile

   .. py:method:: restrict()

      Creates a :py:class:`StrategicRestriction` object,
      which defines a restriction of the game in which only the
      strategies in this profile are present.

   .. py:method:: remove(strategy)

      Modifies the support profile by removing the specified strategy.

      :param Strategy strategy: the strategy to remove
      :raises UndefinedOperationError: if attempting to remove the
				       last strategy for a player

   .. py:method:: difference(other)

      Returns a new support profile containing all the strategies
      which are present in this profile, but not in `other`.

      :param StrategySupportProfile other: another support profile

   .. py:method:: intersection(other)

      Returns a new support profile containing all the strategies
      present in both this profile and in `other`.

      :param StrategySupportProfile other: another support profile

   .. py:method:: union(other)
   
      Returns a new support profile containing all the strategies
      present in this profile, in `other`, or in both.

      :param StrategySupportProfile other: another support profile

.. py:class:: MixedStrategyProfile

   Represents a mixed strategy profile over a :py:class:`Game`.

   .. py:method:: __getitem__(index)

      Returns a slice of the profile based on the parameter
      ``index``.  

      * If ``index`` is a :py:class:`Strategy`, returns the
        probability with which that strategy is played in the profile.
      * If ``index`` is a :py:class:`Player`, returns a list of
        probabilities, one for each strategy belonging to that player.
      * If ``index`` is an integer, returns the ``index`` th entry in
        the profile, treating the profile as a flat list of probabilities.

   .. py:method:: __setitem__(strategy, prob)

      Sets the probability ``strategy`` is played in the profile to ``prob``. 

   .. py:method:: as_behavior()

      Returns a behavior strategy profile :py:class:`BehavProfile` associated
      to the profile.

      :raises pygambit.UndefinedOperationError: if the game does not
                                              have a tree representation.
         
   .. py:method:: copy()

      Creates a copy of the mixed strategy profile.

   .. py:method:: payoff(player)

      Returns the expected payoff to a player if all players play
      according to the profile.

   .. py:method:: strategy_value(strategy)

      Returns the expected payoff to choosing the strategy, if all
      other players play according to the profile.

   .. py:method:: strategy_values(player)

      Returns the expected payoffs for a player's set of strategies 
      if all other players play according to the profile.

   .. py:method:: liap_value()

      Returns the Lyapunov value (see [McK91]_) of the strategy profile.  The
      Lyapunov value is a non-negative number which is zero exactly at
      Nash equilibria.

   .. py:method:: normalize()

      Each player's component of the profile is not enforced to sum to
      one, so that, for example, counts rather than probabilities can
      be expressed.  Calling this returns a profile in which the
      probability distribution over each player's strategies
      sums to one.

      .. versionchanged:: 16.1.0

	 Returns the normalized profile as a copy and leaves the
	 original changed.  Previously the original profile
	 was normalized in place.
      
   .. py:method:: randomize(denom)

      Randomizes the probabilities in the profile.  These are
      generated as uniform distributions over each mixed strategy.  If
      ``denom`` is specified, all probabilities are divisible by
      ``denom``, that is, the distribution is uniform over a discrete
      grid of mixed strategies.  ``denom`` is required for profiles
      in which the probabilities are rational numbers.

      :raises TypeError: if ``denom`` is not specified for a profile
			 with rational probabilities.      
     
.. py:class:: MixedBehaviorProfile

   Represents a behavior strategy profile over a :py:class:`Game`.

   .. py:method:: __getitem__(index)

      Returns a slice of the profile based on the parameter
      ``index``.  

      * If ``index`` is a :py:class:`Action`,
        returns the probability with which that action is played in
        the profile. 
      * If ``index`` is an :py:class:`Infoset`,
        returns a list of probabilities, one for each action belonging
        to that information set.  
      * If ``index`` is a :py:class:`Player`,
        returns a list of lists of probabilities, one list for each
        information set controlled by the player.
      * If ``index`` is an integer, returns the
        ``index`` th entry in the profile, treating the profile as a
        flat list of probabilities.

   .. py:method:: __setitem__(action, prob)

      Sets the probability ``action`` is played in the profile to ``prob``. 

   .. py:method:: as_strategy()

      Returns a :py:class:`MixedStrategyProfile` which is equivalent
      to the profile.

   .. py:method:: belief(node)

      Returns the probability ``node`` is reached, given its information 
      set was reached.

   .. py:method:: belief(infoset)

      Returns a list of belief probabilities of each node in ``infoset``.
         
   .. py:method:: copy()

      Creates a copy of the behavior strategy profile.

   .. py:method:: payoff(player)

      Returns the expected payoff to ``player`` if all players play
      according to the profile.

   .. py:method:: payoff(action)

      Returns the expected payoff to choosing ``action``, conditional
      on having reached the information set, if all
      other players play according to the profile.

   .. py:method:: payoff(infoset)

      Returns the expected payoff to the player who has the move at
      ``infoset``, conditional on the information set being reached,
      if all players play according to the profile.

   .. py:method:: regret(action)

      Returns the regret associated to ``action``.

   .. py:method:: realiz_prob(infoset)

      Returns the probability with which information set ``infoset``
      is reached, if all players play according to the profile.

   .. py:method:: liap_value()

      Returns the Lyapunov value (see [McK91]_) of the strategy profile.  The
      Lyapunov value is a non-negative number which is zero exactly at
      Nash equilibria.

   .. py:method:: normalize()

      Each information set's component of the profile is not enforced to sum to
      one, so that, for example, counts rather than probabilities can
      be expressed.  Calling this returns a profile in which the
      probability distribution over each information set's actions
      sums to one.

      .. versionchanged:: 16.1.0

	 Returns the normalized profile as a copy and leaves the
	 original changed.  Previously the original profile
	 was normalized in place.
      
   .. py:method:: randomize(denom)

      Randomizes the probabilities in the profile.  These are
      generated as uniform distributions over the actions at each
      information set.  If
      ``denom`` is specified, all probabilities are divisible by
      ``denom``, that is, the distribution is uniform over a discrete
      grid of mixed strategies.  ``denom`` is required for profiles
      in which the probabilities are rational numbers.

      :raises TypeError: if ``denom`` is not specified for a profile
			 with rational probabilities.      
     

      
Elements of games
~~~~~~~~~~~~~~~~~

These classes represent elements which exist inside of the definition
of game.

.. py:class:: Rational

   .. versionadded:: 15.0.0

   Represents a rational number in specifying numerical data for a
   game, or in a computed strategy profile.  This is implemented as a
   subclass of the Python standard library
   :py:class:`fractions.Fraction`, with additional instrumentation for
   rendering in IPython notebooks.


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

      Add a :py:class:`Player` to the game.  If label
      is specified, sets the text label for the player. In the case
      of extensive games this will create a new player with no 
      moves. In the case of strategic form games it creates a player
      with one strategy. If the provided player label is shared by
      another player a warning will be returned.

.. py:class:: Player

   Represents a player in a :py:class:`Game`.

   .. py:attribute:: game

      Returns the :py:class:`Game` in which the player is.

   .. py:attribute:: label

      A text label useful for identification of the player.

   .. py:attribute:: number

      Returns the number of the player in the :py:class:`Game`.
      Players are numbered starting with ``0``.

   .. py:attribute:: is_chance

      Returns ``True`` if the player object represents the chance player.

   .. py:attribute:: infosets

      Returns a list-like object representing the information sets of the player.

   .. py:attribute:: strategies

      Returns a :py:class:`pygambit.Strategies` collection object
      representing the strategies of the player.

   .. py:attribute:: min_payoff

      Returns the smallest payoff for the player in any outcome of the game.

   .. py:attribute:: max_payoff

      Returns the largest payoff for the player in any outcome of the game.


.. py:class:: Infoset

   An information set for an extensive form game.

   .. py:method:: precedes(node)

      Returns ``True`` or ``False`` depending on whether the specified node
      precedes the information set in the extensive game. 

   .. py:method:: reveal(player)

      Reveals the information set to a player.

   .. py:attribute:: actions

      Returns a :py:class:`pygambit.Actions` collection object representing 
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


.. py:class:: Actions
   
   A collection object representing the actions available at an
   information set in a game.

   .. py:method:: len()

      Returns the number of actions for the player.

   .. py:method:: __getitem__(i)

      Returns action number ``i``.  Actions are numbered
      starting with ``0``.

   .. py:method:: add([action=None])

      Add a :py:class:`Action` to the list of actions of an 
      information set.


.. py:class:: Action

   An action associated with an information set.

   .. py:method:: delete()

      Deletes this action from the game.

      :raises pygambit.UndefinedOperationError: when the action is the
                                              last one of its infoset.

   .. py:method:: precedes(node)

      Returns ``True`` if ``node`` precedes this action in the
      extensive game.

   .. py:attribute:: label

      A text label used to identify the action.

   .. py:attribute:: infoset

      Returns the information to which this action is associated.

   .. py:attribute:: prob

      A settable property that represents the probability associated 
      with the action. It can be a value stored as an int,
      :py:class:`pygambit.Rational`, or :py:class:`pygambit.Decimal`.


.. py:class:: Strategies
   
   A collection object representing the strategies available to a
   player in a game.

   .. py:method:: len()

      Returns the number of strategies for the player.

   .. py:method:: __getitem__(i)

      Returns strategy number ``i``.  Strategies are numbered
      starting with ``0``.

   .. py:method:: add([label=""])

      Add a :py:class:`Strategy` to the player's list of strategies.

      :raises TypeError: if called on a game which has an extensive representation.


.. py:class:: Strategy

   Represents a strategy available to a :py:class:`Player`.

   .. py:attribute:: label

      A text label useful for identification of the strategy.



.. py:class:: Node

   Represents a node in a :py:class:`Game`.

   .. py:method:: is_successor_of(node)

      Returns ``True`` if the node is a successor of ``node``.

   .. py:method:: is_subgame_root(node)

      Returns ``True`` if the current node is a root of a proper subgame.

   .. py:attribute:: label

      A text label useful for identification of the node.

   .. py:attribute:: is_terminal

      Returns ``True`` if the node is a terminal node in the game tree.

   .. py:attribute:: children

      Returns a collection of the node's children.

   .. py:attribute:: game

      Returns the :py:class:`Game` to which the node belongs.

   .. py:attribute:: infoset

      Returns the :py:class:`Infoset` associated with the node.

   .. py:attribute:: player

      Returns the :py:class:`Player` associated with the node.

   .. py:attribute:: parent

      Returns the :py:class:`Node` that is the parent of this node.

   .. py:attribute:: prior_action

      Returns the action immediately prior to the node.

   .. py:attribute:: prior_sibling

      Returns the :py:class:`Node` that is prior to the 
      node at the same level of the game tree.

   .. py:attribute:: next_sibling

      Returns the :py:class:`Node` that is the next node at the same
      level of the game tree.

   .. py:attribute:: outcome

      Returns the :py:class:`Outcome` that is associated 
      with the node.

   .. py:method:: append_move(infoset[ , actions])

      Add a move to a terminal node, at the :py:class:`pygambit.Infoset`
      ``infoset``.  Alternatively, a :py:class:`pygambit.Player` can be
      passed as the information set, in which case the move is placed
      in a new information set for that player; in this instance, the
      number of ``actions`` at the new information set must be specified.

      :raises pygambit.UndefinedOperationError: when called on a non-terminal node.
      :raises pygambit.UndefinedOperationError: when called with a :py:class:`Player` object and no actions, or actions < 1.
      :raises pygambit.UndefinedOperationError: when called with a :py:class:`Infoset` object and with actions.
      :raises pygambit.MismatchError: when called with objects from different games.

   .. py:method:: insert_move(infoset[ , actions])

      Insert a move at a node, at the :py:class:`Infoset`
      ``infoset``.  Alternatively, a :py:class:`Player` can be
      passed as the information set, in which case the move is placed
      in a new information set for that player; in this instance, the
      number of ``actions`` at the new information set must be specified.
      The newly-inserted node takes the place of the node in the game
      tree, and the existing node becomes the first child of the new node.

      :raises pygambit.UndefinedOperationError: when called with a :py:class:`Player` object and no actions, or actions < 1.
      :raises pygambit.UndefinedOperationError: when called with a :py:class:`Infoset` object and with actions.
      :raises pygambit.MismatchError: when called with objects from different games.

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

      Copies the subtree rooted at this node to ``node``.

      :raises pygambit.MismatchError: if both objects aren't in the same game.

   .. py:method:: move_tree(node)

      Move the subtree rooted at this node to ``node``.

      :raises pygambit.MismatchError: if both objects aren't in the same game.


.. py:class:: Outcomes
   
   A collection object representing the outcomes of a game.

   .. py:method:: len()

      Returns the number of outcomes in the game.

   .. py:method:: __getitem__(i)

      Returns outcome ``i`` in the game.  Outcomes are numbered
      starting with ``0``.

   .. py:method:: add([label=""])

      Add a :py:class:`Outcome` to the game.  If label
      is specified, sets the text label for the outcome. If the 
      provided outcome label is shared by another outcome a warning 
      will be returned.


.. py:class:: Outcome

   Represents an outcome in a :py:class:`Game`.

   .. py:method:: delete()

      Deletes the outcome from the game.

   .. py:attribute:: label

      A text label useful for identification of the outcome.

   .. py:method:: __getitem__(player)

      Returns the payoff to ``player`` at the outcome.  ``player``
      may be a :py:class:`Player`, a string, or an integer.
      If a string, returns the payoff to the player with that string
      as its label.  If an integer, returns the payoff to player
      number ``player``.

   .. py:method:: __setitem__(player, payoff)

      Sets the payoff to the ``pl`` th player at the outcome to the
      specified ``payoff``.  Payoffs may be specified as integers
      or instances of :py:class:`pygambit.Decimal` or :py:class:`pygambit.Rational`.
      Players may be specified as in :py:func:`__getitem__`.


Representation of errors and exceptions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. py:exception:: MismatchError

   A subclass of :py:exc:`ValueError` which is raised when
   attempting an operation among objects from different games.

.. py:exception:: UndefinedOperationError

   A subclass of :py:exc:`ValueError` which is raised when an
   operation which is not well-defined is attempted.


Computation of Nash equilibria
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. py:module:: pygambit.nash

.. py:function:: enumpure_solve(game, use_strategic=True, external=False)

   Compute :ref:`pure-strategy Nash equilibria <gambit-enumpure>` of a
   game.

   :param bool use_strategic: Use the strategic form.  If
			      :literal:`False`, computes agent-form
     		              pure-strategy equilibria, which treat
			      only unilateral deviations at an
			      individual information set
   :param bool external: Call the external command-line solver instead
			 of the internally-linked implementation


.. py:function:: enummixed_solve(game, rational=True, external=False, use_lrs=False)

   Compute all :ref:`mixed-strategy Nash equilibria
   <gambit-enummixed>` of a two-player strategic game.

   :param bool rational: Compute using rational precision (more
			 precise, often much slower)
   :param bool external: Call the external command-line solver instead
			 of the internally-linked implementation
   :param bool use_lrs: Use the lrslib-based implementation.  This is
		   experimental but preliminary results suggest it is
		   significantly faster.			
   :raises RuntimeError: if game has more than two players.
      
.. py:function:: lcp_solve(game, rational=True, use_strategic=False, external=False, stop_after=None, max_depth=None)

   Compute Nash equilibria of a two-player game using :ref:`linear
   complementarity programming <gambit-lcp>`.

   :param bool rational: Compute using rational precision (more
			 precise, often much slower)
   :param bool use_strategic: Use the strategic form version even for
			      extensive games
   :param bool external: Call the external command-line solver instead
			 of the internally-linked implementation
   :param int stop_after: Number of equilibria to contribute (default
			  is to compute until all reachable equilbria
			  are found)
   :param int max_depth: Maximum recursion depth (default is no limit)
   :raises RuntimeError: if game has more than two players.


.. py:function:: lp_solve(game, rational=True, use_strategic=False, external=False)

   Compute Nash equilibria of a two-player constant-sum game using :ref:`linear
   programming <gambit-lp>`.

   :param bool rational: Compute using rational precision (more
			 precise, often much slower)
   :param bool use_strategic: Use the strategic form version even for
			      extensive games
   :param bool external: Call the external command-line solver instead
			 of the internally-linked implementation
   :raises RuntimeError: if game has more than two players.

.. py:function:: simpdiv_solve(game, external=False)

   Compute Nash equilibria of a game using :ref:`simplicial
   subdivision <gambit-simpdiv>`.

   :param bool external: Call the external command-line solver instead
			 of the internally-linked implementation

.. py:function:: ipa_solve(game, external=False)

   Compute Nash equilibria of a game using :ref:`iterated polymatrix
   approximation <gambit-ipa>`.

   :param bool external: Call the external command-line solver instead
			 of the internally-linked implementation

.. py:function:: gnm_solve(game, external=False)

   Compute Nash equilibria of a game using :ref:`the global Newton
   method <gambit-gnm>`.

   :param bool external: Call the external command-line solver instead
			 of the internally-linked implementation
			 

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
