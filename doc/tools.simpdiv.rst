.. _gambit-simpdiv:

:program:`gambit-simpdiv`: Compute equilibria via simplicial subdivision
========================================================================

:ref:`Algorithm description <simpdiv>`

The algorithm begins with any mixed strategy profile consisting of
rational numbers as probabilities. Without any options, the algorithm
begins with the centroid, and computes one Nash equilibrium. To
attempt to compute other equilibria that may exist, use the
:option:`gambit-simpdiv -r` or :option:`gambit-simpdiv -s`
options to specify additional starting points for the algorithm.

.. program:: gambit-simpdiv

.. cmdoption:: -g

   Sets the granularity of the grid refinement. By
   default, when the grid is refined, the stepsize is cut in half, which
   corresponds to specifying `-g 2`. If this parameter is specified, the
   grid is refined at each step by a multiple of MULT .

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -n

   Randomly generate COUNT starting points. Only
   applicable if option :option:`gambit-simpdiv -r` is also specified.

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.

.. cmdoption:: -r

   Generate random starting points with denominator DENOM.
   Since this algorithm operates on a grid, by its nature the
   probabilities it works with are always rational numbers. If this
   parameter is specified, starting points for the procedure are
   generated randomly using the uniform distribution over strategy
   profiles with probabilities having denominator DENOM.

.. cmdoption:: -s

   Specifies a file containing a list of starting points
   for the algorithm. The format of the file is comma-separated values,
   one mixed strategy profile per line, in the same format used for
   output of equilibria (excluding the initial NE tag).

.. cmdoption:: -m

   .. versionadded:: 16.2.0

   Specify the maximum regret criterion for acceptance as an approximate Nash equilibrium
   (default is 1e-8).  See :ref:`pygambit-nash-maxregret` for interpretation and guidance.

.. cmdoption:: -d DECIMALS

   .. versionadded:: 16.2.0

   Simplicial subdivision operates on a triangulation grid in the set of mixed strategy profiles.
   Therefore, it produces output in which all probabilities are expressed as rational numbers, and
   by default the output reports these.  By specifying this option, instead probabilities are
   expressed as floating-point numbers with the specified number of decimal places.  Specifying
   this option sacrifices some precision in reporting the output of the method, in exchange for
   probabilities which are more human-readable.

.. cmdoption:: -v

   Sets verbose mode. In verbose mode, initial points, as well as
   the approximations computed at each grid refinement, are all output,
   in addition to the approximate equilibrium profile found.


Computing an equilibrium in mixed strategies of :download:`e02.efg
<../contrib/games/e02.efg>`, the example in Figure 2 of Selten
(International Journal of Game Theory, 1975)::

   $ gambit-simpdiv e02.nfg
   Compute Nash equilibria using simplicial subdivision
   Gambit version 16.4.0, Copyright (C) 1994-2025, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1,0,0,1,0
