.. _update-the-catlog:

Updating the Games Catalog
==========================

This page covers the process for contributing to and updating Gambit's :ref:`Games Catalog <pygambit-catalog>`.
To do so, you will need to have the `gambit` GitHub repo cloned and be able to submit pull request via GitHub;
you may wish to first review the :ref:`contributor guidelines <contributing>`.

You can add games to the catalog saved in a valid representation :ref:`format <file-formats>`, e.g. for `.efg` for extensive form games.
Alternatively, you can define games for the catalog in *pygambit* code.
You may wish to first review the :ref:`pygambit <pygambit>` docs pages.

.. dropdown:: Add game files to the catalog
   :class-container: sd-border-0

   1. **Create the game file:**

      Use either :ref:`pygambit <pygambit>`, the Gambit :ref:`CLI <command-line>` or :ref:`GUI <section-gui>` to create and save game in a valid representation :ref:`format <file-formats>`.

   2. **Add the game file:**

      Create a new branch in the `gambit` repo and commit the new game to `src/pygambit/catalog_game_files`

   3. **Update the catalog:**

      Use the `catalog_update.py` script to update the catalog and associated documatation & build files.

      .. code-block:: bash

         pip install ruamel.yaml
         cd src/pygambit
         python catalog_update.py

      .. note::
       Run this script in a Python environment where `pygambit` itself is also :ref:`installed <build-python>`

   4. **[Optional] Edit the catalog entry:**

      Open `src/pygambit/catalog.yml` and find the new entry that was created in the previous step.
      The entry will be named after the game file, but in camel case, with "Game" prefixed if the file started with a number.

      If the game file doesn't already include a description ("comments" in older versions of Gambit) you should add a description field:

      .. code-block:: yaml

        MyGame:
          file: my_game.nfg
          description: "A game that... originally created by Author (2000)."

   5. **[Optional] Add custom metadata to catalog games:**

      Open `src/pygambit/catalog.py` and add update the `CatalogGame` base class with the new metadata field.
      Include a type hint, a default value and a docstring.

      .. code-block:: python

        # Metadata fields
        my_metadata: int | None = None
        """Explanation of my_metadata field."""

      Open `src/pygambit/catalog.yml` and add the field under the `metadata` for the game you added, and any others that shouldn't have the default.

      .. code-block:: yaml

        MyGame:
          file: my_game.efg
          metadata:
            my_metadata: 17

   6. **Submit a pull request to GitHub with all changes:**

      .. warning::
        If you made changes in step 4 or 5 above, re-run the update script from step 3.

      .. warning::
        Make sure you commit all changed files e.g. run `git add --all` before committing and pushing.

.. dropdown:: Code games for the catalog
   :class-container: sd-border-0

   1. **Write the pygambit code:**

      Write code for your game with :ref:`pygambit <pygambit>`.
      Your code should create a ``Game`` object with a title and description.

      .. note::
        Test your game by visualising it in `draw_tree` and running Gambit's solvers.

   2. **Create a new Python class for your game:**

      Open `src/pygambit/catalog_games.py` and add a subclass of `CatalogGame` with your code implemented as the `_game` function,
      which should be defined as a `staticmethod` returning a ``Game`` object.
      You can optionally include paramaters to generate game variants.

      .. code-block:: python

        class MyGame(gbt.catalog.CatalogGame):

            @staticmethod
            def _game(some_param: bool = False) -> gbt.Game:
                """
                Docstring for _game function.

                Parameters
                ----------
                some_param : bool, optional
                    Description of optional paramater.
                    Defaults to False.

                Returns
                -------
                gbt.Game
                    The constructed game.

                Examples
                --------
                >>> MyGame(some_param=False) # Constructs the standard game
                >>> MyGame(some_param=True) # Constructs the alternate game
                """
                g = gbt.Game.new_tree(
                    players=["Buyer", "Seller"], title="My game"
                )
                g.description = "Longer description of My game, originally by Author (2000)."
                g.append_move(g.root, "Buyer", ["Trust", "Not trust"])
                g.append_move(g.root.children[0], "Seller", ["Honor", "Abuse"])
                g.set_outcome(g.root.children[0].children[0], g.add_outcome([1, 1], label="Trustworthy"))
                if some_param:
                    g.set_outcome(
                        g.root.children[0].children[1], g.add_outcome(["1/2", 2], label="Untrustworthy")
                    )
                else:
                    g.set_outcome(
                        g.root.children[0].children[1], g.add_outcome([-1, 2], label="Untrustworthy")
                    )
                g.set_outcome(g.root.children[1], g.add_outcome([0, 0], label="Opt-out"))
                return g

      .. note::
        If you add a docstring to the class itself, this will be used in the API reference docs as the game description instead of `g.description`

   3. **Update the catalog:**

      Use the `catalog_update.py` script to update the catalog and associated documatation & build files.

      .. code-block:: bash

         pip install ruamel.yaml
         cd src/pygambit
         python catalog_update.py

      .. note::
       Run this script in a Python environment where `pygambit` itself is also :ref:`installed <build-python>`

   4. **[Optional] Add custom metadata to catalog games:**

      Open `src/pygambit/catalog.py` and add update the `CatalogGame` base class with the new metadata field.
      Include a type hint, a default value and a docstring.

      .. code-block:: python

        # Metadata fields
        my_metadata: int | None = None
        """Explanation of my_metadata field."""

      Open `src/pygambit/catalog_games.py` and add a value for the field as a class attribute for the game(s) you added.

      .. code-block:: python

        class MyGame(gbt.catalog.CatalogGame):
            my_metadata = 17
        ...

   6. **Submit a pull request to GitHub with all changes:**

      .. warning::
        If you made changes in step 4 above, re-run the update script from step 3.

      .. warning::
        Make sure you commit all changed files e.g. run `git add --all` before committing and pushing.
