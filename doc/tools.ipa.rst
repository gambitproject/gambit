.. _gambit-ipa:

:program:`gambit-ipa`: Compute Nash equilibria in a strategic game using iterated polymatrix approximation
==========================================================================================================

:program:`gambit-ipa` reads a game on standard input and computes Nash
equilibria using an iterated polymatrix approximation approach
developed by Govindan and Wilson [GovWil04]_.
This program is based on the
`Gametracer 0.2 <http://dags.stanford.edu/Games/gametracer.html>`_
implementation by Ben Blum and Christian Shelton.

The algorithm takes as a parameter a mixed strategy profile.  This profile is
interpreted as defining a ray in the space of games.  The profile must have
the property that, for each player, the most frequently played strategy must
be unique.

The algorithm finds at most one equilibrium starting from any given profile.
Multiple starting profiles may be generated via the `-n` option or specified
via the `-s` option; different starting profiles may result in different
equilibria being found.


.. program:: gambit-ipa

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


Computing an equilibrium of :download:`e02.nfg <../contrib/games/e02.nfg>`,
the reduced strategic form of the example in Figure 2 of Selten
(International Journal of Game Theory, 1975)::

   $ gambit-ipa e02.nfg
   Compute Nash equilibria using iterated polymatrix approximation
   Gametracer version 0.2, Copyright (C) 2002, Ben Blum and Christian Shelton
   Gambit version 16.3.2, Copyright (C) 1994-2025, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1.000000,0.000000,0.000000,1.000000,0.000000



.. seealso::

   :ref:`gambit-gnm`.
