.. _command-line:

Command-line tools
==================

Gambit provides command-line interfaces for each method for computing
Nash equilibria.  These are suitable for scripting or calling from
other programs.  This chapter describes the use of these programs.
For a general overview of methods for computing equilibria,
see the survey of [McKMcL96]_.

The graphical interface also provides a frontend for calling these
programs and evaluating their output.  Direct use of the command-line
programs is intended for advanced users and applications.

These programs take an extensive or strategic game file
on standard input, and output a list of equilibria computed. The
equilibria computed are presented as a list of comma-separated
probabilities, preceded by the tag "NE". Many of the programs
optionally output additional information about the operation of the
algorithm.  These outputs have other, program-specific tags, described
in the individual program documentation.


:program:`gambit-enumpure`: Enumerate pure-strategy equilibria of a game
------------------------------------------------------------------------

:program:`gambit-enumpure` reads a game on standard input and searches for
pure-strategy Nash equilibria.

.. versionchanged:: 14.0.2
   The effect of the `-S` switch is now purely cosmetic, determining
   how the equilibria computed are represented in the
   output. Previously, `-S` computed using the strategic game; if this
   was not specified for an extensive game, the agent form equilibria
   were returned.

.. program:: gambit-enumpure

.. cmdoption:: -S

   Report equilibria in reduced strategic form strategies, even if the
   game is an extensive game.  By default, if passed an extensive
   game, the output will be in behavior strategies.  Specifying this switch
   does not imply any change in operation internally, as pure-strategy
   equilibria are defined in terms of reduced strategic form
   strategies.

.. cmdoption:: -D

   .. versionadded:: 14.0.2

   The default output format for computed equilibria is a
   comma-separated list of strategy or action probabilities, suitable
   for postprocessing by automated tools.  Specifying `-D` instead
   causes the program to output greater detail on each equilbrium
   profile computed.

.. cmdoption:: -A

   .. versionadded:: 14.0.2

   Report agent form equilibria, that is, equilibria which consider
   only deviations at one information set.  Only has an effect for
   extensive games, as strategic games have only one information set
   per player.

.. cmdoption:: -P

   By default, the program computes all pure-strategy Nash
   equilibria in an extensive game. This switch instructs the program to
   find only pure-strategy Nash equilibria which are subgame perfect.
   (This has no effect for strategic games, since there are no proper
   subgames of a strategic game.)

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.


Computing the equilibria of an extensive game::

   $ gambit-enumpure < e02.efg
   Search for Nash equilibria in pure strategies
   Gambit version 14.0.2, Copyright (C) 1994-2014, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1,0,0,0,1,0

With the `-S` switch, the set of equilibria returned is the same,
except expressed in strategic game strategies rather than behavior
strategies::

   $ gambit-enumpure -S < e02.efg
   Search for Nash equilibria in pure strategies
   Gambit version 14.0.2, Copyright (C) 1994-2014, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1,0,0,1,0

The `-A` switch considers only behavior strategy profiles where there
is no way for a player to improve his payoff by changing action at
only one information set; therefore the set of solutions is larger::

   $ gambit-enumpure -A < e02.efg   
   Search for Nash equilibria in pure strategies
   Gambit version 14.0.2, Copyright (C) 1994-2014, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1,0,1,0,1,0
   NE,1,0,1,0,0,1
   NE,1,0,0,1,1,0



:program:`gambit-enumpoly`: Compute equilibria of a game using polynomial systems of equations
----------------------------------------------------------------------------------------------

:program:`gambit-enumpoly` reads a game on standard input and
computes Nash equilibria by solving systems of polynomial equations
and inequalities.

This program searches for all Nash equilibria in a strategic game
using a support enumeration approach. This approach computes all the
supports which could, in principle, be the support of a Nash
equilibrium, and then searches for a totally mixed equilibrium on that
support by solving a system of polynomial equalities and inequalities
formed by the Nash equilibrium conditions. The ordering of the
supports is done in such a way as to maximize use of previously
computed information, making it suited to the computation of all Nash
equilibria.

When the verbose switch `-v` is used, the program outputs each support
as it is considered. The supports are presented as a comma-separated
list of binary strings, where each entry represents one player. The
digit 1 represents a strategy which is present in the support, and the
digit 0 represents a strategy which is not present. Each candidate
support is printed with the label "candidate,".

Note that the subroutine to compute a solution to the system of
polynomial equations and inequalities will fail in degenerate cases.
When the verbose switch `-v` is used, these supports are identified on
standard output with the label "singular,". It is possible that there
exist equilibria, often a connected component of equilibria, on these
singular supports.


.. program:: gambit-enumpoly

