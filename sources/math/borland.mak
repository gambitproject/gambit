#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Makefile for Borland C++ 5.5 for math module
#

.AUTODEPEND

!include ..\makedef.bcc

EXTRACPPFLAGS = -v -I$(BCCDIR)\include -I.. -D__BCC55__
EXTRALINKFLAGS = -Tpe -aa -v -V4.0 -c

libmath_a_SOURCES = \
	complex.cc \
	double.cc \
	gdpvect.cc \
	gmatrix.cc \
	gnulib.cc \
	gnumber.cc \
	gpvector.cc \
	gsmatrix.cc \
	gvector.cc \
	integer.cc \
	mathinst.cc \
	misc.cc \
	rational.cc

OBJECTS = $(libmath_a_SOURCES:.cc=.obj)

CFG = ..\gambit32.cfg

OPT = -Od

.cc.obj:
	bcc32 $(CPPFLAGS) -P -c {$< }

LINKFLAGS= /Tpe /L$(WXLIBDIR);$(BCCDIR)\lib $(EXTRALINKFLAGS)
OPT = -Od
DEBUG_FLAGS= -v


CPPFLAGS= $(WXINC) $(EXTRACPPFLAGS) $(OPT) @$(CFG)

math: $(OBJECTS)
        -erase math.lib
	tlib math /P1024 @&&!
+$(OBJECTS:.obj =.obj +) +$(PERIPH_LIBS:.lib =.lib +)
!

clean:
        -erase *.obj
        -erase *.exe
        -erase *.res
        -erase *.map
        -erase *.rws

