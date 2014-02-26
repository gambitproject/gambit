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

Students who are interested in applying to participate in the
Google Summer of Code program should also first read our
introductory document and application template at
`<http://www.gambit-project.org/application.txt>`_.

For beginning contibutors - especially those who are interested
in potentially applying to work on Gambit projects in future
Google Summer of Code editions - there are a number of
`issues in the Gambit issue tracker tagged as "easy"
<https://github.com/gambitproject/gambit/issues?labels=easy&sort=created&direction=desc&state=open&page=1>`_.
These are excellent ways to get familiar with the Gambit codebase.
Contributors who have completed one or more such easy tasks will have
a significantly greater chance of being considered for possible
GSoC work.

The `Gambit source tree
<http://gambit.git.sourceforge.net/git/gitweb-index.cgi>`_ is managed
using `git <http://www.git-scm.com>`_.  It is recommended to have some
familiarity with how git works, or to be willing to learn.  (It's not
that hard, and once you do learn it, you'll wonder how you ever lived
without it.)

This section lists project ideas pertaining to the Gambit library
and desktop interface.  There are additional project opportunities
in the Game Theory Explorer web/cloud interface.  These are
`listed separately here <http://gte.csc.liv.ac.uk/index/index.html#document-ideas>`_.


Refactor and update game representation library
-----------------------------------------------

The basic library (in :file:`src/libgambit`) for representing games was
written in the mid-1990s.  As such, it predates many modern C++
features (including templates, STL, exceptions, Boost, and so on).
There are a number of projects for taking existing functionality,
refactoring it into separate components, and then enhancing what
those components can do.


File formats for serializing games
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Gambit supports a number of file formats for reading and writing
games.  There are traditional formats for extensive and strategic
games.  The graphical interface wraps these formats into an XML
document which can store additional metadata about the game.  The Game
Theory Explorer also defines an XML format for storing games.  And,
from Gambit 14, there is a special file format for representing
games in action graph format.

Separately, Gambit has a command-line tool for outputting games in
HTML and LaTeX formats.

This project would refactor these features into a unified framework
for reading and writing games in various formats.  It would include:

* Migrating the XML manipulation code from the graphical interface
  into the basic game representation library.
* Implementing in C++ the reader/writer for Game Theory Explorer files
  (a first version of this is available in Python in the gambit.gte
  module).
* Unifying the :program:`gambit-nfg2tex` and
  :program:`gambit-nfg2html` command line tools into a
  :program:`gambit-convert` tool, which would convert from and to many
  file formats.
* Extend all the Gambit command-line tools to read files of any
  accepted format, and not just .efg and .nfg files.

* **Languages**: C++; Python/Cython optional; XML experience helpful
* **Prerequisites**: Introductory game theory for familiarity with
  terminology of the objects in a game; undergraduate-level software
  engineering experience.

Structure equilibrium calculations using the strategy pattern
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Gambit's architecture packages methods for finding Nash equilibria as
standalone command-line tools.  Owing to different histories in
implementing these methods, internally the interfaces to these methods
at the C++ level are quite heterogeneous.  In some cases, something
like the "strategy pattern" has been used to encapsulate these
algorithms.  In others, the interface is simply a global-scope
function call with little or no structured interface.

This project would involve organizing all these interfaces in a
consistent and unified way using the "strategy pattern."  One can see
an emerging structure in the :program:`gambit-enumpure`
implementation at :file:`src/tools/enumpure/enumpure.cc` in the master
branch of the git repository.  The idea would be to develop a unified
framework for being able to interchange methods to compute Nash
equilibria (or other concepts) on games.  If the project were to go
well, as an extension these interfaces could then be wrapped in the
Python API for direct access to the solvers (which are currently
called via the command-line tools).

* **Languages**: C++.
* **Prerequisites**: Introductory game theory for familiarity with
  terminology of the objects in a game; undergraduate-level software
  engineering experience.


Implement Strategic Restriction of a game in C++
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Gambit has a concept of a :cpp:class:`StrategySupport` (defined in
:file:`src/libgambit/stratspt.h` and :file:`src/libgambit/stratspt.cc`), 
which is used, among other things, to represent a game where strictly
dominated strategies have been eliminated (which can be useful in
improving the efficiency of equilibrium computations).  The
implementation of this has historically been awkward.  Proper OO
design would suggest that a :cpp:class:`StrategySupport` should be
able to be used anywhere a :cpp:class:`Game` could be used, but this
is not the case.  In practice, a :cpp:class:`StrategySupport` has just
been a subset of the strategies of a game, which has to be passed
along as a sidecar to the game in order to get anything done.

Recently, in the Python API, the model for dealing with this has been
improved.  In Python, there is a :py:class:`StrategicRestriction`
of a game, which in fact can be used seamlessly anyplace a game can be
used.  Separately, there is a :py:class:`StrategySupportProfile`,
which is basically just a subset of strategies of a game.  This
separation of concepts has proven to be clean and useful.

The project would be to develop the concept of a strategic restriction
in C++, using the Python API as a model, with the idea of ultimately
replacing the :cpp:class:`StrategySupport`.

* **Languages**: C++; Python/Cython useful for understanding the
  current implementation in Python.
* **Prerequisites**: Introductory game theory for familiarity with
  terminology of the objects in a game; undergraduate-level software
  engineering experience.

Implement Behavior Restriction of a game in Python
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Python API has a concept of a :py:class:`StrategicRestriction` of
a game, which is the restriction of a game to a subset of strategies
for each player.  This restriction can be used seamlessly anywhere a
game can be used.

This project would develop a parallel concept of a
:py:class:`BehaviorRestriction`.  Logically this is similar to a
:py:class:`StrategicRestriction`, except that instead of operating on
the set of reduced strategic game strategies, it would operate on
behavior strategies (actions at information sets) in a game tree.

This is a bit more challenging than the
:py:class:`StrategicRestriction` because of the need to be able to
traverse the resulting game tree.  Removing actions from a game can
result in entire subtrees of the game being removed, which can then
include the removal of information sets from the game so restricted.

The idea of this project is to carry out the implementation in
Python/Cython first, as the experience from the strategic restriction
project was that the more rapid prototyping possible in Python was a
big help.  However, as the ultimate goal will be to provide this at
the C++ level, there is also the possibility of attacking the problem
directly in C++ as well.

* **Langauges**: Python/Cython; C++.
* **Prerequisites**: Introductory game theory for familiarity with
  terminology of the objects in a game; undergraduate-level software
  engineering experience.





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

* **Langauges:** C++
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


 

