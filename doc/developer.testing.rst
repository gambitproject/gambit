.. _testing:

Tests and checks
================

The test suite
--------------

The `pytest <https://docs.pytest.org/en/stable/>`_ suite tests C++ through ``pygambit``.  There are
no separate C++ tests, so ``make check`` is a no-op.

* ``tests/test_*.py``: ``pygambit`` tests by area, such as ``test_nash.py`` for solvers and
  ``test_io.py`` for game-file I/O.
* ``tests/test_games``: test game files.  ``tests/games.py`` loads them and constructs games;
  tests also use the :ref:`catalog <repository-layout>`.
* ``tests/cli``: tests of the ``gambit-*`` command-line tools.
* ``tests/test_tutorials.py``: runs the tutorial notebooks in ``doc/tutorials``.
* ``build_support/releases/test_extract_changelog.py``: ``ChangeLog`` format tests.
* ``tests/test_catalog_*.py``: fetching catalog releases and commits.

``pyproject.toml``'s ``[tool.pytest.ini_options]`` configures pytest, including solver and
``slow`` markers.

After installing the ``test`` dependencies (see :ref:`build-python`), run from the repository
root::

    pytest --run-tutorials

Omit ``--run-tutorials`` to skip the slower notebooks.  They require both ``test`` and ``doc``
dependencies.

Running the tests with sanitizers
---------------------------------

The ``sanitizers.yml`` workflow builds ``pygambit`` with AddressSanitizer and
UndefinedBehaviorSanitizer, then runs the tests.  It runs weekly on the default branch and on
manual dispatch::

    gh workflow run sanitizers.yml --ref <branch>

To run the same test locally on Linux with GCC, from the repository root: ::

    SAN="-fsanitize=address,undefined -fno-sanitize-recover=undefined"
    SAN="$SAN -fno-omit-frame-pointer -g -O1 -D_GLIBCXX_ASSERTIONS"
    CXXFLAGS="$SAN" CFLAGS="$SAN" LDFLAGS="-fsanitize=address,undefined" pip install -v ".[test]"
    LD_PRELOAD=$(gcc -print-file-name=libasan.so) \
    ASAN_OPTIONS=detect_leaks=0:detect_stack_use_after_return=1 \
    UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
    PYTHONMALLOC=malloc \
    pytest --capture=sys

Preloading instruments the uninstrumented Python interpreter.  ``--capture=sys`` preserves ASan
reports when it aborts, while both ``CXXFLAGS`` and ``CFLAGS`` ensure all C++ files receive the
flags.

On macOS with Apple clang, omit ``-D_GLIBCXX_ASSERTIONS`` and preload with
``DYLD_INSERT_LIBRARIES`` instead of ``LD_PRELOAD``::

    DYLD_INSERT_LIBRARIES=$(clang -print-file-name=libclang_rt.asan_osx_dynamic.dylib) \
    ASAN_OPTIONS=detect_leaks=0 pytest --capture=sys

Because ``tests/cli/test_smoke.py`` subprocesses do not receive the ASan runtime on macOS, exclude
them with ``--ignore=tests/cli/test_smoke.py``.

.. _linting-tools:

Formatting and linting
----------------------

Configuration is in:

* ``.clang-format`` and ``.clang-tidy``, for the C++ sources.
* ``pyproject.toml``, for ``ruff`` and ``cython-lint`` (line length 99).
* ``.pre-commit-config.yaml``, which runs ``ruff``, ``cython-lint``, ``clang-format`` and some
  whitespace checks on each commit.

`pre-commit <https://pre-commit.com>`__ runs many pull-request checks locally.  Install it, then
enable it in the repository::

    pre-commit install

Two ``make`` targets run the version of ``clang-tidy`` pinned in
``.github/lint-requirements.txt``.  Run them from a directory configured as in
:ref:`cli-gui-from-source`::

    # every C++ source file, 8 at a time (the default is 1)
    make clang-tidy CLANG_TIDY_JOBS=8
    # only the lines changed since origin/master (BASE=... to change it)
    make clang-tidy-diff

``src/gui`` is checked when ``configure`` finds wxWidgets.  On macOS, set
``SDKROOT=$(xcrun --show-sdk-path)`` if standard library headers are missing.
``clang-tidy-diff`` checks changed ``.cc`` lines but not header-only changes; the full target checks
the pull request.

.. _ci-workflows:

Continuous integration
----------------------

The GitHub Actions workflows are in ``.github/workflows``:

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Workflow
     - What it does
   * - ``python.yml``
     - Builds ``pygambit`` and runs the test suite, including the tutorials, on Linux, macOS
       and Windows.  Runs on every push and pull request.
   * - ``tools.yml``
     - Builds the graphical interface with the autotools on Linux, macOS and Windows.
       Runs on every push and pull request.
   * - ``lint.yml``
     - ``clang-format``, ``clang-tidy``, ``ruff``, ``cython-lint``, ``mypy``, and the size
       limit on the Cython layer (see :ref:`pygambit-layers`).
       Runs on every push and pull request.
   * - ``wheel-smoke.yml``
     - Builds a ``pygambit`` wheel and checks that it installs and imports.
       Runs on every pull request.
   * - ``wheels.yml``
     - Builds the ``pygambit`` wheels and source distribution for publication.
       Runs on version tags, and monthly.
   * - ``osxbinary.yml``
     - Builds the macOS graphical interface binary.  Runs on version tags, and weekly.
   * - ``sanitizers.yml``
     - Runs the test suite with sanitizers (see above).  Runs weekly, or by manual dispatch.
   * - ``release.yml``
     - Creates a GitHub release, with notes from ``ChangeLog``, when a version tag is pushed.
