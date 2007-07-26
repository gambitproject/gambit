## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit-enummixed

AM_CPPFLAGS = -I$(top_srcdir)/src

gambit_enummixed_SOURCES = \
	lrsmp.h lrsmp.c lrslib.h lrslib.c lrsnash.cc \
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

gambit_enummixed_LDADD = \
	$(top_builddir)/src/libgambit/libgambit.la

