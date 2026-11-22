.. _architecture-overview:

Code organisation
=================

.. _repository-layout:

Repository layout
-----------------

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Location
     - Contents
   * - ``src/core``, ``src/games``, ``src/solvers``
     - The C++ library: utility code, game representations, and solvers
       (see :ref:`cpp-library`).
   * - ``src/gambit.h``, ``src/games.h``, ``src/solvers.h``
     - Umbrella headers for the C++ library (see :ref:`header-organization`).
   * - ``src/pygambit``
     - The ``pygambit`` Python package: the Cython extension module (``gambit.pyx`` and
       ``*.pxi``) and the pure-Python modules (see :ref:`pygambit-layers`).
   * - ``src/pygambit/cli``
     - The ``gambit-*`` command-line tools, installed as entry points of ``pygambit``.
   * - ``src/gui``
     - The graphical interface, written in C++ using wxWidgets.
   * - ``tests``
     - The ``pytest`` test suite (see :ref:`testing`).
   * - ``catalog``
     - The catalog of games: the game files in ``catalog/games``, which are packaged into
       ``pygambit`` as ``pygambit.catalog``, the scripts that index them, and the catalog's
       own documentation in ``catalog/doc``.
   * - ``doc``
     - This documentation (see :ref:`editing-docs`).
   * - ``contrib``
     - Example game files (``contrib/games``, a few of which are used by the tests), and
       standalone experimental programs (``contrib/src``) which are not part of either build.
   * - ``pyproject.toml``, ``setup.py``, ``MANIFEST.in``
     - The build of ``pygambit``, and the configuration of ``pytest``, ``ruff`` and
       ``cython-lint``.
   * - ``configure.ac``, ``Makefile.am``, ``acinclude.m4``
     - The autotools build of the graphical interface.
   * - ``build_support``
     - ``GAMBIT_VERSION``, the single source of the version number; packaging files for
       macOS (``osx``) and Windows (``msw``); release scripts (``releases``); and the
       Cython size check (``cython_lines.py``).
   * - ``.github``
     - Continuous-integration workflows (``.github/workflows``, see :ref:`ci-workflows`),
       and issue and pull request templates.
   * - ``.pre-commit-config.yaml``, ``.clang-format``, ``.clang-tidy``
     - Formatting and linting configuration (see :ref:`linting-tools`).
   * - ``.readthedocs.yaml``
     - The ReadTheDocs build of this documentation.
   * - ``.devcontainer``
     - A development container definition (Ubuntu, with Python and the autotools).
   * - ``ChangeLog``
     - Release notes, in `Keep a Changelog <https://keepachangelog.com>`__ format.

The C++ sources have two independent builds:

* ``setuptools`` (``pyproject.toml`` and ``setup.py``) compiles ``src/core``, ``src/games`` and
  ``src/solvers`` into the ``pygambit`` extension module.
* The graphical interface is built by the autotools (``configure.ac`` and ``Makefile.am``).

See :ref:`building-from-source` for both.

.. _cpp-library:

The C++ library
---------------

Gambit's C++ library represents and analyses finite games.  It has three directories:

* ``src/core``: generic utilities not specific to game theory.
* ``src/games``: interfaces and data structures for games and related concepts.
* ``src/solvers``: game algorithms, principally exact and approximate Nash equilibrium
  computation.  Each algorithm has a subdirectory, such as ``src/solvers/lcp``.

Gambit uses C++20 and avoids C++ library dependencies to simplify cross-platform packaging.

Most clients use the ``pygambit`` Python package.  Its ``pytest`` suite also tests the underlying
C++ data structures and algorithms; the autotools build defines no ``make check`` tests.

``pygambit`` also provides the command-line tools, which were implemented in C++ before Gambit 17.

The C++ wxWidgets interface in ``src/gui`` also uses the core and is built by ``make`` when a
supported wxWidgets installation is found.

The C++ implementations remain separate from the Python wrapper so they can be used directly.
The graphical interface is likewise separate, allowing other interfaces to use the same core.

.. _header-organization:

Header organization
--------------------

Three top-level umbrella headers expose the library:

* ``src/games.h`` covers ``src/core`` and ``src/games``: game representations, equilibrium
  profiles, and file I/O.
* ``src/solvers.h`` includes ``games.h`` and each solver's public header, such as
  ``src/solvers/lcp/lcp.h``.
* ``src/gambit.h`` includes both ``games.h`` and ``solvers.h``.

Solver headers depend on representation types, never the reverse; ``solvers.h`` therefore sits
above ``games.h``.

Every header in these directories should be self-contained and include its direct dependencies
rather than rely on an umbrella header.  Algorithm internals, such as ``src/solvers/linalg`` and
``src/solvers/enumpoly``, should depend only on ``src/core`` unless they need game
representations.

This separation permits a future standalone C++ library build.

.. _pygambit-layers:

Layers of pygambit
------------------

``pygambit`` has three layers:

* C++ (``src/core``, ``src/games``, ``src/solvers``): game semantics and algorithms.
* Cython (``src/pygambit/*.pyx`` and ``*.pxi``): converting values and handles between Python and
  C++.
* Python (``src/pygambit/**/*.py``): validation, label resolution, error messages,
  value types and presentation.

Code belongs in the lowest suitable layer.  Python validates and resolves labels, then Cython
passes values and handles to C++ without further decisions.

CI limits the Cython layer's size.  ``build_support/cython_lines.py`` counts its source lines,
excluding comments, blanks, and docstrings; the ``cython-lines`` job in
``.github/workflows/lint.yml`` enforces its ``--max`` value.  Lower the limit when code moves to
Python, and raise it only for bindings to new C++ functionality.
