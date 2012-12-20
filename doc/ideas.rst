For contributors: Ideas and suggestions for Gambit-related projects
=====================================================================

Research on doing computation on finite games, and using numerical and
algorithmic methods to analyze games, are areas of quite active
research.  There are a number of opportunities for programmers of all
skill levels and backgrounds to contribute to improving and extending
Gambit.

A number of such ideas are outlined in this section.
Each project includes the required implementation environment,
and a summary of the background prerequisites
someone should have in order to take on the project successfully, in
terms of mathematics, game theory, and software engineering.

For beginning contibutors - especially those who are interested
in potentially applying to work on Gambit projects in future
Google Summer of Code editions - there are a number of
`issues in the Gambit issue tracker tagged as "easy"
<https://github.com/gambitproject/gambit/issues?labels=easy&sort=created&direction=desc&state=open&page=1>`_.
These are excellent ways to get familiar with the Gambit codebase.
Contributors who have completed one or more such easy tasks will have
a significantly greater chance of being considered for possible
GSoC work.

The `Gambit source tree <http://gambit.git.sourceforge.net/git/gitweb-index.cgi>`_
is managed using `git <http://www.git-scm.com>`_.  It is recommended to have some familiarity with how git works, or to be willing to learn.  (It's not that hard, and once you do learn it, you'll wonder how you ever lived without it.)

Here is the main system architecture and some terminology, useful for
understanding how the various projects fit into the greater whole:

* Gambit is a collection of algorithms for manipulating and analysing games.
* A user-friendly front end is GTE (game theory explorer), run as a
  web-client in a browser, which provides a GUI to enter games in 
  extensive and strategic (normal) form.
* GTE-generated games are stored in a file format (for 
  extensive and strategic form games), and are processed by 
  algorithms for finding their Nash equilibria. These algorithms
  are executed on the web server, which can also be locally installed.
* Gambit provides also a command-line tool where games can 
  be defined and the equilibrium-finding algorithms called.
  This program is invoked in a command shell with standard
  input or reading text files. These allow for the
  systematic generation of larger games with parameters.

Interfacing GTE with equilibrium-finding modules
------------------------------------------------

GTE is implemented in Java on the server and Flash
(Action Script) on the client side.
So far, all equilibrium-finding algorithms have been ported
into Java. These should be replaced by better performing
original algorithms, such as lrs in C, by invoking them via
system calls. These should be the same modules that are
invoked from the Gambit command line.

Improving the GTE graphical interface
-------------------------------------

There are a number of outstanding issues in the GTE interface,
including:

* Improved input of games in strategic form
* Input of games with more than two players

Porting GTE to JavaScript
-------------------------

The initial implementation of GTE is in Flash.  There is interest in
porting this implementation to JavaScript for greater portability.

Enhancing the web interface with online storage of games
--------------------------------------------------------

Web frameworks offer standard functionality of storage of
user data, including user identification via login. 
The project is to create such a framework and webpages to retrieve
typically used games, and store games created by the user. 

Possible extension: Record-keeping and display of results
for computational experiments.
If games are generated systematically with various
parameters, running times and computational results
should be recorded systematically.


Implementing algorithms for finding equilibria in games
-------------------------------------------------------

Each of the following are separate ideas for open projects on
computing equilibria and other interesting quantities on games.
Each of these is a single project  For GSoC applications, you should
select exactly one of these, as each is easily a full summer's worth
of work (no matter how easy some of them may seem at first read!)


Improve integration and testing of Gametracer
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Gambit incorporates the 
`Gametracer <http://dags.stanford.edu/Games/gametracer.html>`_ package
to provide
implementations of two methods for computing equilibria,
:ref:`gambit-gnm <gambit-gnm>` and :ref:`gambit-ipa <gambit-ipa>`.
The integration
is rather crude, as internally the program converts the game
from native Gambit representation into Gametracer's
representation, and the converts the output back.  Using
Gametracer's implementations as a starting point, refactor
the implementation to use Gambit's native classes directly,
and carry out experiments on the reliability and performance
of the algorithms.

