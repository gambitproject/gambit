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


Refactor and update game representation library
-----------------------------------------------

The basic library (in `src/libgambit`) for representing games was
written in the mid-1990s.  As such, it predates many modern C++
features (including templates, STL, exceptions, Boost, and so on).
This project involves carrying out a thorough review of the
basic library code.  Tasks will/may include:

* Replacing the existing reference-counting mechanism with a more
  flexible approach to referring to elements of games.
* Implementing the concepts of "strategy support profiles" and
  "strategy sets" as currently implemented at the Python level.
* Implementing internal representations using STL, and iterators over
  objects in STL-compatible ways.
* Conducting experiments on internal representation structures for
  performance scalability on larger games.

All tasks will involve coordination with the Python API to ensure
this does not break as changes are made, so a working knowledge of
Python/Cython is indicated.

* **Languages**: C++; Python/Cython.
* **Prerequisites**: Introductory game theory for familiarity with
  terminology of the objects in a game; undergraduate-level software
  engineering experience.

Integrate Action Graph Games support into main Gambit distribution
------------------------------------------------------------------

Through the work of Albert Jiang and the research group of
Prof Kevin Leyton-Brown at University of British Columbia, there
is an implementation of support for the Action Graph Games
representation structure.  See

  Jiang, A. X., Leyton-Brown, K., and Bhat, N. A. R. (2011)
  Action-graph games. Games and Economic Behavior 71(1): 141-173.
  http://dx.doi.org/10.1016/j.geb.2010.10.012

A preliminary integration of the work has been done in the
`agg` branch in the Gambit git repository.

This project would involve completing the integration of their
work for distribution.  The primary tasks will involve code
refactoring, documentation, and the construction of a test suite.

* **Languages**: C++; optionally Python/Cython for integration with the
  Python API.
* **Prerequisites**: Undergraduate-level software engineering
  experience; adequate game theory to understand the action graph
  games representation as described in the 2011 article.



Implementing algorithms for finding equilibria in games
-------------------------------------------------------

Each of the following are separate ideas for open projects on
computing equilibria and other interesting quantities on games.
Each of these is a single project  For GSoC applications, you should
select exactly one of these, as each is easily a full summer's worth
of work (no matter how easy some of them may seem at first read!)

Enumerating all equilibria of a two-player bimatrix game using the EEE algorithm
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The task is to implement the EEE algorithm, which is a published algorithm to
enumerate all extreme equilibria of a bimatrix game.

* **Languages:** C, Java
* **Prerequisites:**  Background in game theory, basic linear
  algebra and linear programming.  Experience with programs of at least
  medium complexity so that existing code can be expanded.

**Fuller details:**

The task is to implement the EEE algorithm, which is a published algorithm to
enumerate all extreme equilibria of a bimatrix game.

The most up-to-date version can be found in Sections 7 and 8
of

    D. Avis, G. Rosenberg, R. Savani, and B. von Stengel (2010),
    Enumeration of Nash equilibria for two-player games.
    Economic Theory 42, 9-37. 

    http://www.maths.lse.ac.uk/Personal/stengel/ETissue/ARSvS.pdf

Extra information, including some code,
is provided in the following report:

    G. Rosenberg (2004),
    Enumeration of All Extreme Equilibria of Bimatrix Games with Integer Pivoting and Improved Degeneracy Check.
    CDAM Research Report LSE-CDAM-2004-18.

    http://www.cdam.lse.ac.uk/Reports/Files/cdam-2005-18.pdf

The original algorithm was described in the following paper:
    
    C. Audet, P. Hansen, B. Jaumard, and G. Savard (2001),
    Enumeration of all extreme equilibria of bimatrix games. 
    SIAM Journal on Scientific Computing 23, 323–338.

The implementation should include a feature to compare the
algorithm's output (a list of extreme equilibria) with the
ouput of other algorithms for the same task (e.g.
``lrsnash``).

In addition a framework that compares running times (and the
number of recursive calls, calls to pivoting methods, and
other crucial operations) should be provided.
The output should record and document the computational
experiments so that they can be reproduced, in a general
setup - sufficiently documented - that can be used for
similar comparisons.


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


 

