## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit-lp

AM_CPPFLAGS = -I$(top_srcdir)/sources

gambit_lp_SOURCES = \
	basis.cc basis.h \
	bfs.cc bfs.h \
	btableau.cc btableau.h btableau.imp \
	lpsolve.cc lpsolve.h lpsolve.imp \
	lptab.cc lptab.h lptab.imp \
	ludecomp.cc ludecomp.h ludecomp.imp \
	tableau.cc tableau.h tableau.imp \
	efglp.cc nfglp.cc lp.cc

gambit_lp_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la
