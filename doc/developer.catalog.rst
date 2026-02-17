Updating the Games Catalog
==========================

This page covers the process for contributing to and updating Gambit's :ref:`Games Catalog <pygambit-catalog>`.
To do so, you will need to have the `gambit` GitHub repo cloned and be able to submit pull request via GitHub;
you may wish to first review the :ref:`contributor guidelines <contributing>`.

You can add games to the catalog saved in a valid representation :ref:`format <file-formats>`.
Currently supported representations are:

- `.efg` for extensive form games
- `.nfg` for normal form games

Add new game files
------------------

1. **Create the game file:**

   Use either :ref:`pygambit <pygambit>`, the Gambit :ref:`CLI <command-line>` or :ref:`GUI <section-gui>` to create and save game in a valid representation :ref:`format <file-formats>`.

2. **Add the game file:**

   Create a new branch in the ``gambit`` repo. Add your new game file(s) inside the ``catalog`` dir and commit them.

3. **Update the catalog:**

   Use the ``update.py`` script to update Gambit's documentation & build files.

   .. code-block:: bash

       python build_support/catalog/update.py --build

   .. note::

      Run this script in a Python environment where ``pygambit`` itself is also :ref:`installed <build-python>`.

   .. warning::

      Running the script with the ``--build`` flag updates `Makefile.am`. If you moved games that were previously in `contrib/games` you'll need to also manually remove those files from `EXTRA_DIST`.

4. **Submit a pull request to GitHub with all changes.**

   .. warning::

      Make sure you commit all changed files e.g. run ``git add --all`` before committing and pushing.

Code new games & add game families
----------------------------------

1. **Add the game code:**

   Open `catalog/__init__.py` and create a new function, or modify an existing one. Ensure your function returns a ``Game`` object.
   You may wish to vary the game title and/or description based on the chosen parameters.

2. **Update the catalog:**

   Update the dictionary returned by ``family_games()`` in `catalog/__init__.py` with all variants of your game(s) you want in the catalog.
   Ensure each entry has unique game slug as key (this will be used by ``pygambit.catalog.load('slug')``), and returns a call of the function with specific parameters.

3. **Submit a pull request to GitHub with all changes.**
