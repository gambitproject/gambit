.. _catalog:

Catalog of games
================

Below is a complete list of games included in Gambit's catalog.
Check out the :ref:`pygambit API reference <pygambit-catalog>` for instructions on how to search and load these games in Python, and the :ref:`Updating the games catalog <updating-catalog>` guide for instructions on how to contribute new games to the catalog.

.. _catalog-openspiel:

Loading OpenSpiel games
-----------------------

Games from the `OpenSpiel <https://openspiel.readthedocs.io/en/latest/games.html>`_ library
can be loaded dynamically using the ``open_spiel/`` prefix:

.. code-block:: python

   pygambit.catalog.load("open_spiel/matrix_rps")
   pygambit.catalog.load("open_spiel/tiny_hanabi")

This requires ``open_spiel`` to be installed (``pip install open_spiel``;
not available on Windows). The game is exported to NFG or EFG format on the fly
and loaded into Gambit. Not all OpenSpiel games can be exported; a
:class:`ValueError` is raised for games that are incompatible with either format.
See the OpenSpiel interoperability tutorial for worked examples.

.. include:: catalog_table.rst
