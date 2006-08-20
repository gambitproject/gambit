## Process this file with automake to create Makefile.in

SUBDIRS = bitmaps

bin_PROGRAMS = gambit

AM_CPPFLAGS = -I$(top_srcdir)/sources -I$(top_srcdir)/sources/labenski/include ${WX_CXXFLAGS}

gambit_SOURCES = \
	analysis.cc \
	analysis.h \
	edittext.cc edittext.h \
	dlabout.cc \
	dlabout.h \
	dleditmove.cc \
	dleditmove.h \
	dleditnode.cc \
	dleditnode.h \
	dlefglayout.cc \
	dlefglayout.h \
	dlefglegend.cc \
	dlefglegend.h \
	dlefglogit.cc \
	dlefglogit.h \
	dlefgreveal.cc \
	dlefgreveal.h \
	dlgameprop.cc \
	dlgameprop.h \
	dlinsertmove.cc \
	dlinsertmove.h \
	dlnash.cc \
	dlnash.h \
	dlnashmon.cc \
	dlnashmon.h \
	dlnfglogit.cc \
	efgdisplay.cc \
	efgdisplay.h \
	efglayout.cc \
	efglayout.h \
	efgpanel.cc \
	efgpanel.h \
	efgprofile.cc \
	efgprofile.h \
	gambit.cc \
	gambit.h \
	gamedoc.cc \
	gamedoc.h \
	gameframe.cc \
	gameframe.h \
	menuconst.h \
	nfgpanel.cc \
	nfgpanel.h \
	nfgprofile.cc \
	nfgprofile.h \
	nfgtable.cc \
	nfgtable.h \
	renratio.cc \
	renratio.h \
	style.cc \
	style.h \
	valnumber.cc \
	valnumber.h \
	tinyxml.cc \
	tinyxml.h \
	tinyxmlerror.cc \
	tinyxmlparser.cc \
	dcsvg.cc dcsvg.h


gambit_LDADD = \
        $(RC_OBJECT_PATH) \
	$(top_builddir)/sources/labenski/src/liblabenski.la \
	$(WX_LIBS) \
	$(top_builddir)/sources/libgambit/libgambit.la



