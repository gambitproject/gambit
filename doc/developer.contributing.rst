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

Editing this documentation
--------------------------

1. If you haven't already, clone the Gambit repository from GitHub: ::

    git clone https://github.com/gambitproject/gambit.git
    cd gambit

2. Either install the docs requirements into your existing PyGambit development environment, or create a new virtual environment and install both the requirements and PyGambit there. For example, you can use `venv` to create a new environment: ::

    python -m venv docenv
    source docenv/bin/activate

3. Install the requirements and make the docs: ::

    pip install .
    cd doc
    pip install -r requirements.txt
    make html  # or make livehtml for live server with auto-rebuild

4. Open ``doc/_build/html/index.html`` in your browser to view the documentation.

5. Make any changes you want to the `.rst` files in the ``doc`` directory and rebuld the documentation to check your changes.

6. Follow the usual GitHub workflow to commit your changes and push them to the repository.

7. Core developers will review your changes and merge to the master branch, which automatically deploys the documentation via the ReadTheDocs service.

.. TODO: Add instructions for the GitHub workflow during contributor docs refactoring.
   See https://github.com/gambitproject/gambit/issues/541