* **Languages:** C++
* **Prerequisites:** Some level of comfort with linear algebra;
  enjoyment of refactoring code.


Interface with lrslib
^^^^^^^^^^^^^^^^^^^^^

Gambit's :ref:`gambit-enummixed <gambit-enummixed>` tool computes all
extreme Nash equilibria of a two-player game.  There is another
package, `lrslib by David Avis
<http://cgm.cs.mcgill.ca/~avis/C/lrs.html>`_, which implements the
same algorithm more efficiently and robustly.  There is a partial
interface with an older version of lrslib in the Gambit source tree,
which has proven not to be very reliable.  The project is to complete
the integration and testing of the lrslib integration.

* **Languages:** C/C++
* **Prerequisites:** Some level of comfort with linear algebra.

 
Finding all equilibria reachable by Lemke-Howson
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For a two-player in strategic form (also called bimatrix
games), what are the Nash equilibria that can be found using
the Lemke-Howson method?  Each pure strategy as an
"initially dropped label" leads to an equilibrium along a
computational path obtained by "pivoting" in a linear
system.  If two equilibria found in that way are different,
using the second label on the first equilibrium (and vice
versa) will find yet another equilibrium.  The set of all
equilibria reachable in that way should be recorded and is a
(normally) fast way to find many equilibria when the game is
large.

* **Prerequisites:**
  Theoretical understanding of the Lemke-Howson
  method or of the Simplex algorithm for Linear Programming.
  Literature exists that is accessible for students with at
  least senior-level background in computer science,
  mathematics or operations research.  An existing
  implementation of a Lemke-Howson style pivoting algorithm
  should be adapted with suitable alterations.

**Fuller details:**

This figure shows the typical situation for a nondegenerate
two-player game:

.. figure:: figures/lh-net.*
            :alt: a two-player game
	    :align: center

There is an "artificial equilibrium" 0 and five equilibria
1,2,3,4,5, each of which has a *sign* or *index* + or -.
The Lemke-Howson (LH) algorithm computes a piecewise linear
path from a known equilibrium, originally only 0, to another
equilibrium.  There are different ways to start, one for
each pure strategy of a player which define different LH
paths.  Here only two ways are shown, in blue and red. 
An LH path always connects two equilibria of opposite sign,
so there are an even number of them, minus the artificial
equilibrium, which gives an odd number overall.  
Here, the blue and red paths lead to two different
equilibria 1 and 2 of positive index (+).  Then the
algorithm can be run backwards on equilibrium 1 where the
blue path leads back to 0, but the red path must find
another equilibrium, here 3, of negative index (-).  
The blue path from equilibrium 2 could possibly find another
negatively indexed equilibrium like 4, but does not, it also
finds 3.  So the "network" of LH paths here is not connected
and only finds equilibria 1,2,3, but not the two equilibria
4,5 which are only connected among themselves.

Given the LH algorithm, all this is relatively
straightforward, but there is no implementation for finding
negatively indexed equilibria and the described "network".
It would also be useful to study if all equilibria can be
found for random or typical examples.

The LH algorithm is described in

    B. von Stengel (2007), Equilibrium computation for
    two-player games in strategic and extensive form. Chapter 3,
    Algorithmic Game Theory, eds. N. Nisan, T. Roughgarden, E.
    Tardos, and V. Vazirani, Cambridge Univ. Press, Cambridge,
    53-78. 

    http://www.maths.lse.ac.uk/Personal/stengel/TEXTE/agt-stengel.pdf

It is related to the simplex algorithm for linear
programming but with a different *complementary* pivoting
rule.  It is also numerically not stable because rounding
errors may violate the rule, so it needs to be implemented
with *integer pivoting*, also described in the article.

There are versions around in C and Java that implement this
which are not yet part of the public Gambit code, but will
be made public once the project starts.

