.. _command-line:

***
CLI
***

Gambit provides command-line interfaces for each method for computing
Nash equilibria.  These are suitable for scripting or calling from
other programs.  This chapter describes the use of these programs.
For a general overview of methods for computing equilibria,
see the survey of :cite:p:`McKMcL96`.

.. versionchanged:: 17.0.0

   These programs are now implemented in Python, as part of the
   :ref:`pygambit <pygambit>` package, rather than being separate
   C++ executables.  Installing PyGambit (``pip install pygambit``,
   see :ref:`install`) installs these command-line programs alongside
   it; no separate download or build is needed.  Each program's
   command-line interface, options, and output are unchanged from
   the previous C++ versions, except as otherwise noted.

These programs take an extensive or strategic game file, which can be
specified on the command line or piped via standard input, and output
a list of equilibria computed. The default output format is to present
equilibria computed as a list of comma-separated probabilities,
preceded by the tag `NE`. For mixed strategy profiles, the
probabilities are sorted lexicographically by player, then by
strategy.  For behavior strategy profiles, the probabilities are sorted
by player, then information set, then action number, where the
information sets for a player are sorted by the order in which they
are encountered in a depth-first traversal of the game tree.
Many programs take an option `-D`, which, if specified, instead prints
a more verbose, human-friendly description of each strategy profile
computed.

Many of the programs optionally output additional information
about the operation of the algorithm.  These outputs have other,
program-specific tags, described in the individual program
documentation.


.. toctree::
   :maxdepth: 2

   tools.enumpure
   tools.enummixed
   tools.enumpoly
   tools.lcp
   tools.lp
   tools.liap
   tools.simpdiv
   tools.logit
   tools.gnm
   tools.ipa
