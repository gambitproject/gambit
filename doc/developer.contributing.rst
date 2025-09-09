Contributing to the Gambit Project
==================================

This section provides guidelines for contributing to the Gambit project, including how to report bugs, suggest features, and contribute code.
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

1. To get started contributing code in the `Gambit GitHub repo <https://github.com/gambitproject/gambit>`__, do one of the following:

- Core developers: request contributor access from one of the `team <https://www.gambit-project.org/team/>`__
- External contributors: fork the repository on GitHub.

2. Clone the repository to your local machine ::

    git clone https://github.com/gambitproject/gambit.git  # or your fork URL
    cd gambit

3. Create a new branch for your changes ::

    git checkout -b feature/your-feature-name

4. Make your changes. Commit each change with a clear commit message ::

    git add .
    git commit -m "Add feature X or fix bug Y"

5. Push your changes to your fork or branch ::

    git push origin feature/your-feature-name

6. Open a pull request on GitHub to the master branch of the upstream repository, describing your changes and linking to any relevant issues.
7. Core developers will review your changes, provide feedback, and merge them into the master branch if they meet the project's standards.

Editing this documentation
--------------------------

1. If you haven't already, clone the Gambit repository from GitHub: ::

    git clone https://github.com/gambitproject/gambit.git
    cd gambit

2. Either install the docs requirements into your existing PyGambit development environment, or create a new virtual environment and install both the requirements and PyGambit there. For example, you can use `venv` to create a new environment: ::

    python -m venv docenv
    source docenv/bin/activate

3. `Install Pandoc <https://pandoc.org/installing.html>`_ for your OS

4. Install the requirements and make the docs: ::

    pip install .
    cd doc
    pip install -r requirements.txt
    make html  # or make livehtml for live server with auto-rebuild

5. Open ``doc/_build/html/index.html`` in your browser to view the documentation.

6. Make any changes you want to the `.rst` files in the ``doc`` directory and rebuild the documentation to check your changes.

7. Follow the usual GitHub workflow (see :ref:`contributing-code` above) to commit your changes and push them to the repository.

8. Core developers will review your changes and merge to the master branch, which automatically deploys the documentation via the ReadTheDocs service.


