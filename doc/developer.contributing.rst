.. _contributing:

Contributing to Gambit
======================

This section provides guidelines for contributing to Gambit, including how to report bugs, suggest features, and contribute code.
It includes information relevant to both core developers and external contributors.

GitHub issues
-------------

In the first instance, bug reports, feature requests and improvements to the Gambit documentation should be
posted to the Gambit issue tracker, located at
`<http://github.com/gambitproject/gambit/issues>`_.
Use the issue templates to help you provide the necessary information.

When reporting a bug, please be sure to include the following:

* The version(s) of Gambit you are using.  (If possible, it is helpful
  to know whether a bug exists in both the current stable/teaching and
  the current development/research versions.)
* The operating system(s) on which you encountered the bug.
* A detailed list of steps to reproduce the bug.  Be sure to include a
  sample game file or files if appropriate; it is often helpful to
  simplify the game if possible.

.. _contributing-code:

Contributing code
-----------------

Gambit is an open-source project, and contributions are welcome from anyone.
The project is hosted on GitHub, and contributions can be made via pull requests following the standard GitHub workflow.

In the git repository, the branch ``master`` always points to the
latest development version.  New development should in general always
be based off this branch.  Branches labeled ``maintX_Y``, where ``X`` is the major version number and ``Y`` is the minor version number, point to the latest commit on a stable
version.

1. To get started contributing code in the `Gambit GitHub repo <https://github.com/gambitproject/gambit>`__, do one of the following:

   - Core developers: request contributor access from one of the `team <https://www.gambit-project.org/team/>`__
   - External contributors: fork the repository on GitHub.

2. Clone the repository to your local machine ::

    git clone https://github.com/gambitproject/gambit.git  # or your fork URL
    cd gambit

3. Follow the instructions in the :ref:`building-from-source` page to set up your development environment and build Gambit from source. If you only plan to make changes to the PyGambit Python code, you can skip to :ref:`build-python`.

4. *[Optional but recommended]* Install `pre-commit` which is used to run code formatters and linters before each commit. This helps ensure code quality and consistency. You can install it into the virtual environment where you installed PyGambit ::

    pip install pre-commit

   Alternatively, `pre-commit` is also available via various packaging systems.  For example, you can install via `Homebrew <https://brew.sh>`__ ::

    brew install pre-commit

   If you install via a package manager (instead of into the virtual environment), `pre-commit` will be available for use with other projects.

   Then, set it up in the Gambit repository ::

    pre-commit install

   Having `pre-commit` installed is recommended as it runs many of the same checks that are automatically conducted on any pull request.  Every time you commit, it will automatically fix some issues and highlight others for manual adjustment.

5. Create a new branch for your changes. It's good practice to either give the branch a descriptive name or directly reference an issue number ::

    git checkout -b feature/issue-number

6. Make your changes. ::

    git add <files>
    git commit -m "Clear and descriptive commit message"

   Provide a clear commit message.  Gambit does not have its own set of guidelines for commit messages.
   However, there are a number of webpages that have suggestions for writing effective commit messages (and for deciding how to structure your contributions as one or more
   commits as appropriate).  See for example `this page <https://wiki.openstack.org/wiki/GitCommitMessages>`__.

7. Push your changes to your fork or branch ::

    git push origin feature/issue-number

8. Open a pull request on GitHub to the master branch of the upstream repository. Ensure your pull request:

   - Has an informative title.
   - Links to any relevant issues.
   - Includes a clear description of the changes made.
   - Explains how a reviewer can test the changes.

   .. note::
      It's good practice to open a draft pull request early in the development process to facilitate discussion and feedback, ensure there are no merge conflicts, and allow for continuous integration testing via GitHub Actions.

9. Core developers will review your changes, provide feedback, and merge them into the master branch if they meet the project's standards.

10. Once your pull request is merged, delete your branch on GitHub (a button should appear to do this automatically).

Testing your changes
--------------------

Be sure to familiarise yourself with :ref:`contributing-code` before reading this section.

By default, pull requests on GitHub will trigger the running of Gambit's test suite using GitHub Actions.
You can also run the tests locally before submitting your pull request, using `pytest`.

1. Ensure `pygambit` is installed with test dependencies: see :ref:`build-python`.

2. Run pytest: ::

    pytest --run-tutorials

.. tip::
    You can omit the `--run-tutorials` to skip running the tutorial notebook tests which take longest to run.
    Running tests including tutorials requires `doc` as well as `test` dependencies; see :ref:`build-python`.

Adding to the test suite
^^^^^^^^^^^^^^^^^^^^^^^^

Tests can be added to the test suite by creating new test files in the ``tests`` directory.
Tests should be written using the `pytest` framework.
Refer to existing test files for examples of how to write tests or see the `pytest documentation <https://docs.pytest.org/en/stable/>`_ for more information.


Editing this documentation
---------------------------