Finding equilibria reachable by Lemke's algorithm with varying "covering vectors"
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Related to the Lemke-Howson method above, but with a
slightly different algorithm that has an extra parameter,
called the "covering vector".  That parameter can serve a
randomly selected starting point of the computation and
potentially reach many more equilibria.

* **Prerequisites:** Theoretical understanding of the Lemke-Howson
  method or of the Simplex algorithm for Linear Programming.
  Literature exists that is accessible for students with at
  least senior-level background in computer science,
  mathematics or operations research.  An existing
  implementation of a Lemke-Howson style pivoting algorithm
  should be adapted with suitable alterations.

Computing the index of an equilibrium component
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The task is to implement a published algorithm to compute
the so-called index of an equilibrium component in a
bimatrix game.  This component is the output to an existing
enumeration algorithm.

* **Languages:** C
* **Prerequisites:**  Senior-level mathematics, interest in game theory
  and some basic topology.

**Fuller details:**

The aim of this project is to implement an existing
algorithm that finds the index of an equilibrium component.
The relevant description of this is chapter 2 of 

    Anne Balthasar, Geometry and Equilibria in Bimatrix Games,
    PhD Thesis, London School of Economics, 2009. 

    http://www.maths.lse.ac.uk/Personal/stengel/phds/#anne

which are pages 21-41 of
    http://www.maths.lse.ac.uk/Personal/stengel/phds/anne-final.pdf

The mathematics in this chapter are pretty scary (in
particular section 2.2, which is however not needed) but the
final page 41 which describes the algorithm is less scary.

Nevertheless, this is rather advanced material because it
builds on several different existing algorithms (for finding
extreme equilibria in bimatrix games, and "cliques" that
define convex sets of equilibria, and their non-disjoint
unions that define "components").  It requires the
understanding of what equilibria in bimatrix games are
about.  These algorithms are described in

    D. Avis, G. Rosenberg, R. Savani, and B. von Stengel (2010),
    Enumeration of Nash equilibria for two-player games.
    Economic Theory 42, 9-37. 

    http://www.maths.lse.ac.uk/Personal/stengel/ETissue/ARSvS.pdf

and students who do not eventually understand that text
should not work on this project.  For this reason, at least
senior-level (= third year) mathematics is required in terms of
mathematical maturity.  In the Avis et al. (2010) paper,
pages 19-21 describe the lexicographic method for pivoting
as it is used in the simplex method for linear programming.
A variant of this lexicographic method is used in the
chapter by Anne Balthasar.  Understanding this is a
requirement to work on this project (and a good test of how
accessible all this is).

We give here two brief examples that supplement the above
literature.  Consider the following bimatrix game.  It is
very simple, and students of game theory may find it useful
to first find out on their own what the equilibria of this
game are::

    2 x 2 Payoff matrix A:

    1  1
    0  1

    2 x 2 Payoff matrix B:

    1  1
    0  1

    EE = Extreme Equilibrium, EP = Expected Payoff

    EE  1  P1:  (1)  1  0  EP=  1  P2:  (1)  1  0  EP=  1
    EE  2  P1:  (1)  1  0  EP=  1  P2:  (2)  0  1  EP=  1
    EE  3  P1:  (2)  0  1  EP=  1  P2:  (2)  0  1  EP=  1

    Connected component 1:
    {1, 2}  x  {2}
    {1}  x  {1, 2}

This shows the following:  there are 3 Nash equilibria,
which partly use the same strategies of the two players,
which are numbered (1), (2)  for each player.  It will take
a bit of time to understand the above output.  For our
purposes, the bottom "component" is most relevant:
It has two lines, and  {1, 2}  x  {2}   means
that equilibrium (1),(2)  -  which is according to the
previous list the strategy pair (1,0), (1,0)  as well as
(2),(2),   which is   (0,1), (1,0)  are "extreme
equilibria", and moreover any convex combination of (1) and
(2) of player 1  - this is the first {1, 2} - can be
combined with strategy (2) of player 2.
This is part of the "clique" output of Algorithm 2 on page
19 of Avis et al. (2010).
There is a second such convex set of equilibria in the
second line, indicated by {1}  x  {1, 2}.
Moreover, these two convex sets intersect (in the
equilibrium  (1),(2))  and form therefore a "component" of
equilibria.  For such a component, the index has to be
found, which happens to be the integer 1 in this case.

