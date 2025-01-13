.. _gambit-gnm:

:program:`gambit-gnm`: Compute Nash equilibria in a strategic game using a global Newton method
===============================================================================================

:program:`gambit-gnm` reads a game on standard input and computes Nash
equilibria using a global Newton method approach developed by Govindan
and Wilson [GovWil03]_. This program is based on the
`Gametracer 0.2 <http://dags.stanford.edu/Games/gametracer.html>`_
implementation by Ben Blum and Christian Shelton.

The algorithm takes as a parameter a mixed strategy profile.  This profile is
interpreted as defining a ray in the space of games.  The profile must have
the property that, for each player, the most frequently played strategy must
be unique.

The algorithm finds a subset of equilibria starting from any given profile.
Multiple starting profiles may be generated via the `-n` option or specified
via the `-s` option; different starting profiles may result in different
subsets of equilibria being found.


.. program:: gambit-gnm

.. cmdoption:: -d

   Express all output using decimal representations
   with the specified number of digits.

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -n

   Randomly generate the specified number of perturbation vectors.

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.

.. cmdoption:: -s

   Specifies a file containing a list of starting points
   for the algorithm. The format of the file is comma-separated values,
   one mixed strategy profile per line, in the same format used for
   output of equilibria (excluding the initial NE tag).

.. cmdoption:: -m LAMBDA

    .. versionadded:: 16.2.0

   Specifies the value of lambda at which to assume no more equilibria
   are accessible via the specified ray, and terminate tracing.
   Must be a negative number; default is -10.

.. cmdoption:: -f FREQ

    .. versionadded:: 16.2.0

   Specifies the frequency to run a local Newton method step.  This is
   a correction step that reduces accumulated errors in the path-following.
   Default is 3.

.. cmdoption:: -i MAXITS

    .. versionadded:: 16.2.0

   Specifies the maximum number of iterations in a local Newton method step.
   Default is 10.

.. cmdoption:: -c STEPS

    .. versionadded:: 16.2.0

   Specifies the number of steps to take within a support cell.  Larger values
   trade off speed for security in tracing the path.  Default is 100.

.. cmdoption:: -v

   Show intermediate output of the algorithm.  If this option is
   not specified, only the equilibria found are reported.

Computing an equilibrium of :download:`e02.nfg <../contrib/games/e02.nfg>`,
the reduced strategic form of the example in Figure 2 of Selten
(International Journal of Game Theory, 1975)::

    $ gambit-gnm e02.nfg
    Compute Nash equilibria using a global Newton method
    Gametracer version 0.2, Copyright (C) 2002, Ben Blum and Christian Shelton
    Gambit version 16.3.0, Copyright (C) 1994-2025, The Gambit Project
    This is free software, distributed under the GNU GPL

    NE,1,0,2.99905e-12,0.5,0.5


.. seealso::

   :ref:`gambit-ipa`.
