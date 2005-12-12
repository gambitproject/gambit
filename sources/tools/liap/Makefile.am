## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit-efg-liap gambit-nfg-liap

AM_CPPFLAGS = -I$(top_srcdir)/sources

gambit_nfg_liap_SOURCES = funcmin.cc funcmin.h nfgliap.cc

gambit_nfg_liap_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la

gambit_efg_liap_SOURCES = funcmin.cc funcmin.h efgliap.cc

gambit_efg_liap_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la


