.. _pygambit:


PyGambit
========

See installation instructions in the :ref:`install` section.


For newcomers to Gambit, we recommend reading through the PyGambit tutorials, which demonstrate the API's key capabilities for analyzing and solving games.
All of these tutorials assume a basic knowledge of programming in Python.

You can run the tutorials interactively as Jupyter notebooks, see :ref:`local_tutorials`.

New user tutorials
------------------

These tutorials assume no prior knowledge of Game Theory or the PyGambit API and provide detailed explanations of the concepts and code.
They are numbered in the order they should be read.

.. toctree::
   :maxdepth: 2

   tutorials/01_quickstart
   tutorials/02_extensive_form
   tutorials/03_stripped_down_poker
   tutorials/04_creating_images

Advanced user tutorials
-----------------------

These tutorials assume some familiarity with the PyGambit API and Game Theory terminology and concepts including:

- Nash equilibria
- Pure and mixed strategies
- Simplex representations of available strategies
- Logit quantal response equilibrium (LQRE) correspondence

Advanced tutorials:

.. toctree::
   :maxdepth: 2

   tutorials/advanced_tutorials/starting_points
   tutorials/advanced_tutorials/quantal_response
   .. pygambit.external_programs

Interoperability tutorials
--------------------------

These tutorials demonstrate how to use PyGambit alongside other game-theoretic software packages.
These tutorials assume you have read the new user tutorials and are familiar with the PyGambit API, however they do not assume prior knowledge of the other software packages or an advanced understanding of Game Theory:

.. toctree::
   :maxdepth: 2

   tutorials/interoperability_tutorials/openspiel

API documentation
----------------

.. toctree::
   :maxdepth: 2

   pygambit.api
