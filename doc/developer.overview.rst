.. _architecture-overview:

Overview of Gambit architecture
===============================

The core of Gambit is a library of C++ code for the representation and analysis of finite games.
This code is organised into three directories:

* ``src/core``: Generic functions, structures, and utility code which is not specific to
  game theory.
* ``src/games``: Definition of the interface and representation data structures for game objects
  and related concepts.
* ``src/solvers``: Implementation of algorithms for computation on games, principally for the
  computation of (exact or approximate) Nash equilibria.

Gambit is written to the C++20 standard.  Because Gambit is cross-platform, C++ library
dependencies are avoided to facilitate development and packaging.

The principal way in which code is written to use Gambit is via the Python package
``pygambit``.  In particular, testing is done via the Python test suite and ``pytest``,
which therefore (indirectly) tests the C++ data structures and algorithms.
Running ``make check`` has no effect (there are no tests defined in that build system).
The test suite makes use of a variety of games, which are drawn from examples in
the game theory literature and textbooks, as well as examples constructed specifically
to test edge cases.  From Gambit 17, these are all curated as part of Gambit's
:ref:`catalog of games <catalog>`.

From Gambit 17, ``pygambit`` is also responsible for providing command-line interfaces to
equilibrium computation and other facilities; previously the command-line wrappers were
implemented in C++.

Gambit's core is also incorporated into a graphical user interface, also written
in C++ using wxWidgets (see ``src/gui``).  This is built by ``make`` if an installation
of a suitable version of wxWidgets is detected.

This hybrid architecture aims to maintain some flexibility for future development as follows:

1.  Although in practice we expect ``pygambit`` will be the most common path for writing
    code using Gambit, the C++ implementations are kept separate from the Python wrapper
    so they can continue to be used directly in C++
2.  Likewise, the graphical interface layer is kept cleanly separate from the core.
    Graphical-based interfaces (different libraries, languages,
    browser-based interfaces) can be developed without touching the core code.

Header organization
--------------------

Each of the three core directories is exposed through a top-level umbrella header:

* ``src/games.h`` covers game representation, equilibrium profile types, and file I/O
  (that is, all of ``src/core`` and ``src/games``).
* ``src/solvers.h`` includes ``games.h`` plus the single public header for every solver
  algorithm (for example ``src/solvers/lcp/lcp.h``).
* ``src/gambit.h`` is the "everything" umbrella: ``games.h`` followed by ``solvers.h``.

Solver headers depend on the representation types, never the reverse, so
``solvers.h`` sits above ``games.h`` rather than being folded into it.

Within ``src/core``, ``src/games``, and ``src/solvers``, every header should be
self-contained: it should ``#include`` whatever it directly uses, rather than relying
on some other header -- in particular, one of the umbrella headers -- having already
brought a dependency into scope by the time it is reached. A header that implements
algorithm internals (for example the linear algebra in ``src/solvers/linalg`` or the
polynomial-system machinery in ``src/solvers/enumpoly``) should depend only on
``src/core``, not on the game representation, unless it genuinely needs it.

This discipline will allow ``src/core``, ``src/games``, and ``src/solvers``
to be built and installed as a standalone C++ library, should we decide to offer
that in future.
