#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Makefile for Borland C++ 5.5 for base module
#

.AUTODEPEND

!include ..\makedef.bcc

EXTRACPPFLAGS = -v -I$(BCCDIR)\include -D__BCC55__
EXTRALINKFLAGS = -Tpe -aa -v -V4.0 -c

libbase_a_SOURCES = \
	garray.cc \
	gblock.cc \
	glist.cc \
	gmisc.cc \
	gpool.cc \
	grarray.cc \
	grblock.cc \
	gstream.cc \
	gtext.cc \
	gwatch.cc \
	odometer.cc \
	system.cc

OBJECTS = $(libbase_a_SOURCES:.cc=.obj)

CFG = ..\gambit32.cfg

OPT = -Od

.cc.obj:
	bcc32 $(CPPFLAGS) -P -c {$< }

LINKFLAGS= /Tpe /L$(WXLIBDIR);$(BCCDIR)\lib $(EXTRALINKFLAGS)
OPT = -Od
DEBUG_FLAGS= -v


CPPFLAGS= $(WXINC) $(EXTRACPPFLAGS) $(OPT) @$(CFG)

base: $(OBJECTS)
        -erase base.lib
	tlib base /P1024 @&&!
+$(OBJECTS:.obj =.obj +) +$(PERIPH_LIBS:.lib =.lib +)
!

clean:
        -erase *.obj
        -erase *.exe
        -erase *.res
        -erase *.map
        -erase *.rws

