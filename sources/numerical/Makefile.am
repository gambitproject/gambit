## Process this file with automake to create Makefile.in

noinst_LTLIBRARIES = libgambit_numerical.la

AM_CPPFLAGS = -I$(top_srcdir)/sources

numericalincludedir = $(includedir)/gambit-$(VERSION)/gambit/numerical
numericalinclude_HEADERS = \
	basis.h \
	bfs.h \
	btableau.h \
	btableau.imp \
	gfunc.h \
	gfuncmin.h \
	lemketab.h \
	lemketab.imp \
	linrcomb.h \
	linrcomb.imp \
	lpsolve.h \
	lpsolve.imp \
	lptab.h \
	lptab.imp \
	ludecomp.h \
	ludecomp.imp \
	tableau.h \
	tableau.imp \
	vertenum.h \
	vertenum.imp

libgambit_numerical_la_SOURCES = \
	basis.cc \
	basis.h \
	bfs.cc \
	bfs.h \
	btableau.cc \
	btableau.h \
	btableau.imp \
	gfunc.cc \
	gfunc.h \
	gfuncmin.cc \
	gfuncmin.h \
	lemketab.cc \
	lemketab.h \
	lemketab.imp \
	linrcomb.cc \
	linrcomb.h \
	linrcomb.imp \
	lpsolve.cc \
	lpsolve.h \
	lpsolve.imp \
	lptab.cc \
	lptab.h \
	lptab.imp \
	ludecomp.cc \
	ludecomp.h \
	ludecomp.imp \
	tableau.cc \
	tableau.h \
	tableau.imp \
	vertenum.cc \
	vertenum.h \
	vertenum.imp

EXTRA_DIST = borland.mak