## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit-efg-lp gambit-nfg-lp

AM_CPPFLAGS = -I$(top_srcdir)/sources

gambit_nfg_lp_SOURCES = \
	basis.cc basis.h \
	bfs.cc bfs.h \
	btableau.cc btableau.h btableau.imp \
	lpsolve.cc lpsolve.h lpsolve.imp \
	lptab.cc lptab.h lptab.imp \
	ludecomp.cc ludecomp.h ludecomp.imp \
	tableau.cc tableau.h tableau.imp \
	nfglp.cc

gambit_nfg_lp_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la

gambit_efg_lp_SOURCES = \
	basis.cc basis.h \
	bfs.cc bfs.h \
	btableau.cc btableau.h btableau.imp \
	tableau.cc tableau.h tableau.imp \
	lpsolve.cc lpsolve.h lpsolve.imp \
	lptab.cc lptab.h lptab.imp \
	ludecomp.cc ludecomp.h ludecomp.imp \
	efglp.cc

gambit_efg_lp_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la