.. cmdoption:: -d
   
   Express all output using decimal representations with the specified
   number of digits.

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -H

   By default, the program uses an enumeration method designed to
   visit as few supports as possible in searching for all equilibria.
   With this switch, the program uses a heuristic search method based on
   Porter, Nudelman, and Shoham [PNS04]_, which is designed to minimize the
   time until the first equilibrium is found. This switch only has an
   effect when solving strategic games.

.. cmdoption:: -S

   By default, the program uses behavior strategies for extensive
   games; this switch instructs the program to use reduced strategic game
   strategies for extensive games. (This has no effect for strategic
   games, since a strategic game is its own reduced strategic game.)

.. cmdoption:: -q 

   Suppresses printing of the banner at program launch.

.. cmdoption:: -v

   Sets verbose mode. In verbose mode, supports are printed on
   standard output with the label "candidate" as they are considered, and
   singular supports are identified with the label "singular." By
   default, no information about supports is printed.

Example invocation::

   $ gambit-enumpoly < e01.efg 
   Compute Nash equilibria by solving polynomial systems
   Gambit version 14.0.2, Copyright (C) 1994-2014, The Gambit Project
   Heuristic search implementation Copyright (C) 2006, Litao Wei 
   This is free software, distributed under the GNU GPL

   NE,0.000000,1.000000,0.333333,0.666667,1.000000,0.000000
   NE,1.000000,0.000000,1.000000,0.000000,0.250000,0.750000
   NE,1.000000,0.000000,1.000000,0.000000,0.000000,0.000000
   NE,0.000000,1.000000,0.000000,0.000000,1.000000,0.000000


.. _gambit-enummixed:

:program:`gambit-enummixed`: Enumerate equilibria in a two-player game
----------------------------------------------------------------------

:program:`gambit-enummixed` reads a two-player game on standard input and
computes Nash equilibria using extreme point enumeration.

In a two-player strategic game, the set of Nash equilibria can be expressed
as the union of convex sets.  This program generates all the extreme
points of those convex sets. (Mangasarian [Man64]_) 
This is a superset of the points generated by the path-following
procedure of Lemke and Howson (see :ref:`gambit-lcp`).  It was
shown by Shapley [Sha74]_ that there are equilibria not accessible via
the method in :ref:`gambit-lcp`, whereas the output of
:program:`gambit-enummixed` is guaranteed to return all the extreme
points. 

.. program:: gambit-enummixed

.. cmdoption:: -d

   By default, this program computes using exact
   rational arithmetic. Since the extreme points computed by this method
   are guaranteed to be rational when the payoffs in the game are
   rational, this permits exact computation of the equilibrium set.
   Computation using rational arithmetic is in general slow, however. For
   most games, acceptable results can be obtained by computing using the
   computer's native floating-point arithmetic. Using this flag enables
   computation in floating-point, and expresses all output using decimal
   representations with the specified number of digits.

.. cmdoption:: -D

   Since all Nash equilibria involve only strategies which survive
   iterative elimination of strictly dominated strategies, the program
   carries out the elimination automatically prior to computation.
   This is recommended, since it almost always results in superior
   performance.
   Specifying `-D` skips the elimination step and performs the
   enumeration on the full game.

.. cmdoption:: -c

   The program outputs the extreme equilibria as it finds them,
   prefixed by the tag NE . If this option is specified, once all extreme
   equilbria are identified, the program computes the convex sets which
   make up the set of equilibria. The program then additionally outputs
   each convex set, prefixed by convex-N , where N indexes the set. The
   set of all equilibria, then, is the union of these convex sets.

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.

.. cmdoption:: -L

   Use `lrslib <http://cgm.cs.mcgill.ca/~avis/C/lrs.html>`_ by David Avis
   to carry out the enumeration process.  This is an experimental
   feature that has not been widely tested.

Example invocation::

   $ gambit-enummixed < e02.nfg
   Compute Nash equilibria by enumerating extreme points
   Gambit version 14.0.2, Copyright (C) 1994-2014, The Gambit Project
   Enumeration code based on lrslib 4.2b, 
   Copyright (C) 1995-2005 by David Avis (avis@cs.mcgill.ca)
   This is free software, distributed under the GNU GPL

   NE,1,0,0,1,0
   NE,1,0,0,1/2,1/2

