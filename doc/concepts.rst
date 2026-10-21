.. _concepts:

Concepts and conventions
=========================



.. _concepts-representations:

Game representations
--------------------


.. _concepts-players-actions-strategies:

Players, actions, and strategies
----------------------------------



.. _concepts-outcomes:

Outcomes
---------


.. _concepts-numerical-data:

Numerical data conventions
-----------------------------

Payoffs to players and probabilities of actions at events are specified as numbers.
Gambit represents the numerical values in a game in exact precision, using either decimal
or rational representations.

To illustrate, consider a trivial game which just has one move for the chance player:

.. code-block:: python

   small_game = gbt.ExtensiveGame()
   small_game.append_event(gbt.H.path(), dict.fromkeys(["a", "b", "c"], gbt.Rational(1, 3)))
   list(small_game.get_action_probs(gbt.H.path()).values())

Here we specified an equal-probability distribution over the three actions, using
``pygambit``'s :py:class:`.Rational` class, which is derived from Python's
``fractions.Fraction``, to represent the probabilities exactly.

Numerical data can be set as rational numbers.
:py:meth:`.ExtensiveGame.make_event` forms a collection of nodes into an event with a given
distribution; applied to a single node which is already an event, it resets the probabilities.
We can use it to change the distribution, again with ``Rational`` numbers:

.. code-block:: python

   small_game.make_event(
       gbt.H.path(),
       {"a": gbt.Rational(1, 4), "b": gbt.Rational(1, 2), "c": gbt.Rational(1, 4)}
   )
   list(small_game.get_action_probs(gbt.H.path()).values())

Numerical data can also be explicitly specified as decimal numbers:

.. code-block:: python

   small_game.make_event(
       gbt.H.path(),
       {"a": gbt.Decimal(".25"), "b": gbt.Decimal(".50"), "c": gbt.Decimal(".25")}
   )
   list(small_game.get_action_probs(gbt.H.path()).values())

Although the two representations above are mathematically equivalent, ``pygambit``
remembers the format in which the values were specified.

Expressing rational or decimal numbers as above is verbose and tedious.
``pygambit`` offers a more concise way to express numerical data in games: when setting
numerical game data, ``pygambit`` will attempt to convert text strings to their rational or
decimal representation.  The above can therefore be written more compactly using string
representations:

.. code-block:: python

   small_game.make_event(gbt.H.path(), {"a": "1/4", "b": "1/2", "c": "1/4"})
   list(small_game.get_action_probs(gbt.H.path()).values())

   small_game.make_event(gbt.H.path(), {"a": ".25", "b": ".50", "c": ".25"})
   list(small_game.get_action_probs(gbt.H.path()).values())

As a further convenience, ``pygambit`` will accept Python ``int`` and ``float`` values.
``int`` values are always interpreted as :py:class:`.Rational` values.

``pygambit`` attempts to render ``float`` values in an appropriate :py:class:`.Decimal`
equivalent.  In the majority of cases, this creates no problems.  For example:

.. code-block:: python

   small_game.make_event(gbt.H.path(), {"a": .25, "b": .50, "c": .25})
   list(small_game.get_action_probs(gbt.H.path()).values())

However, rounding can cause difficulties when attempting to use ``float`` values to
represent values which do not have an exact decimal representation:

.. code-block:: python

   small_game.make_event(gbt.H.path(), {"a": 1/3, "b": 1/3, "c": 1/3})
   # ValueError: ...

This behavior can be slightly surprising, especially in light of the fact that in Python,

.. code-block:: python

   1/3 + 1/3 + 1/3
   # 1.0

In checking whether these probabilities sum to one, ``pygambit`` first converts each of the
probabilities to a :py:class:`.Decimal` representation, via the following method:

.. code-block:: python

   gbt.Decimal(str(1/3))

and the sum-to-one check then fails because:

.. code-block:: python

   gbt.Decimal(str(1/3)) + gbt.Decimal(str(1/3)) + gbt.Decimal(str(1/3))
   # Decimal('0.9999999999999999166...')

Setting payoffs for players also follows the same rules.  Representing probabilities and
payoffs exactly is essential, because ``pygambit`` offers (in particular for two-player
games) the possibility of computation of equilibria exactly, because the Nash equilibria of
any two-player game with rational payoffs and chance probabilities can be expressed exactly
in terms of rational numbers.

It is therefore advisable always to specify the numerical data of games either in terms of
:py:class:`.Decimal` or :py:class:`.Rational` values, or their string equivalents.  It is
safe to use ``int`` values, but ``float`` values should be used with some care to ensure the
values are recorded as intended.



.. _concepts-information-sets:

Information sets and imperfect information
---------------------------------------------


.. _concepts-strategy-types:

Mixed strategy profiles and mixed behavior profiles
-----------------------------------------------------

A probability distribution over a player's strategies is a ``MixedStrategy``.
An assignment of a ``MixedStrategy`` to each player forms a ``MixedStrategyProfile``, where following the
standard convention in game theory it is assumed players randomize independently.

In an extensive game,
a probability distribution over a player's actions at an information set is a ``MixedAction``.
An assignment of a ``MixedAction`` to each information set of a player is a ``MixedBehavior``.
An assignment of a ``MixedBehavior`` to each player forms a ``MixedBehaviorProfile``.
Following the standard convention in game theory, it is assumed the randomization of each
``MixedAction`` in a profile is realized independently.

``MixedStrategyProfile`` and ``MixedBehaviorProfile`` objects express their probabilities either
as floating-point or rational numbers.  Each profile is one type or the other; the precision of
all probabilities is the same in any profile.

``MixedStrategy`` and ``MixedAction`` objects enforce that the probability assigned to each strategy
or action is nonnegative.  By convention, these values are not enforced to sum to one; if they
do not sum to one, they are interpreted as weights.  It is not valid to set all the probabilities/weights
of a ``MixedStrategy`` or ``MixedAction`` to zero.  ``MixedStrategy`` and ``MixedAction`` objects can be
normalized to sum to one via an explicit operation.
