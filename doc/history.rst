************************
History and contributors
************************

What is Gambit?
===============

Gambit is a set of software tools for doing computation on finite,
noncooperative games. These comprise a graphical interface for
interactively building and analyzing general games in extensive or
strategy form; a number of command-line tools for computing Nash
equilibria and other solution concepts in games; and, a set of file
formats for storing and communicating games to external tools.

A brief history of Gambit
=========================

The Gambit Project was founded in the mid-1980s by Richard McKelvey at
the California Institute of Technology. The original implementation
was written in BASIC, with a simple graphical interface. This code was
ported to C around 1990 with the help of Bruce Bell, and was
distributed publicly as version 0.13 in 1991 and 1992.
The earliest citation of Gambit is in Wilson, R. (1992),
Computing Simply Stable Equilibria, Econometrica 60:5, 1039-1070.

Much of the early development of the main Gambit codebase took place in
1994-1996, under a grant from the National Science Foundation to the
California Institute of Technology and the University of Minnesota
(McKelvey and McLennan, principal investigators), with
`Theodore Turocy <https://www.gambit-project.org/turocy>`__ as the head programmer.
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

Gambit is proud to have participated in the Google Summer of Code
program in the summers of 2011 and 2012 as a mentoring organization.
The Python API, which became part of Gambit from Gambit 13, was
developed during these summers, thanks in particular to the work
of Stephen Kunath and Alessandro Andrioni.

Contributors
============

Others contributing to the development and distribution of Gambit
include:

* Bernhard von Stengel provided advice on implementation of
  sequence form code, and contributed clique code

* Eugene Grayver developed the first version of the
  graphical user interface.

* Gary Wu implemented an early scripting language interface for
  Gambit (since superseded by the Python API).

* Stephen Kunath and Alessandro Andrioni did extensive work to create
  the first release of the Python API.

* From Gambit 14, Gambit contains support for Action Graph Games
  [Jiang11]_.  This has been contributed by Navin Bhat, Albert Jiang,
  Kevin Leyton-Brown, and David Thompson, with funding support
  provided by a University Graduate Fellowship of the University
  of British Columbia, the NSERC Canada Graduate Scholarship, and a
  Google Research Award to Leyton-Brown.
