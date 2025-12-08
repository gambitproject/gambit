.. _algorithms:

Nash equilibria algorithms
==========================

Interfaces to algorithms for computing Nash equilibria are provided in :py:mod:`pygambit.nash`.
The table below summarizes the available PyGambit functions and the corresponding Gambit CLI commands.

================  ===========================================================================   ========================================  ==========================================
Algorithm         Description                                                                   PyGambit function                         CLI command
================  ===========================================================================   ========================================  ==========================================
:ref:`enumpure`   Enumerate pure-strategy equilibria of a game                                  :py:func:`pygambit.nash.enumpure_solve`   :ref:`gambit-enumpure <gambit-enumpure>`
:ref:`enummixed`  Enumerate equilibria in a two-player game                                     :py:func:`pygambit.nash.enummixed_solve`  :ref:`gambit-enummixed <gambit-enummixed>`
:ref:`lp`         Compute equilibria in a two-player constant-sum game via linear programming   :py:func:`pygambit.nash.lp_solve`         :ref:`gambit-lp <gambit-lp>`
:ref:`lcp`        Compute equilibria in a two-player game via linear complementarity            :py:func:`pygambit.nash.lcp_solve`        :ref:`gambit-lcp <gambit-lcp>`
:ref:`liap`       Compute Nash equilibria using function minimization                           :py:func:`pygambit.nash.liap_solve`       :ref:`gambit-liap <gambit-liap>`
:ref:`logit`      Compute quantal response equilbria                                            :py:func:`pygambit.nash.logit_solve`      :ref:`gambit-logit <gambit-logit>`
:ref:`simpdiv`    Compute equilibria via simplicial subdivision                                 :py:func:`pygambit.nash.simpdiv_solve`    :ref:`gambit-simpdiv <gambit-simpdiv>`
:ref:`ipa`        Compute Nash equilibria using iterated polymatrix approximation               :py:func:`pygambit.nash.ipa_solve`        :ref:`gambit-ipa <gambit-ipa>`
:ref:`gnm`        Compute Nash equilibria using a global Newton method                          :py:func:`pygambit.nash.gnm_solve`        :ref:`gambit-gnm <gambit-gnm>`
================  ===========================================================================   ========================================  ==========================================

.. _enumpure:

enumpure
--------

Reads a game on standard input and searches for pure-strategy Nash equilibria.

.. _enummixed:

enummixed
---------

.. _enumpoly:

enumpoly
--------

.. _lcp:

lcp
---

.. _lp:

lp
---

.. _liap:

liap
----

.. _simpdiv:

simpdiv
--------

.. _logit:

logit
-----

.. _gnm:

gnm
---

.. _ipa:

ipa
---

.. _convert:

convert
-------
