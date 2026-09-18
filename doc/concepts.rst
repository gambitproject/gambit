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



.. _concepts-regret:

Regret and the Liapunov value
--------------------------------