Be sure to familiarise yourself with :ref:`contributing-code` before reading this section.

You can make changes to the documentation by editing the `.rst` files in the ``doc`` directory.
Creating a pull request with your changes will automatically trigger a build of the documentation via the ReadTheDocs service, which can be viewed online.
You can also build the documentation locally to preview your changes before submitting a pull request.

1. `Install Pandoc <https://pandoc.org/installing.html>`_ for your OS

2. Ensure `pygambit` is installed with doc dependencies: see :ref:`build-python`.

3. Navigate to the Gambit repo and build the docs: ::

    cd doc
    make html  # or make livehtml for live server with auto-rebuild

4. Open ``doc/_build/html/index.html`` in your browser to view the documentation.

Contributing tutorials
^^^^^^^^^^^^^^^^^^^^^^

To submit a tutorial for inclusion in the Gambit documentation, please follow these steps:

1. Open a GitHub issue using the *Tutorial request* issue template on the `Gambit GitHub repo <https://github.com/gambitproject/gambit/issues/new/choose>`__ or choose an issue already opened with the `tutorial` label.

2. Write the tutorial as a Jupyter notebook (`.ipynb` file), following the style and format of existing tutorials in the `doc/tutorials` directory. Develop this on a branch as per the instructions in :ref:`contributing-code`. Add the tutorial to the `doc/tutorials` directory in the repository. Put it in an appropriate subdirectory or create a new one if necessary.

3. Update `doc/pygambit.rst` to ensure the tutorial is listed in the docs at an appropriate location.

4. *[Optional]* If your tutorial requires additional dependencies not already listed in the ``doc`` list under ``[project.optional-dependencies]`` inside ``pyproject.toml``, please add them to the file.


Recognising contributions
-------------------------

Gambit is set up with `All Contributors <https://allcontributors.org/>`__ to recognise all types of contributions, including code, documentation, bug reports, and more.

You can see the list of contributors on the README page of the `Gambit GitHub repo <https://github.com/gambitproject/gambit>`__.

To add a contributor, comment on a GitHub Issue or Pull Request, asking @all-contributors to add a contributor:

 @all-contributors please add @<username> for <contributions>

Refer to the `emoji key <https://allcontributors.org/docs/en/emoji-key>`__ for a list of contribution types.

Releases & maintenance branches
-------------------------------

Releases of Gambit are made by core developers.
Details of previous releases can be found in the `GitHub releases page <https://github.com/gambitproject/gambit/releases>`__.

Branches labeled ``maintX`` and ``maintX_Y``, where ``X`` is the major version number and ``Y`` is the minor version number, are associated with releases and point to the latest commit on a stable version.
Navigate to the Gambit repository on GitHub and select the `branches` tab to see the list of active maintenance branches.
Be sure to delete any maintenance branches that are no longer being maintained.

.. _making-a-new-release:

Making a new release
^^^^^^^^^^^^^^^^^^^^

When making a new release of Gambit, follow these steps:

1. Create a new branch from the latest commit on the ``master`` branch named ``maintX_Y``, where ``X`` is the major version number and ``Y`` is the minor version number of the new release.

2. Update the version number in the ``build_support/GAMBIT_VERSION`` file to ``X.Y.Z``.

   All other files will automatically use the updated version number:

   - `pyproject.toml` reads from GAMBIT_VERSION file at build time
   - `configure.ac` reads from GAMBIT_VERSION file and substitutes into `build_support/osx/Info.plist` and `build_support/msw/gambit.wxs`
   - `src/pygambit/__init__.py` reads from installed package metadata or GAMBIT_VERSION file
   - `doc/conf.py` reads from GAMBIT_VERSION file at documentation build time
   - Documentation pages reference the `|release|` substitution variable to automatically reflect the updated version number.

3. Update the `ChangeLog` file with a summary of changes

4. Once there are no further commits to be made for the release, create a tag for the release from the latest commit on the maintenance branch. ::

    git tag -a vX.Y.Z -m "Gambit version X.Y.Z"

5. Push the maintenance branch and tags to the GitHub repository. ::

    git push origin maintX_Y
    git push origin --tags

6. Create a new release on the `GitHub releases page <https://github.com/gambitproject/gambit/releases>`__, using the tag created in step 4.
   Include a summary of changes from the `ChangeLog` file in the release notes.

7. Currently there is no automated process for pushing the new release to PyPI. This must be done manually.

.. TODO: update this process to be automated via GitHub Actions: Issue #557

Patching maintained versions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you have bug fixes that should be applied across maintained versions:

1. Make a branch from the oldest maintenance branch to which the change applies
2. Apply the change and make a pull request to that maintenance branch
3. After the pull request is merged, open new pull requests to each subsequent maintenance branch and finally to the master branch, merging each in turn
4. Create new releases from each maintenance branch as needed, following the steps in :ref:`making-a-new-release`.