The following bimatrix game has also two convex sets of Nash
equilibria, but they are disjoint and therefore listed as
separate components on their own::

    3 x 2 Payoff matrix A:

    1  1
    0  1
    1  0

    3 x 2 Payoff matrix B:

    2  1
    0  1
    0  1

    EE = Extreme Equilibrium, EP = Expected Payoff

    Rational Output

    EE  1  P1:  (1)    1    0    0  EP=  1  P2:  (1)  1  0  EP= 2
    EE  2  P1:  (2)  1/2  1/2    0  EP=  1  P2:  (2)  0  1  EP= 1
    EE  3  P1:  (3)  1/2    0  1/2  EP=  1  P2:  (1)  1  0  EP= 1
    EE  4  P1:  (4)    0    1    0  EP=  1  P2:  (2)  0  1  EP= 1

    Connected component 1:
    {1, 3}  x  {1}

    Connected component 2:
    {2, 4}  x  {2}

Here the first component has index 1 and the second has
index 0.  One reason for the latter is that if the game is
slightly perturbed, for example by giving a slightly lower
payoff than 1 in row 2 of the game, then the second strategy
of player 1 is strictly dominated and the equilibria (2) and
(4) of player 1, and thus the entire component 2, disappear
altogether.  This can only happen if the index is zero, so
the index gives some useful information as to whether an
equilibrium component is "robust" or "stable" when payoffs
are slightly perturbed.


Enumerating all equilibria of a two-player game tree
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Extension of an existing algorithm for enumerating all
equilibria of a bimatrix game to game trees with imperfect
information using the so-called "sequence form".  The method
is described in abstract form but not implemented.  

* **Prerequisites:** Background in game theory and basic linear
  algebra.  Experience with programs of at least
  medium complexity so that existing code can be expanded.


Solving for equilibria using polynomial systems of equations
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The set of Nash equilibrium conditions can be expressed as a
system of polynomial equations and inequalities.  The field
of algebraic geometry has been developing packages to
compute all solutions to a system of polynomial equations.
Two such packages are 
`PHCpack <http://www.math.uic.edu/~jan/download.html">`_ and 
`Bertini <http://www.nd.edu/~sommese/bertini/>`_.  
Gambit has an
experimental interface, written in Python, to build the
required systems of equations, call out to the solvers, and
identify solutions corresponding to Nash equilibria.
Refactor the implementation to be more flexible and
Pythonic, and carry out experiments on the reliability and
performance of the algorithms.

* **Languages:** Python
* **Prerequisites:** Experience with text processing to pass data to
  and from the external solvers.

Implement Herings-Peeters homotopy algorithm to compute Nash equilibria
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Herings and Peeters 
(`Economic Theory, 18(1), 159-185, 2001 <http://dx.doi.org/10.1007/PL00004129>`_) have proposed a
homotopy algorithm to compute Nash equilibria.  They have
created a
`first implementation of the method in Fortran <http://www.personeel.unimaas.nl/r.peeters/software.htm>`_,
using `hompack <http://www.netlib.org/hompack/>`_.
Create a Gambit implementation of this method, and carry out
experiments on the reliability and performance of the
algorithms.

* **Languages:** C/C++, ability to at least read Fortran
* **Prerequisites:** Basic game theory and knowledge of pivoting
  algorithms like the Simplex method for Linear Programming or
  the Lemke-Howson method for games.  Senior-level
  mathematics, mathematical economics, or operations research.


 

