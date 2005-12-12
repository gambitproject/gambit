GameTracer README
Copyright 2002, Ben Blum, Christian Shelton

CONTENTS
1. Description of algorithms
	1a. GNM
	1b. IPA
2. Installation
3. Inclusion in other applications
4. Instructions for use of GameTracer
	4a. Input format
	4b. Output format
5. Acknowledgements



1. DESCRIPTION OF ALGORITHMS

The GameTracer package includes two algorithms for solving normal form
games, both due to Srihari Govindan and Robert Wilson.  The first
algorithm, a global Newton method, is referred to henceforth as GNM.
The second is an iterative polymatrix approximation algorithm, and
will be referred to as IPA.

1A. GNM

GNM works by considering a ray of perturbed games with its origin at
the target game.  The perturbation takes the form of a vector of
payoff bonuses for each player's pure strategies.  Far out along this
ray, the bonuses are large enough to dominate the structure of the
game, so that the perturbed game is easily soluble.  The algorithm
then traces the equilibrium back along the ray to an equilibrium of
the original game.  The path may pass back and forth along the point
where the perturbation is zero, so many equilibria may be returned
while tracing a single trajectory.  The equilibria returned by GNM are
exact, with an error of approximately 1e-14.

1B. IPA

IPA approximates the game at each point with a bimatrix game , one in
which the interaction between each pair of players is independent from
all other players; thus, a player's payoff under a strategy profile is
the sum of his payoffs due to his individual interactions with each
other player.  Such games can be solved quickly using a variant of the
Lemke Howson algorithm.  IPA solves a succession of such games, one
for each step of the algorithm.  It returns approximate equilibria,
with an adjustable error threshold.  The algorithm only finds one
equilibrium in a single execution.  However, it is much faster than
GNM.  IPA can be used as a quick start for the GNM algorithm, although
this functionality is not implemented in GameTracer.


2. INSTALLATION

After the source files have been unpacked into a directory, GameTracer
can be compiled simply by issuing the make command with the
appropriate arguments.  GameTracer has been tested with both Linux and
Solaris systems.  The system type must be specified by setting the
SYSNAME compilation variable to either Linux or Solaris, using the
command line argument SYSNAME=<sysname>, where <sysname> is either
LINUX or SOLARIS (LINUX is the default if none is specified).  The
GameTracer executable, gt, can be compiled with the following command
(for a Linux system):

make SYSNAME=LINUX


3. INCLUSION IN OTHER APPLICATIONS

The gt executable included in the GameTracer package is rather
limited; you may wish to use the GNM or IPA algorithms in more general
settings.  If so, you will need to include gnm.h or ipa.h in your
source file.  The relevant function prototypes, and a description of
the meaning of each of their input variables, can be found in the
header files.

4. INSTRUCTIONS FOR USE OF GAMETRACER

The executable file for GameTracer is named gt, and is compiled into
the same directory as the source code.  The command line usage
instructions for gt are printed out when the function runs with no
arguments.  These instructions are as follows:

GameTracer 0.1
usage: gt [-i] (file|-r players actions gameseed) rayseed

-i:      use IPA (iterative polymatrix approximation)
file:    read game in from file
-r:      generate a game with the specified number of players and
         actions per player, with payoffs chosen randomly from [0,1]
rayseed: random seed for the perturbation ray


4a. INPUT FORMAT

gt runs the the gnm algorithm (or the ipa algorithm, if the -i flag is
issued) on a game and returns the results.  The IPA algorithm executes
more quickly, but only returns a single approximate equilibrium.  The
GNM algorithm, which is the default, executes more slowly but returns
multiple exact equilibria.  The target game can be specified in two
different ways: first, it can be read in from a file, and second, it
can be generated randomly.  If read in from a file, the command-line
syntax is, for example,

gt input-file 23

Here 23 is the seed used to randomize the perturbation ray in the
algorithm.  Different choices of seed will determine different search
trajectories through the space of equilibria of perturbed games, and
may return different equilibria.  Additionally, some rays will cause
the algorithm to terminate much faster than others.

The input file specification is as follows:

