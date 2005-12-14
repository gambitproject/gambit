## Process this file with automake to create Makefile.in

lib_LTLIBRARIES = liblabenski.la

AM_CPPFLAGS = -I../include ${WX_CXXFLAGS} -DwxPLOT_FAST_GRAPHICS=0

liblabenski_la_SOURCES = \
	sheetatr.cpp \
	sheet.cpp \
	sheetedg.cpp \
	sheetedt.cpp \
	sheetren.cpp \
	sheetsel.cpp \
	sheetspt.cpp \
	sheettbl.cpp \
	block.cpp \
	bmpcombo.cpp \
	dropdown.cpp \
	genergdi.cpp \
	geometry.cpp \
	grid.cpp \
	menubtn.cpp \
	optvalue.cpp \
	range.cpp \
	spinctld.cpp \
	toggle.cpp \
	fparser.cpp fparser.hh fourier.c \
	lm_lsqr.cpp \
	plotcurv.cpp \
	plotdata.cpp \
	plotfunc.cpp \
	plotwin.cpp

EXTRA_DIST = ledgreen.xpm ledgrey.xpm fparser.txt fourier.txt

liblabenski_la_LDFLAGS = $(WX_LIBS)