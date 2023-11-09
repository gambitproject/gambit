.. _gambit-ipa:

:program:`gambit-ipa`: Compute Nash equilibria in a strategic game using iterated polymatrix approximation
==========================================================================================================

:program:`gambit-ipa` reads a game on standard input and computes Nash
equilibria using an iterated polymatrix approximation approach
developed by Govindan and Wilson [GovWil04]_.
This program is a wrapper around the
`Gametracer 0.2 <http://dags.stanford.edu/Games/gametracer.html>`_
implementation by Ben Blum and Christian Shelton.

.. program:: gambit-ipa

.. cmdoption:: -d

   Express all output using decimal representations
   with the specified number of digits.

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.


Computing an equilibrium of :download:`e02.nfg <../contrib/games/e02.nfg>`,
the reduced strategic form of the example in Figure 2 of Selten
(International Journal of Game Theory, 1975)::

   $ gambit-ipa e02.nfg
   Compute Nash equilibria using iterated polymatrix approximation
   Gametracer version 0.2, Copyright (C) 2002, Ben Blum and Christian Shelton
   Gambit version 16.1.0, Copyright (C) 1994-2023, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1.000000,0.000000,0.000000,1.000000,0.000000



.. note::

   This is an experimental program and has not been extensively tested.

.. seealso::

   :ref:`gambit-gnm`.
