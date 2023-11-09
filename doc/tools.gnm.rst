.. _gambit-gnm:

:program:`gambit-gnm`: Compute Nash equilibria in a strategic game using a global Newton method
===============================================================================================

:program:`gambit-gnm` reads a game on standard input and computes Nash
equilibria using a global Newton method approach developed by Govindan
and Wilson [GovWil03]_. This program is a wrapper around the
`Gametracer 0.2 <http://dags.stanford.edu/Games/gametracer.html>`_
implementation by Ben Blum and Christian Shelton.

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

.. cmdoption:: -v

   Show intermediate output of the algorithm.  If this option is
   not specified, only the equilibria found are reported.

Computing an equilibrium of :download:`e02.nfg <../contrib/games/e02.nfg>`,
the reduced strategic form of the example in Figure 2 of Selten
(International Journal of Game Theory, 1975)::

    $ gambit-gnm e02.nfg
    Compute Nash equilibria using a global Newton method
    Gametracer version 0.2, Copyright (C) 2002, Ben Blum and Christian Shelton
    Gambit version 16.1.0, Copyright (C) 1994-2023, The Gambit Project
    This is free software, distributed under the GNU GPL

    NE,1,0,2.99905e-12,0.5,0.5


.. note::

   This is an experimental program and has not been extensively tested.

.. seealso::

   :ref:`gambit-ipa`.
