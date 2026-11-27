.. _catalog-versions:

Using a version of the catalog of games
=======================================

Gambit's `catalog of games <https://gambit-catalog.readthedocs.io/>`_ is developed and released
separately from Gambit.  Each pygambit release includes one release of the catalog, which
:func:`pygambit.catalog.load` and :func:`pygambit.catalog.games` use, and whose version
:func:`pygambit.catalog.version` reports:

.. code-block:: python

   import pygambit as gbt

   gbt.catalog.version()
   g = gbt.catalog.load("journals/geb/bagwell1995")

To use another version of the catalog, newer or older, create a :class:`pygambit.catalog.Catalog`
with the version you want.  Versions other than the one included with pygambit are downloaded
once and kept in a cache locally:

.. code-block:: python

   catalog = gbt.catalog.Catalog("0.2.0")
   catalog.games()
   g = catalog.load("journals/geb/bagwell1995")


Unreleased games
----------------

A catalog can also be named by a commit of the
`catalog's repository <https://github.com/gambitproject/gambit-catalog>`_, to use games that have
been added but not yet released.  A branch name, tag or abbreviated SHA gives whatever commit it
names at the time; the full SHA it resolved to is :attr:`~pygambit.catalog.Catalog.ref`, which
gives the same games when passed again:

.. code-block:: python

   catalog = gbt.catalog.Catalog("main")
   catalog.ref   # the full SHA of the commit

A copy of the catalog on your own computer, such as a checkout of its repository, can be read with
``Catalog(path=...)``.  Setting the environment variable ``GAMBIT_CATALOG_DIR`` to the ``games``
directory of such a copy makes it the catalog used by :func:`pygambit.catalog.load` and
:func:`pygambit.catalog.games`, and by the graphical interface.