In fact, the game e02.nfg has a one-dimensional continuum of
equilibria.  This fact can be observed by examining the connectedness
information using the `-c` switch::
  
   $ gambit-enummixed -c < ../contrib/games/e02.nfg
   Compute Nash equilibria by enumerating extreme points
   Gambit version 14.0.2, Copyright (C) 1994-2014, The Gambit Project
   Enumeration code based on lrslib 4.2b, 
   Copyright (C) 1995-2005 by David Avis (avis@cs.mcgill.ca)
   This is free software, distributed under the GNU GPL

   NE,1,0,0,1,0
   NE,1,0,0,1/2,1/2
   convex-1,1,0,0,1/2,1/2
   convex-1,1,0,0,1,0


.. _gambit-gnm:

:program:`gambit-gnm`: Compute Nash equilibria in a strategic game using a global Newton method
-----------------------------------------------------------------------------------------------

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

Example invocation::

    $ gambit-gnm < e02.nfg
    Compute Nash equilibria using a global Newton method
    Gametracer version 0.2, Copyright (C) 2002, Ben Blum and Christian Shelton
    Gambit version 14.0.2, Copyright (C) 1994-2014, The Gambit Project
    This is free software, distributed under the GNU GPL

    NE,1,0,2.99905e-12,0.5,0.5


.. note:: 

   This is an experimental program and has not been extensively tested.

.. seealso::
  
   :ref:`gambit-ipa`.


.. _gambit-ipa:

:program:`gambit-ipa`: Compute Nash equilibria in a strategic game using iterated polymatrix approximation
----------------------------------------------------------------------------------------------------------

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


Example invocation::

   $ gambit-ipa < e02.nfg
   Compute Nash equilibria using iterated polymatrix approximation
   Gametracer version 0.2, Copyright (C) 2002, Ben Blum and Christian Shelton
   Gambit version 14.0.2, Copyright (C) 1994-2014, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1.000000,0.000000,0.000000,1.000000,0.000000



.. note:: 

   This is an experimental program and has not been extensively tested.

.. seealso::
  
   :ref:`gambit-gnm`.


.. _gambit-lcp:

:program:`gambit-lcp`: Compute equilibria in a two-player game via linear complementarity
-----------------------------------------------------------------------------------------


:program:`gambit-lcp` reads a two-player game on standard input and
computes Nash equilibria by finding solutions to a linear
complementarity problem. For extensive games, the program uses the
sequence form representation of the extensive game, as defined by
Koller, Megiddo, and von Stengel [KolMegSte94]_, and applies the
algorithm developed by Lemke. For strategic games, the program using
the method of Lemke and Howson [LemHow64]_.  There exist strategic
games for which some equilibria cannot be located by this method; see
Shapley [Sha74]_.

In a two-player strategic game, the set of Nash equilibria can be expressed
as the union of convex sets. This program will find extreme points
of those convex sets.  See :ref:`gambit-enummixed` for a method
which is guaranteed to find all the extreme points for a strategic
game.

.. program:: gambit-lcp

.. cmdoption:: -d

   By default, this program computes using exact
   rational arithmetic. Since the extreme points computed by this method
   are guaranteed to be rational when the payoffs in the game are
   rational, this permits exact computation of the equilibrium set.
   Computation using rational arithmetic is in general slow, however. For
   most games, acceptable results can be obtained by computing using the
   computer's native floating-point arithmetic. Using this flag enables
   computation in floating-point, and expresses all output using decimal
   representations with the specified number of digits.

.. cmdoption:: -S

   By default, the program uses behavior strategies for extensive
   games; this switch instructs the program to use reduced strategic game
   strategies for extensive games. (This has no effect for strategic
   games, since a strategic game is its own reduced strategic game.)

.. cmdoption:: -D

   .. versionadded:: 14.0.2

   The default output format for computed equilibria is a
   comma-separated list of strategy or action probabilities, suitable
   for postprocessing by automated tools.  Specifying `-D` instead
   causes the program to output greater detail on each equilbrium
   profile computed.

.. cmdoption:: -P

   By default, the program computes Nash equilibria in an extensive
   game. This switch instructs the program to find only equilibria
   which are subgame perfect.  (This has no effect for strategic
   games, since there are no proper subgames of a strategic game.)

.. cmdoption:: -h 

   Prints a help message listing the available options.

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.


Example invocation::

   $ gambit-lcp < e02.efg
   Compute Nash equilibria by solving a linear complementarity program
   Gambit version 14.0.2, Copyright (C) 1994-2014, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1,0,1/2,1/2,1/2,1/2



:program:`gambit-lp`: Compute equilibria in a two-player constant-sum game via linear programming
-------------------------------------------------------------------------------------------------

:program:`gambit-lp` reads a two-player constant-sum game on standard input
and computes a Nash equilibrium by solving a linear program. The
program uses the sequence form formulation of Koller, Megiddo, and von
Stengel [KolMegSte94]_ for extensive games.

