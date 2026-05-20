.. _updating-catalog:

Updating the Games Catalog
==========================

This page includes developer notes regarding the catalog module, and the process for contributing to and updating Gambit's :ref:`Games Catalog <pygambit-catalog>`.
To do so, you will need to have the `gambit` GitHub repo cloned and be able to submit pull request via GitHub;
you may wish to first review the :ref:`contributor guidelines <contributing>`.
You'll also need to have a developer install of `pygambit` available in your Python environment, see :ref:`build-python`.

Add new game files
------------------

You can add games to the catalog saved in a valid representation :ref:`format <file-formats>`.
Currently supported representations are:

- `.efg` for extensive form games
- `.nfg` for normal form games

1. **Create the game file:**

   Use either :ref:`pygambit <pygambit>`, the Gambit :ref:`CLI <command-line>` or :ref:`GUI <section-gui>` to create and save game in a valid representation :ref:`format <file-formats>`.
   Make sure the game includes a description, with any citations referencing the :ref:`bibliography <bibliography>`.
   Use a full link to the bibliography entry, so the link can be accessed from the file directly, as well as being rendered in the docs e.g. ```Rei2008 <https://gambitproject.readthedocs.io/en/latest/biblio.html#Rei2008>`_``

2. **Add the game file:**

   Create a new branch in the ``gambit`` repo. Add your new game file(s) inside the ``catalog`` dir and commit them.

3. **Update the catalog:**

   Reinstall the package to pick up the new game file(s) in the ``pygambit.catalog`` module.
   Then use the ``update.py`` script to update Gambit's documentation & build files, as well as generating images for the new game(s).

   .. code-block:: bash

       pip install .
       python build_support/catalog/update.py --build

   .. note::

      Update the ``catalog_draw_tree_settings`` in ``build_support/catalog/update.py`` to change the default visualization parameters for your game(s).
      You can use the ``--regenerate-images`` flag when building the docs locally (readthedocs does this by default).

   .. warning::

      Running the script with the ``--build`` flag updates `Makefile.am`. If you moved games that were previously in `contrib/games` you'll need to also manually remove those files from `EXTRA_DIST`.

4. **Submit a pull request to GitHub with all changes.**

   .. warning::

      Make sure you commit all changed files e.g. run ``git add --all`` before committing and pushing.


Access from pygambit
--------------------

We keep the ``catalog`` directory at the top level of the repository because it is in principle independent
of the Python and C++ code.  However, in order to include these games with the Python package, there is a bit
of extra infrastructure.

In ``setup.py`` we have a custom build command which first copies the contents of ``catalog/`` into the build
directory for the Python package.  These are then exposed as data in the ``catalog_data`` directory (changing
the name to avoid confusion or clashes with ``catalog.py``, which is responsible for accessing the catalog).

The main implication is that if you are working via the Python package and you add new games to the catalog,
you will need to rebuild and reinstall the Python extension in order to access the new games.  That is, changing
the contents of the catalog is no different than changing any other source code in the Python package; you'll
need to execute ``pip install .`` after the addition or change.
