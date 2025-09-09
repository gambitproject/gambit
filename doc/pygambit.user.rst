.. _pygambit-user:

User guide
----------

Representation of numerical data of a game
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Payoffs to players and probabilities of actions at chance information sets are specified
as numbers.  Gambit represents the numerical values in a game in exact precision,
using either decimal or rational representations.

To illustrate, we consider a trivial game which just has one move for the chance player:

.. ipython:: python

   import pygambit as gbt
   g = gbt.Game.new_tree()
   g.append_move(g.root, g.players.chance, ["a", "b", "c"])
   [act.prob for act in g.root.infoset.actions]

The default when creating a new move for chance is that all actions are chosen with
equal probability.  These probabilities are represented as rational numbers,
using ``pygambit``'s :py:class:`.Rational` class, which is derived from Python's
`fractions.Fraction`.  Numerical data can be set as rational numbers:

.. ipython:: python

  g.set_chance_probs(g.root.infoset,
                     [gbt.Rational(1, 4), gbt.Rational(1, 2), gbt.Rational(1, 4)])
  [act.prob for act in g.root.infoset.actions]

They can also be explicitly specified as decimal numbers:

.. ipython:: python

   g.set_chance_probs(g.root.infoset,
                      [gbt.Decimal(".25"), gbt.Decimal(".50"), gbt.Decimal(".25")])
   [act.prob for act in g.root.infoset.actions]

Although the two representations above are mathematically equivalent, ``pygambit``
remembers the format in which the values were specified.

Expressing rational or decimal numbers as above is verbose and tedious.
``pygambit`` offers a more concise way to express numerical data in games:
when setting numerical game data, ``pygambit`` will attempt to convert text strings to
their rational or decimal representation.  The above can therefore be written
more compactly using string representations:

.. ipython:: python

   g.set_chance_probs(g.root.infoset, ["1/4", "1/2", "1/4"])
   [act.prob for act in g.root.infoset.actions]

   g.set_chance_probs(g.root.infoset, [".25", ".50", ".25"])
   [act.prob for act in g.root.infoset.actions]

As a further convenience, ``pygambit`` will accept Python ``int`` and ``float`` values.
``int`` values are always interpreted as :py:class:`.Rational` values.
``pygambit`` attempts to render `float` values in an appropriate :py:class:`.Decimal`
equivalent.  In the majority of cases, this creates no problems.
For example,

.. ipython:: python

   g.set_chance_probs(g.root.infoset, [.25, .50, .25])
   [act.prob for act in g.root.infoset.actions]

However, rounding can cause difficulties when attempting to use `float` values to
represent values which do not have an exact decimal representation

.. ipython:: python
   :okexcept:

   g.set_chance_probs(g.root.infoset, [1/3, 1/3, 1/3])

This behavior can be slightly surprising, especially in light of the fact that
in Python,

.. ipython:: python

   1/3 + 1/3 + 1/3

In checking whether these probabilities sum to one, ``pygambit`` first converts each
of the probabilitiesto a :py:class:`.Decimal` representation, via the following method

.. ipython:: python

   gbt.Decimal(str(1/3))

and the sum-to-one check then fails because

.. ipython:: python

   gbt.Decimal(str(1/3)) + gbt.Decimal(str(1/3)) + gbt.Decimal(str(1/3))

Setting payoffs for players also follows the same rules.  Representing probabilities
and payoffs exactly is essential, because ``pygambit`` offers (in particular for two-player
games) the possibility of computation of equilibria exactly, because the Nash equilibria
of any two-player game with rational payoffs and chance probabilities can be expressed exactly
in terms of rational numbers.

It is therefore advisable always to specify the numerical data of games either in terms
of :py:class:`.Decimal` or :py:class:`.Rational` values, or their string equivalents.
It is safe to use `int` values, but `float` values should be used with some care to ensure
the values are recorded as intended.



Available Nash equilibria algorithms
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Interfaces to algorithms for computing Nash equilibria are provided in :py:mod:`pygambit.nash`.

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

Using external programs to compute Nash equilibria
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Because the problem of finding Nash equilibria can be expressed in various
mathematical formulations (see [McKMcL96]_), it is helpful to make use
of other software packages designed specifically for solving those problems.

There are currently two integrations offered for using external programs to solve
for equilibria:

- :py:func:`.enummixed_solve` supports enumeration of equilibria in
  two-player games via `lrslib`.  [#lrslib]_
- :py:func:`.enumpoly_solve` supports computation of totally-mixed equilibria
  on supports in strategic games via `PHCpack`.  [#phcpack]_

For both calls, using the external program requires passing the path to the
executable (via the `lrsnash_path` and `phcpack_path` arguments, respectively).

The user must download and compile or install these programs on their own; these are
not packaged with Gambit. The solver calls do take care of producing the required
input files, and reading the output to convert into Gambit objects for further
processing.


.. [#lrslib] http://cgm.cs.mcgill.ca/~avis/C/lrs.html

.. [#phcpack] https://homepages.math.uic.edu/~jan/PHCpack/phcpack.html

.. [McKMcL96] McKelvey, Richard D. and McLennan, Andrew M.  (1996) Computation of equilibria
              in finite games.  In Handbook of Computational Economics, Volume 1,
              pages 87-142.
