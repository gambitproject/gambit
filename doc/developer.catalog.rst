Updating the Games Catalog
==========================

This page covers the process for contributing to and updating Gambit's :ref:`Games Catalog <pygambit-catalog>`.
To do so, you will need to have the `gambit` GitHub repo cloned and be able to submit pull request via GitHub;
you may wish to first review the :ref:`contributor guidelines <contributing>`.

You can add games to the catalog saved in a valid representation :ref:`format <file-formats>`.
Currently supported representations are:

- `.efg` for extensive form games
- `.nfg` for normal form games

Add new games
-------------

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
