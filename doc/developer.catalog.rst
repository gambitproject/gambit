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

      On a new branch, commit the new game to `src/pygambit/catalog_game_files`

   3. **Update the catalog:**

      Use the `catalog_update.py` script to update the catalog and associated documatation & build files.

      .. code-block:: bash

         pip install ruamel.yaml
         cd src/pygambit
         python catalog_update.py

      .. note::
       Run this script in a Python environment where `pygambit` itself is also :ref:`installed <build-python>`

   4. **[Optional] Edit the catalog entry**

      - Open `src/pygambit/catalog.yml` and find the new entry that was created in the previous step.
      - The entry will be named after the game file, but in camel case, with "Game" prefixed if the file started with a number.
      - If you edit the name of the entry, re-run the update script in the step above.

   5. **[Optional] Add custom metadata to catalog games:**

      Open `src/pygambit/catalog.py` and add update the `CatalogGame` base class with the new metadata field.
      Include a type hint, a default value and a docstring.

      .. code-block:: python

        # Metadata fields
        my_metadata: int | None = None
        """Explanation of my_metadata field."""

      Open `src/pygambit/catalog.yml` and the field under the `metadata` for the game you added, and any others that shouldn't have the default.

      .. code-block:: yaml

        MyGame:
        file: my_game.efg
        metadata:
            my_metadata: 17

   6. Sumbmit a pull request to GitHub with all changes

      .. warning::
        Make sure you commit all changed files e.g. run `git add --all ` before committing and pushing.
