## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit-nfg-gnm gambit-nfg-ipa

AM_CPPFLAGS = -I$(top_srcdir)/sources

gambit_nfg_gnm_SOURCES = \
	cmatrix.cc cmatrix.h \
	gnm.cc gnm.h \
	gnmgame.cc gnmgame.h \
	nfgame.cc nfgame.h \
	nfggnm.cc

gambit_nfg_gnm_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la


gambit_nfg_ipa_SOURCES = \
	cmatrix.cc cmatrix.h \
	gnmgame.cc gnmgame.h \
	ipa.cc ipa.h \
	nfgame.cc nfgame.h \
	nfgipa.cc

gambit_nfg_ipa_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la

EXTRA_DIST = README

