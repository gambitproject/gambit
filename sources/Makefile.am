## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit

AM_CPPFLAGS = $(WX_CXXFLAGS) -I/usr/local/include/gambit

gambit_SOURCES = \
	control-player.cc \
	control-player.h \
	dialog-about.cc \
	dialog-about.h \
	gambit.cc \
	gambit.h \
	game-document.cc \
	game-document.h \
	game-frame.cc \
	game-frame.h \
	pairarr.h \
	panel-nash.cc \
	panel-nash.h \
	panel-qre.cc \
	panel-qre.h \
	sheet.cc \
	sheet.h \
	sheetctl.cc \
	sheetctl.h \
	sheetsel.cc \
	sheetsel.h \
	sheetspt.cc \
	sheetspt.h \
	table-matrix.cc \
	table-matrix.h \
	table-schelling.cc \
	table-schelling.h \
	tree-display.cc \
	tree-display.h \
	wx24defs.h

gambit_LDADD = -lgambit

EXTRA_DIST = borland.mak gambit.rc

