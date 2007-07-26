## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit-gnm gambit-ipa

AM_CPPFLAGS = -I$(top_srcdir)/src

gambit_gnm_SOURCES = \
	cmatrix.cc cmatrix.h \
	gnm.cc gnm.h \
	gnmgame.cc gnmgame.h \
	nfgame.cc nfgame.h \
	nfggnm.cc

gambit_gnm_LDADD = \
	$(top_builddir)/src/libgambit/libgambit.la


gambit_ipa_SOURCES = \
	cmatrix.cc cmatrix.h \
	gnmgame.cc gnmgame.h \
	ipa.cc ipa.h \
	nfgame.cc nfgame.h \
	nfgipa.cc

gambit_ipa_LDADD = \
	$(top_builddir)/src/libgambit/libgambit.la

EXTRA_DIST = README

