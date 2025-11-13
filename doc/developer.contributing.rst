Contributing to Gambit
======================

This section provides guidelines for contributing to Gambit, including how to report bugs, suggest features, and contribute code.
It includes information relevant to both core developers and external contributors.

GitHub issues
----------------

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
----------------

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

4. Create a new branch for your changes ::

    git checkout -b feature/your-feature-name

5. Make your changes. Commit each change with a clear commit message ::

    git add .
    git commit -m "Add feature X or fix bug Y"

6. Push your changes to your fork or branch ::

    git push origin feature/your-feature-name

7. Open a pull request on GitHub to the master branch of the upstream repository, describing your changes and linking to any relevant issues.
8. Core developers will review your changes, provide feedback, and merge them into the master branch if they meet the project's standards.

Testing your changes
--------------------

Be sure to familiarise yourself with :ref:`contributing-code` before reading this section.

By default, pull requests on GitHub will trigger the running of Gambit's test suite using GitHub Actions.
You can also run the tests locally before submitting your pull request, using `pytest`.

1. Install the test dependencies (into the virtual environment where you installed PyGambit): ::

    pip install -r tests/requirements.txt

2. Navigate to the Gambit repository and run the tests: ::

    pytest

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

2. Install the docs dependencies (into the virtual environment where you installed PyGambit): ::

    pip install -r doc/requirements.txt

3. Navigate to the Gambit repo and build the docs: ::

    cd doc
    make html  # or make livehtml for live server with auto-rebuild

4. Open ``doc/_build/html/index.html`` in your browser to view the documentation.




Recognising contributions
-------------------------

Gambit is set up with `All Contributors <https://allcontributors.org/>`__ to recognise all types of contributions, including code, documentation, bug reports, and more.

You can see the list of contributors on the README page of the `Gambit GitHub repo <https://github.com/gambitproject/gambit>`__.

To add a contributor, comment on a GitHub Issue or Pull Request, asking @all-contributors to add a contributor:

 @all-contributors please add @<username> for <contributions>

Refer to the `emoji key <https://allcontributors.org/docs/en/emoji-key>`__ for a list of contribution types.