<n>
<p1> <p2> ... <pn>
<payoff0(0,0,...,0)> <payoff0(1,0,...,0)> ... <payoff0(p1,0,...,0>
<payoff0(0,1,0,...,0)> <payoff0(1,1,0,...,0)> ... <payoff0(p1,1,0,...,0>
.
.
.
<payoff0(0,p2,0,...,0)> <payoff0(1,p2,0,...,0)> ... <payoff0(p1,p2,0,...,0)>
.
.
.
<payoffn(p1,p2,...,pn)>

Here <n> is the number of players, and <pi> is the number of available
actions for player i to take.  Payoffi(a1,a2,...,an) is the payoff to
player i when each player j takes action aj.

Several example game files have been included:
symm2.gam         the symmetric game with 2 players
symm3.gam         the symmetric game with 3 players
rps3.gam          the three-player rock-paper-scissors game
rpsw.gam          a weighted two-player rock-paper-scissors game
bos.gam           the two-player Bach or Stravinsky game

A game can be generated randomly by issuing the -r command-line
argument.  In this case, the command-line syntax is, for example,

gt -r 3 4 4522 8712

This generates a random game with three players, each of which has
four possible actions.  The seed used to randomize the payoff matrix
is 4522, and the seed used to randomize the perturbation ray is 8712.
The command

gt -r 3 4 4522 1065

for example, attempts to solve the same game using a different
trajectory.

If gt is executed with incorrect parameters, it will exit with an
error.

There are a number of other input parameters to the GNM and IPA
algorithms.  The values of these parameters have been defined as
constants in the gt.cc source file.  The meaning of each parameter is
discussed briefly in the appropriate header file, gnm.h or ipa.h; for
a more in-depth discussion, refer to the papers mentioned in the
Acknowledgements section of this document.  The constant values of
these parameters are reasonable ones that should work for a wide
variety of games, but you may wish to change them.  For example, if
GNM gets too far off the correct trajectory (this error threshold is a
parameter), it will halt without finding any equilibria.  This might
be avoided by increasing the number of steps taken within a support
cell; this amounts to tracing the trajectory in smaller, more careful
increments.  If you do change the values of these parameters, be sure
to recompile.


4b. OUTPUT FORMAT

If the GNM algorithm is executed, by omitting the -i flag, the output
of gt is a list of row vectors, separated by empty lines,
representing equilibria of the game.  If the -i flag is issued, the
IPA algorithm will execute, and only one equilibrium will be returned.
A vector consists of player1's mixed strategy, followed by player 2's
mixed strategy, and so forth.  Suppose, for example, that we are
looking at a game with three players, each of which has two actions,
and that the strategy profile in which player1 chooses his first
action deterministically, player 2 chooses his second action
deterministically, and player3 mixes equally between his two actions,
is an equilibrium.  Then the output of gt will be:

1 0 0 1 0.5 0.5

Occasionally, especially on large games, gt will fail to produce an
output or will loop indefinitely.  There are several ways to avoid
this issue.  One is to turn on wobbles, by setting the constant
WOBBLE in gt.cc equal to 1.  This can cause GNM to loop indefinitely,
but can help keep it from straying off the correct path.  Another is
to increase the STEPS constant in the gt.cc source file, or to
increase the LNMFREQ constant, the frequency of LNM use (LNM stands
for Local Newton Method, and is a method for reducing accumulated
errors while tracing a path).  These constants are defined at the top
of gt.cc.  You will need to recompile after changing these constants.


5. ACKNOWLEDGEMENTS

Daphne Koller was the advisor for this project.

This work was funded under Air Force contract F30602-00-2-0598 under
DARPA's TASK program and ONR contract N00014-00-1-0637 under the MURI
program "Decision Making under Uncertainty"

The GNM algorithm is outlined in a paper by Srihari Govindan and
Robert Wilson titled "A global Newton method to compute Nash 
equilibria".  It will appear in the Journal of Economic Theory in 
2003.  

It can be found at 
http://faculty-gsb.stanford.edu/wilson/pdf%20files/GovindanWilsonGlobalNewtonMethod010904.pdf

The IPA algorithm is outlined in a companion to the above paper, also
by Govindan and Wilson, titled "Computing Nash Equilibria by Iterated
Polymatrix Approximation".  It will appear in the Journal of Economic 
Dynamics and Control in 2003.

It can be found at
http://faculty-gsb.stanford.edu/wilson/pdf%20files/PolymatrixApproximationJEDC,010718.pdf

For information about similar results for extensive form games, please 
refer to the following paper by Govindan and Wilson:
Structure theorems for game trees. Govindan, S., and Wilson, R.  
Proceedings of the National Academy of Sciences, 2002. Vol. 99, 
Issue 13: 9077-9080

It can be found at
http://www.pnas.org/cgi/doi/10.1073/pnas.082249599
