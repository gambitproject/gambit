## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit-lcp

AM_CPPFLAGS = -I$(top_srcdir)/src

gambit_lcp_SOURCES = \
	basis.cc basis.h \
	bfs.cc bfs.h \
	btableau.cc btableau.h btableau.imp \
	lemketab.cc lemketab.h lemketab.imp \
	lhtab.cc lhtab.h lhtab.imp \
	ludecomp.cc ludecomp.h ludecomp.imp \
	tableau.cc tableau.h tableau.imp \
	efglcp.cc nfglcp.cc lcp.cc

gambit_lcp_LDADD = \
	$(top_builddir)/src/libgambit/libgambit.la


