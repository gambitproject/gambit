## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit

AM_CPPFLAGS = $(WX_CXXFLAGS) -I$(top_srcdir)/sources

gambit_SOURCES = \
	corplot.cc \
	corplot.h \
	dlabout.cc \
	dlabout.h \
	dlch.cc \
	dlch.h \
	dleditbehav.cc \
	dleditbehav.h \
	dleditcont.cc \
	dleditcont.h \
	dleditefg.cc \
	dleditefg.h \
	dleditmixed.cc \
	dleditmixed.h \
	dleditmove.cc \
	dleditmove.h \
	dleditnode.cc \
	dleditnode.h \
	dlefgcolor.cc \
	dlefgcolor.h \
	dlefgdelete.cc \
	dlefgdelete.h \
	dlefglayout.cc \
	dlefglayout.h \
	dlefglegend.cc \
	dlefglegend.h \
	dlefgnash.cc \
	dlefgnash.h \
	dlefgreveal.cc \
	dlefgreveal.h \
	dlelimbehav.cc \
	dlelimbehav.h \
	dlelimmixed.cc \
	dlelimmixed.h \
	dlinsertmove.cc \
	dlinsertmove.h \
	dlnewgame.cc \
	dlnewgame.h \
	dlnfgnash.cc \
	dlnfgnash.h \
	dlnfgproperties.cc \
	dlnfgproperties.h \
	dlnfgqre.cc \
	dlnfgqre.h \
	dlnfgstrategies.cc \
	dlnfgstrategies.h \
	dlqrefile.cc \
	dlqrefile.h \
	dlreport.cc \
	dlreport.h \
	dlspinctrl.cc \
	dlspinctrl.h \
	efgconst.h \
	efglayout.cc \
	efglayout.h \
	efgnavigate.cc \
	efgnavigate.h \
	efgoutcome.cc \
	efgoutcome.h \
	efgprint.cc \
	efgprint.h \
	efgprofile.cc \
	efgprofile.h \
	efgshow.cc \
	efgshow.h \
	efgsupport.cc \
	efgsupport.h \
	gambit.cc \
	gambit.h \
	gamedoc.cc \
	gamedoc.h \
	nfgconst.h \
	nfgnavigate.cc \
	nfgnavigate.h \
	nfgoutcome.cc \
	nfgoutcome.h \
	nfgprint.cc \
	nfgprint.h \
	nfgprofile.cc \
	nfgprofile.h \
	nfgshow.cc \
	nfgshow.h \
	nfgsupport.cc \
	nfgsupport.h \
	nfgtable.cc \
	nfgtable.h \
	numberedit.cc \
	numberedit.h \
	prefs.cc \
	prefs.h \
	treewin.cc \
	treewin.h \
	valnumber.cc \
	valnumber.h \
	wxstatus.cc \
	wxstatus.h 

gambit_LDADD = \
        $(GUI_LIBS) \
	$(top_builddir)/sources/nash/libnash.a \
	$(top_builddir)/sources/game/libgame.a \
	$(top_builddir)/sources/poly/libpoly.a \
	$(top_builddir)/sources/pelican/libpelican.a \
	$(top_builddir)/sources/numerical/libnumerical.a \
	$(top_builddir)/sources/math/libmath.a \
	$(top_builddir)/sources/base/libbase.a

EXTRA_DIST = borland.mak gambit.rc

