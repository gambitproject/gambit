## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit

AM_CPPFLAGS = -I$(top_srcdir)/sources

gambit_SOURCES = \
	dleditbehav.cc \
	dleditcont.cc \
	dleditefg.cc \
	dleditmixed.cc \
	dleditmove.cc \
	dleditnode.cc \
	dlefgdelete.cc \
	dlefgnash.cc \
	dlefgreveal.cc \
	dlelimbehav.cc \
	dlelimmixed.cc \
	dlinsertmove.cc \
	dlnewgame.cc \
	dlnfgnash.cc \
	dlnfgproperties.cc \
	dlnfgstrategies.cc \
	efglayout.cc \
	efgnavigate.cc \
	efgoutcome.cc \
	efgprint.cc \
	efgprofile.cc \
	efgshow.cc \
	efgsupport.cc \
	gambit.cc \
	nfgnavigate.cc \
	nfgoutcome.cc \
	nfgprofile.cc \
	nfgprint.cc \
	nfgshow.cc \
	nfgsupport.cc \
	nfgtable.cc \
	splash.cc \
	treedraw.cc \
	treewin.cc \
	userprefs.cc \
	wxio.cc

#	algenumpure.cc \
#	algenummixed.cc \
#	alglcp.cc \
#	algliap.cc \
#	alglp.cc \
#	algpolenum.cc \
#	algqre.cc \
#	algqregrid.cc \
#	algsimpdiv.cc \

gambit_LDADD = \
	$(top_builddir)/sources/guishare/libguishare.a \
	$(top_builddir)/sources/nash/libnash.a \
	$(top_builddir)/sources/game/libgame.a \
	$(top_builddir)/sources/poly/libpoly.a \
	$(top_builddir)/sources/pelican/libpelican.a \
	$(top_builddir)/sources/numerical/libnumerical.a \
	$(top_builddir)/sources/math/libmath.a \
	$(top_builddir)/sources/base/libbase.a



