An Overview of Gambit
================================

What is Gambit?
--------------------

Gambit is a set of software tools for doing computation on finite,
noncooperative games. These comprise a graphical interface for
interactively building and analyzing general games in extensive or
strategy form; a number of command-line tools for computing Nash
equilibria and other solution concepts in games; and, a set of file
formats for storing and communicating games to external tools.

A brief history of Gambit
------------------------------

The Gambit Project was founded in the mid-1980s by Richard McKelvey at
the California Institute of Technology. The original implementation
was written in BASIC, with a simple graphical interface. This code was
ported to C around 1990 with the help of Bruce Bell, and was
distributed publicly as version 0.13 in 1991 and 1992.

A major step in the evolution of Gambit took place with the awarding
of the NSF grants in 1994, with McKelvey and Andrew McLennan as
principal investigators, and `Theodore Turocy <http://www.gambit-project.org/turocy>`__ as the head programmer.
The grants sponsored a complete rewrite of Gambit in C++. The
graphical interface was made portable across platforms through the use
of the wxWidgets library (`http://www.wxwidgets.org
<http://www.wxwidgets.org>`__). Version 0.94 of Gambit was released in
the late summer of 1994, version 0.96 followed in 1999, and version
0.97 in 2002. During this time, many students at Caltech and Minnesota
contributed to the effort by programming, testing, and/or documenting.
These include, alphabetically, Bruce Bell, Anand Chelian, Matthew
Derer, Nelson Escobar, Ben Freeman, Eugene Grayver, Todd Kaplan, Geoff
Matters, Brian Trotter, Michael Vanier, Roberto Weber, and Gary Wu.

Over the same period, Bernhard von Stengel, of the London School of
Economics, made significant contributions in the implementation of the
sequence form methods for two-player extensive games, and for
contributing his "clique" code for identification of equilibrium
components in two-player strategic games, as well as other advice
regarding Gambit's implementation and architecture.

The present series of Gambit releases, numbered 0.2005.mm.dd according
to release date, focuses on two objectives. First, the graphical
interface was reimplemented and modernized, with the goal of following
good interaction design principles, especially in regards to easing
the learning curve for users new to Gambit and new to game theory.
Second, the internal architecture of Gambit was refactored to increase
interoperability between the tools provided by Gambit and those
written independently. In particular, there are many more researchers
active in computation in game theory in 2005 then during the last
major Gambit development phase circa 1995, and the architecture of
Gambit is evolving to reflect this.


Key features of Gambit
---------------------------

Gambit has a number of features useful both for the researcher and the
instructor:

**Interactive, cross-platform graphical interface.** All Gambit
features are available through the use of a graphical interface, which
runs under multiple operating systems: Windows, various flavors of
Un*x (including Linux), and Mac OS X. The interface offers flexible
methods for creating extensive and strategic games. It offers an
interface for running algorithms to compute Nash equilibria, and for
visualizing the resulting profiles on the game tree or table, as well
as an interactive tool for analyzing the dominance structure of
actions or strategies in the game. The interface is useful for the
advanced researcher, but is intended to be accessible for students
taking a first course in game theory as well.

**Command-line tools for computing equilibria.** More advanced
applications often require extensive computing time and/or the ability
to script computations. All algorithms in Gambit are packaged as
individual, command-line programs, whose operation and output are
configurable.

**Extensibility and interoperability.** The Gambit tools read and
write file formats which are textual and documented, making them
portable across systems and able to interact with external tools. It
is therefore straightforward to extend the capabilities of Gambit by,
for example, implementing a new method for computing equilibria,
reimplementing an existing one more efficiently, or creating tools to
programmatically create, manipulate, and transform games, or for
econometric analysis on games.


Limitations of Gambit
--------------------------

Gambit has a few limitations that may be important in some
applications. We outline them here.

**Gambit is for finite games only.** Because of the mathematical
structure of finite games, it is possible to write many general-
purpose routines for analyzing these games. Thus, Gambit can be used
in a wide variety of applications of game theory. However, games that
are not finite, that is, games in which players may choose from a
continuum of actions, or in which players may have a continuum of
types, do not admit the same general-purpose methods.

**Gambit is for noncooperative game theory only.** Gambit focuses on
the branch of game theory in which the rules of the game are written
down explicitly, and in which players choose their actions
independently. Gambit's analytical tools center primarily around Nash
equilibrium, and related concepts of bounded rationality such as
quantal response equilibrium. Gambit does not at this time provide any
representations of, or methods for, analyzing games written in
cooperative form. (It should be noted that some problems in
cooperative game theory do not suffer from the computational
complexity that the Nash equilibrium problem does, and thus
cooperative concepts could be an interesting future direction of
development.)

