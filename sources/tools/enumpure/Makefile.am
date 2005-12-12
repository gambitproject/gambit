## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit-efg-enumpure gambit-nfg-enumpure

AM_CPPFLAGS = -I$(top_srcdir)/sources

gambit_nfg_enumpure_SOURCES = nfgenumpure.cc

gambit_nfg_enumpure_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la

gambit_efg_enumpure_SOURCES = efgenumpure.cc

gambit_efg_enumpure_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la


