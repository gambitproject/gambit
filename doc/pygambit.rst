.. _pygambit:


PyGambit
========

Gambit provides a Python package, ``pygambit``, which is available on `PyPI
<https://pypi.org/project/pygambit/>`_ and can be installed with pip::

   pip install pygambit

Tutorials
---------

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

User guide
----------

.. toctree::
   :maxdepth: 2

   pygambit.user

API documentation
----------------

.. toctree::
   :maxdepth: 2

   pygambit.api