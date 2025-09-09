.. _pygambit-user:

User guide
----------

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
