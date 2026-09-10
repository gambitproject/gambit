.. _catalog-generators:

Generating games for the catalog
=================================

Alongside the curated catalog of games (built and hosted separately; see the
`catalog's developer guide <https://gambit-catalog.readthedocs.io/en/latest/developer.html>`_
for how to contribute new games), pygambit can generate games on the fly from two
external sources: the OpenSpiel library and the GAMUT suite.

.. _catalog-openspiel:

.. rubric:: Loading OpenSpiel games

Games from the `OpenSpiel <https://openspiel.readthedocs.io/en/latest/games.html>`_ library
can be generated using :func:`pygambit.catalog.generate_openspiel`:

.. code-block:: python

   pygambit.catalog.generate_openspiel("matrix_rps")
   pygambit.catalog.generate_openspiel("tiny_hanabi")
   pygambit.catalog.generate_openspiel("blotto", params={"players": 2, "coins": 3, "fields": 2})

The ``params`` argument is forwarded directly to ``pyspiel.load_game``; see the
`OpenSpiel game list <https://openspiel.readthedocs.io/en/latest/games.html>`_ for
available parameters per game.

This requires ``open_spiel`` to be installed (``pip install open_spiel``;
not available on Windows). The game is exported to EFG or NFG format on the fly
and loaded into Gambit. Not all OpenSpiel games can be exported; a
:class:`ValueError` is raised for games that are incompatible with either format.
See the :doc:`OpenSpiel interoperability tutorial <tutorials/interoperability_tutorials/openspiel>` for worked examples.

.. _catalog-gamut:

.. rubric:: Generating GAMUT games

Games from the `GAMUT <http://gamut.stanford.edu>`_ generator suite can be created
using :func:`pygambit.catalog.generate_gamut`:

.. code-block:: python

   pygambit.catalog.generate_gamut("RandomGame", params={"players": 2, "actions": 3}, gamut_jar="/path/to/gamut.jar")
   pygambit.catalog.generate_gamut("CovariantGame", params={"players": 2, "actions": [3, 3]}, gamut_jar="/path/to/gamut.jar")
   pygambit.catalog.generate_gamut(
       "RandomGame",
       params={"players": 3, "actions": 4, "normalize": True, "min_payoff": 0, "max_payoff": 100},
       gamut_jar="/path/to/gamut.jar",
   )

The ``params`` argument maps directly to GAMUT command-line flags. Boolean flags such as
``-normalize`` and ``-int_payoffs`` are passed as ``True``; list values for ``-actions``
expand to space-separated tokens. See the
`GAMUT documentation <http://gamut.stanford.edu/documentation.htm>`_ for the full list
of game classes and parameters.

GAMUT requires Java (install from `java.com <https://www.java.com/en/download/>`_) and
``gamut.jar`` to be installed. Provide the path to ``gamut.jar`` via the ``gamut_jar``
argument or the ``GAMUT_JAR`` environment variable. Download GAMUT from
http://gamut.stanford.edu/.

.. dropdown:: All 35 GAMUT game classes

   .. jupyter-execute::
      :hide-code:

      import pygambit as gbt
      gbt.catalog.gamut_games()

See the :doc:`GAMUT interoperability tutorial <tutorials/interoperability_tutorials/gamut>` for worked examples.