**Analyzing large games may become infeasible surprisingly quickly.**
While the specific formal complexity classes of computing Nash
equilibria and related concepts are still an area of active research,
it is clear that, in the typical case, the amount of time required to
compute equilibria increases rapidly in the size of the game. In other
words, it is quite easy to write down games which will take Gambit an
unacceptably long amount time to compute the equilibria of. There are
two ways to deal with this problem in practice. One way is to better
identify good heuristic approaches for guiding the equilibrium
computation process. Another way is to take advantage of known
features of the game to guide the process. Both of these approaches
are now becoming areas of active interest. While it will certainly not
be possible to analyze every game that one would like to, it is hoped
that Gambit will both contribute to these two areas of research, as
well as make the resulting methods available to both students and
practitioners.

Developers
----------

The principal developers of Gambit are:

* `Theodore Turocy <http://www.gambit-project.org/turocy>`__, 
  University of East Anglia: director.

* Richard D. McKelvey, California Institute of Technology: 
  project founder.

* Andrew McLennan, University of Queensland: co-PI during main 
  development, developer and maintainer of polynomial-based algorithms
  for equilibrium computation.

Much of the development of the main Gambit codebase took place in
1994-1996, under a grant from the National Science Foundation to the 
California Institute of Technology and the University of Minnesota 
(McKelvey and McLennan, principal investigators).

Others contributing to the development and distribution of Gambit
include:

* Bernhard von Stengel provided advice on implementation of 
  sequence form code, and contributed clique code

* Eugene Grayver developed the first version of the 
  graphical user interface.

* Gary Wu implemented an early scripting language interface for th
  Gambit library.

* Many undergraduate and graduate students at Caltech and the
  University of Minnesota contributed in testing and documenting
  various features.

.. _section-downloading:

Downloading Gambit
------------------

Versions of Gambit are available for download from
`http://sourceforge.net/projects/gambit/files <http://sourceforge.net/projects/gambit/files>`_.

The current stable version is 0.2010.09.01. 

* `gambit-0.2010.09.01.tar.gz <http://sourceforge.net/projects/gambit/files/gambit-stable/0.2010.09.01/gambit-0.2010.09.01.tar.gz/download>`_ (source tarball, for Linux/Windows/Mac OSX)
* `gambit-0.2010.09.01.dmg <http://sourceforge.net/projects/gambit/files/gambit-stable/0.2010.09.01/gambit-0.2010.09.01.dmg/download>`_ (application binary for Mac OSX 10.6)

The previous stable version of Gambit was 0.2007.12.04.  

* `gambit-0.2007.12.04.tar.gz <http://sourceforge.net/projects/gambit/files/gambit-stable/0.2007.12.04/gambit-0.2007.12.04.tar.gz/download>`_ (source tarball, for Linux/Windows/Mac OSX)

* `gambit-0.2007.12.04.exe <http://sourceforge.net/projects/gambit/files/gambit-stable/0.2007.12.04/gambit-0.2007.12.04.exe/download>`_ (binary installer distribution for Microsoft Windows).

.. note::
   There is no Windows installer yet available for the latest stable
   version.  The previous stable version contains all the essential
   functionality and is adequate for most users.  There is a known bug
   involving viewing dominated actions in extensive games in the
   0.2007.12.04 version of the graphical interface.  Users are advised
   to avoid using this feature.  The error is entirely cosmetic and
   does not affect the correctness of equilibria calculated.


Community
---------

The following mailing lists are available for those interested in the
use and further development of Gambit:

`gambit-announce@lists.sourceforge.net <http://lists.sourceforge.net/lists/listinfo/gambit-announce>`_
  Announcement-only mailing list for notifications of new releases of
  Gambit.

`gambit-users@lists.sourceforge.net <http://lists.sourceforge.net/lists/listinfo/gambit-users>`_
  General discussion forum for teaching and research users of Gambit.

`gambit-devel@lists.sourceforge.net <http://lists.sourceforge.net/lists/listinfo/gambit-devel>`_
  Discussion for those interested in devleoping or extending Gambit, 
  or using Gambit source code in other applications.

Bug reports and inquiries
-------------------------

Bug reports or other inquiries about Gambit should be directed to
`Theodore Turocy <http://www.gambit-project.org/turocy>`__, 
project director, at ted.turocy (aht) gmail (daht) com.