While the set of equilibria in a two-player constant-sum strategic
game is convex, this method will only identify one of the extreme
points of that set.


.. program:: gambit-lp

.. cmdoption:: -d

   By default, this program computes using exact
   rational arithmetic. Since the extreme points computed by this method
   are guaranteed to be rational when the payoffs in the game are
   rational, this permits exact computation of an equilibrium.
   Computation using rational arithmetic is in general slow, however. For
   most games, acceptable results can be obtained by computing using the
   computer's native floating-point arithmetic. Using this flag enables
   computation in floating-point, and expresses all output using decimal
   representations with the specified number of digits.

.. cmdoption:: -S

   By default, the program uses behavior strategies for extensive
   games; this switch instructs the program to use reduced strategic game
   strategies for extensive games. (This has no effect for strategic
   games, since a strategic game is its own reduced strategic game.)

.. cmdoption:: -D

   .. versionadded:: 14.0.3

   The default output format for computed equilibria is a
   comma-separated list of strategy or action probabilities, suitable
   for postprocessing by automated tools.  Specifying `-D` instead
   causes the program to output greater detail on each equilbrium
   profile computed.

.. cmdoption:: -P

   By default, the program computes Nash equilibria in an extensive
   game. This switch instructs the program to find only equilibria
   which are subgame perfect.  (This has no effect for strategic
   games, since there are no proper subgames of a strategic game.)

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.

Example invocation::

   $ gambit-lp < 2x2const.nfg
   Compute Nash equilibria by solving a linear program
   Gambit version 14.0.2, Copyright (C) 1994-2014, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1/3,2/3,1/3,2/3


:program:`gambit-liap`: Compute Nash equilibria using function minimization
---------------------------------------------------------------------------

:program:`gambit-liap` reads a game on standard input and computes
approximate Nash equilibria using a function minimization approach.

This procedure searches for equilibria by generating random starting
points and using conjugate gradient descent to minimize the Lyapunov
function of the game. This function is a nonnegative function which is
zero exactly at strategy profiles which are Nash equilibria.

Note that this procedure is not globally convergent. That is, it is
not guaranteed to find all, or even any, Nash equilibria.


.. program:: gambit-liap

.. cmdoption:: -d
  
   Express all output using decimal representations with the
   specified number of digits.

.. cmdoption:: -n

   Specify the number of starting points to randomly generate.

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.

.. cmdoption:: -s

   Specifies a file containing a list of starting points
   for the algorithm. The format of the file is comma-separated values,
   one mixed strategy profile per line, in the same format used for
   output of equilibria (excluding the initial NE tag).

.. cmdoption:: -S

   By default, the program uses behavior strategies for extensive
   games; this switch instructs the program to use reduced strategic game
   strategies for extensive games. (This has no effect for strategic
   games, since a strategic game is its own reduced strategic game.)

.. cmdoption:: -v

   Sets verbose mode. In verbose mode, initial points, as well as
   points at which the minimization fails at a constrained local minimum
   that is not a Nash equilibrium, are all output, in addition to any
   equilibria found.

Example invocation::

   $ gambit-liap < ../contrib/games/e02.nfg
   Compute Nash equilibria by minimizing the Lyapunov function
   Gambit version 14.0.2, Copyright (C) 1994-2014, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE, 0.998701, 0.000229, 0.001070, 0.618833, 0.381167




:program:`gambit-simpdiv`: Compute equilibria via simplicial subdivision
------------------------------------------------------------------------

:program:`gambit-simpdiv` reads a game on standard input and computes
approximations to Nash equilibria using a simplicial subdivision
approach.

This program implements the algorithm of van der Laan, Talman, and van
Der Heyden [VTH87]_. The algorithm proceeds by constructing a triangulated grid
over the space of mixed strategy profiles, and uses a path-following
method to compute an approximate fixed point. This approximate fixed
point can then be used as a starting point on a refinement of the
grid. The program continues this process with finer and finer grids
until locating a mixed strategy profile at which the maximum regret is
small.

The algorithm begins with any mixed strategy profile consisting of
rational numbers as probabilities. Without any options, the algorithm
begins with the centroid, and computes one Nash equilibrium. To
attempt to compute other equilibria that may exist, use the
:option:`gambit-simpdiv -r` or :option:`gambit-simpdiv -s`
options to specify additional starting points for the algorithm.

.. program:: gambit-simpdiv

.. cmdoption:: -d

   Express all output using decimal representations
   with DECIMALS digits. Because the algorithm is based on a grid, this
   algorithm inherently computes using rational numbers. By default,
   these rational probabilities are output. Expressing the profile
   probabilities as decimal numbers may make reading the output easier.

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

