#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Makefile for Borland C++ 5.5 for pelican module
#

.AUTODEPEND

!include ..\makedef.bcc

EXTRACPPFLAGS = -v -I$(BCCDIR)\include -I.. -D__BCC55__
EXTRALINKFLAGS = -Tpe -aa -v -V4.0 -c

libpelican_a_SOURCES = \
	pelclhpk.cc \
	pelclqhl.cc \
	pelclyal.cc \
	pelconv.cc \
	peleval.cc \
	pelgennd.cc \
	pelgmatr.cc \
	pelhomot.cc \
	pelpred.cc \
	pelprgen.cc \
	pelproc.cc \
	pelpsys.cc \
	pelqhull.cc \
	pelsymbl.cc \
	pelutils.cc

OBJECTS = $(libpelican_a_SOURCES:.cc=.obj)

CFG = ..\gambit32.cfg

OPT = -Od

.cc.obj:
	bcc32 $(CPPFLAGS) -P -c {$< }

LINKFLAGS= /Tpe /L$(WXLIBDIR);$(BCCDIR)\lib $(EXTRALINKFLAGS)
OPT = -Od
DEBUG_FLAGS= -v


CPPFLAGS= $(WXINC) $(EXTRACPPFLAGS) $(OPT) @$(CFG)

pelican: $(OBJECTS)
        -erase pelican.lib
	tlib pelican /P1024 @&&!
+$(OBJECTS:.obj =.obj +) +$(PERIPH_LIBS:.lib =.lib +)
!

clean:
        -erase *.obj
        -erase *.exe
        -erase *.res
        -erase *.map
        -erase *.rws

