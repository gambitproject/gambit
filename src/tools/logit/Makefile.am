## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit-logit gambit-nfg-logitdyn

AM_CPPFLAGS = -I$(top_srcdir)/sources

gambit_logit_SOURCES = logbehav.h logbehav.imp efglogit.cc nfglogit.cc logit.cc

gambit_logit_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la

gambit_nfg_logitdyn_SOURCES = nfgdyn.cc

gambit_nfg_logitdyn_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la
