## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit-efg-lcp gambit-nfg-lcp

AM_CPPFLAGS = -I$(top_srcdir)/sources

gambit_nfg_lcp_SOURCES = \
	basis.cc basis.h \
	bfs.cc bfs.h \
	btableau.cc btableau.h btableau.imp \
	lemketab.cc lemketab.h lemketab.imp \
	lhtab.cc lhtab.h lhtab.imp \
	ludecomp.cc ludecomp.h ludecomp.imp \
	tableau.cc tableau.h tableau.imp \
	nfglcp.cc

gambit_nfg_lcp_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la


gambit_efg_lcp_SOURCES = \
	basis.cc basis.h \
	bfs.cc bfs.h \
	btableau.cc btableau.h btableau.imp \
	lemketab.cc lemketab.h lemketab.imp \
	ludecomp.cc ludecomp.h ludecomp.imp \
	tableau.cc tableau.h tableau.imp \
	efglcp.cc

gambit_efg_lcp_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la


