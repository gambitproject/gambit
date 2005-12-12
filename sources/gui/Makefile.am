## Process this file with automake to create Makefile.in

SUBDIRS = bitmaps

bin_PROGRAMS = gambit

AM_CPPFLAGS = -I$(top_srcdir)/sources ${WX_CXXFLAGS}

gambit_SOURCES = \
	analysis.cc \
	analysis.h \
	ctrlplay.cc \
	ctrlplay.h \
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
	dlnfglogit.h \
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
	subsolve.cc \
	subsolve.h \
	style.cc \
	style.h \
	valnumber.cc \
	valnumber.h \
	sheetatr.cc sheetatr.h \
	sheet.cc sheet.h \
	sheetedg.cc sheetedg.h \
	sheetedt.cc sheetedt.h \
	sheetren.cc sheetren.h \
	sheetsel.cc sheetsel.h \
	sheetspt.cc sheetspt.h \
	sheettbl.cc sheettbl.h \
	sheetval.cc sheetval.h \
	pairarr.h sheetdef.h srtobjar.h wx24defs.h \
	tinyxml.cc \
	tinyxml.h \
	tinyxmlerror.cc \
	tinyxmlparser.cc


gambit_LDADD = \
        $(RC_OBJECT_PATH) $(WX_LIBS) \
	$(top_builddir)/sources/libgambit/libgambit.la