.. cmdoption:: -v

   Sets verbose mode. In verbose mode, initial points, as well as
   the approximations computed at each grid refinement, are all output,
   in addition to the approximate equilibrium profile found.


Example invocation::

   $ gambit-simpdiv < e02.nfg 
   Compute Nash equilibria using simplicial subdivision
   Gambit version 14.0.2, Copyright (C) 1994-2014, The Gambit Project
   This is free software, distributed under the GNU GPL

   NE,1,0,0,1,0


:program:`gambit-logit`: Compute quantal response equilbria
-----------------------------------------------------------

:program:`gambit-logit` reads a game on standard input and computes the
principal branch of the (logit) quantal response correspondence.

The method is based on the procedure described in Turocy [Tur05]_ for 
strategic games and Turocy [Tur10]_ for extensive games.
It uses standard path-following methods (as
described in Allgower and Georg's "Numerical Continuation Methods") to
adaptively trace the principal branch of the correspondence
efficiently and securely.

The method used is a predictor-corrector method, which first generates
a prediction using the differential equations describing the branch of
the correspondence, followed by a corrector step which refines the
prediction using Newton's method for finding a zero of a function. Two
parameters control the operation of this tracing. The option `-s` sets
the initial step size for the predictor phase of the tracing. This
step size is then dynamically adjusted based on the rate of
convergence of Newton's method in the corrector step. If the
convergence is fast, the step size is adjusted upward (accelerated);
if it is slow, the step size is decreased (decelerated). The option
`-a` sets the maximum acceleration (or deceleration). As described in
Turocy [Tur05]_, this acceleration helps to
efficiently trace the correspondence when it reaches its asymptotic
phase for large values of the precision parameter lambda.

.. program:: gambit-logit

.. cmdoption:: -d

   Express all output using decimal representations with the specified
   number of digits.  The default is `-d 6`.

.. cmdoption:: -s

   Sets the initial step size for the predictor phase of
   the tracing procedure. The default value is .03. The step size is
   specified in terms of the arclength along the branch of the
   correspondence, and not the size of the step measured in terms of
   lambda. So, for example, if the step size is currently .03, but the
   position of the strategy profile on the branch is changing rapidly
   with lambda, then lambda will change by much less then .03 between
   points reported by the program.

.. cmdoption:: -a

   Sets the maximum acceleration of the step size during
   the tracing procedure. This is interpreted as a multiplier. The
   default is 1.1, which means the step size is increased or decreased by
   no more than ten percent of its current value at every step. A value
   close to one would keep the step size (almost) constant at every step.

.. cmdoption:: -m

   Stop when reaching the specified value of the
   parameter lambda. By default, the tracing stops when lambda reaches
   1,000,000, which is usually suitable for computing a good
   approximation to a Nash equilibrium. For applications, such as to
   laboratory experiments, where the behavior of the correspondence for
   small values of lambda is of interest and the asymptotic behavior is
   not relevant, setting MAXLAMBDA to a much smaller value may be
   indicated.

.. cmdoption:: -l

   While tracing, compute the logit equilibrium points
   with parameter LAMBDA accurately.

.. cmdoption:: -S

   By default, the program uses behavior strategies for extensive
   games; this switch instructs the program to use reduced strategic game
   strategies for extensive games. (This has no effect for strategic
   games, since a strategic game is its own reduced strategic game.)

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -e 

   By default, all points computed are output by the program. If
   this switch is specified, only the approximation to the Nash
   equilibrium at the end of the branch is output.

Example invocation::

   $ gambit-logit < e02.nfg
   Compute a branch of the logit equilibrium correspondence
   Gambit version 14.0.2, Copyright (C) 1994-2014, The Gambit Project
   This is free software, distributed under the GNU GPL

   0.000000,0.333333,0.333333,0.333333,0.5,0.5
   0.022853,0.335873,0.328284,0.335843,0.501962,0.498038
   0.047978,0.338668,0.322803,0.33853,0.504249,0.495751
   0.075600,0.341747,0.316863,0.34139,0.506915,0.493085
   0.105965,0.345145,0.310443,0.344413,0.510023,0.489977
   0.139346,0.348902,0.303519,0.347578,0.51364,0.48636
  
   ...

   735614.794714,1,0,4.40659e-11,0.500016,0.499984
   809176.283787,1,0,3.66976e-11,0.500015,0.499985
   890093.921767,1,0,3.05596e-11,0.500014,0.499986
   979103.323545,1,0,2.54469e-11,0.500012,0.499988
   1077013.665501,1,0,2.11883e-11,0.500011,0.499989
