.. _pygambit:


PyGambit
========

Gambit provides a Python package, ``pygambit``, which is available on `PyPI
<https://pypi.org/project/pygambit/>`_ and can be installed with pip::

   pip install pygambit


For newcomers to Gambit, we recommend reading through the PyGambit tutorials, which demonstrate the API's key capabilities for analyzing and solving Game Theory games.
These tutorials are available to be run interactively as Jupyter notebooks, see :ref:`local_tutorials`.

Tutorials **1-3** assume no prior knowledge of Game Theory or the Gambit API and provide detailed explanations of the concepts and code used.

Tutorials **4-5** demonstrate more advanced topics and do not re-explain the fundamentals of the Gambit API.
They also assume some familiarity with Game Theory terminology and concepts including:

- Nash equilibria
- Pure and mixed strategies
- Simplex representations of available strategies
- Logit quantal response equilibrium (LQRE) correspondence

.. toctree::
   :maxdepth: 2

   tutorials/01_quickstart
   tutorials/02_extensive_form
   tutorials/03_poker
   tutorials/04_starting_points
   tutorials/05_quantal_response
   tutorials/running_locally
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