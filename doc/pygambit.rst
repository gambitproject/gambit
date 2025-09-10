.. _pygambit:


PyGambit
========

Gambit provides a Python package, ``pygambit``, which is available on `PyPI
<https://pypi.org/project/pygambit/>`_ and can be installed with pip::

   pip install pygambit


The goal of these tutorials is to introduce users to the Gambit API and its capabilities for analyzing and solving Game Theory games.

Tutorials 1-3 assume no prior knowledge of Game Theory or the Gambit API and provide detailed explanations of the concepts and code used.

Tutorials 4-6 follow from tutorials 1-3 and do not re-explain the fundamentals of the Gambit API.

Tutorial 4 assumes some familiarity with Game Theory terminology and concepts including:
- Nash equilibria
- Mixed strategies
- Simplex representations of available strategies

.. toctree::
   :maxdepth: 2

   tutorials/running_locally
   tutorials/01_quickstart
   tutorials/02_extensive_form
   tutorials/03_poker
   tutorials/04_starting_points
   tutorials/05_quantal_response
   pygambit.external_programs


API documentation
----------------

Interfaces to algorithms for computing Nash equilibria are provided in :py:mod:`pygambit.nash`.
The table below summarizes the available PyGambit functions and the corresponding Gambit CLI commands.

==========================================    ========================================
PyGambit function                             CLI command
==========================================    ========================================
:py:func:`pygambit.nash.enumpure_solve`      :ref:`gambit-enumpure <gambit-enumpure>`
:py:func:`pygambit.nash.enummixed_solve`     :ref:`gambit-enummixed <gambit-enummixed>`
:py:func:`pygambit.nash.lp_solve`            :ref:`gambit-lp <gambit-lp>`
:py:func:`pygambit.nash.lcp_solve`           :ref:`gambit-lcp <gambit-lcp>`
:py:func:`pygambit.nash.liap_solve`          :ref:`gambit-liap <gambit-liap>`
:py:func:`pygambit.nash.logit_solve`         :ref:`gambit-logit <gambit-logit>`
:py:func:`pygambit.nash.simpdiv_solve`       :ref:`gambit-simpdiv <gambit-simpdiv>`
:py:func:`pygambit.nash.ipa_solve`           :ref:`gambit-ipa <gambit-ipa>`
:py:func:`pygambit.nash.gnm_solve`           :ref:`gambit-gnm <gambit-gnm>`
==========================================    ========================================

.. toctree::
   :maxdepth: 2

   pygambit.api