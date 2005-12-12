## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit-efg-logit gambit-nfg-logit gambit-nfg-logitdyn

AM_CPPFLAGS = -I$(top_srcdir)/sources

gambit_nfg_logit_SOURCES = nfglogit.cc

gambit_nfg_logit_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la

gambit_nfg_logitdyn_SOURCES = nfgdyn.cc

gambit_nfg_logitdyn_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la

gambit_efg_logit_SOURCES = efglogit.cc

gambit_efg_logit_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la


