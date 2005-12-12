## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit-nfg-enummixed

AM_CPPFLAGS = -I$(top_srcdir)/sources

gambit_nfg_enummixed_SOURCES = \
	basis.cc \
	basis.h \
	bfs.cc \
	bfs.h \
	btableau.cc \
	btableau.h \
	btableau.imp \
	lptab.cc \
	lptab.h \
	lptab.imp \
	ludecomp.cc \
	ludecomp.h \
	ludecomp.imp \
	tableau.h \
	tableau.imp \
	vertenum.h \
	vertenum.imp \
	clique.cc \
	clique.h \
	enummixed.cc

gambit_nfg_enummixed_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la

