## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit

AM_CPPFLAGS = $(WX_CXXFLAGS) -I/usr/local/include/gambit

gambit_SOURCES = \
	dialog-about.cc \
	dialog-about.h \
	gambit.cc \
	gambit.h \
	game-document.cc \
	game-document.h \
	game-frame.cc \
	game-frame.h

gambit_LDADD = -lgambit

EXTRA_DIST = borland.mak gambit.rc

