#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Makefile for Borland C++ 5.5 for game module
#

.AUTODEPEND

!include ..\makedef.bcc

EXTRACPPFLAGS = -v -I$(BCCDIR)\include -I.. -D__BCC55__
EXTRALINKFLAGS = -Tpe -aa -v -V4.0 -c

libgame_a_SOURCES = \
        actiter.cc \
	behavinst.cc \
	efbasis.cc \
	efdom.cc \
	efg.cc \
	efgensup.cc \
	efginst.cc \
	efgiter.cc \
	efgnfg.cc \
	efgutils.cc \
	efstrat.cc \
	nfdom.cc \
	nfdommix.cc \
	nfg.cc \
	nfgensup.cc \
	nfginst.cc \
	nfgiter.cc \
	nfgutils.cc \
	nfstrat.cc \
	readefg.cc \
	readnfg.cc \
	sfg.cc \
	sfstrat.cc \
        striter.cc

OBJECTS = $(libgame_a_SOURCES:.cc=.obj)

CFG = ..\gambit32.cfg

OPT = -Od

.cc.obj:
	bcc32 $(CPPFLAGS) -P -c {$< }

LINKFLAGS= /Tpe /L$(WXLIBDIR);$(BCCDIR)\lib $(EXTRALINKFLAGS)
OPT = -Od
DEBUG_FLAGS= -v


CPPFLAGS= $(WXINC) $(EXTRACPPFLAGS) $(OPT) @$(CFG)

game: $(OBJECTS)
        -erase game.lib
	tlib game /P1024 @&&!
+$(OBJECTS:.obj =.obj +) +$(PERIPH_LIBS:.lib =.lib +)
!

clean:
        -erase *.obj
        -erase *.exe
        -erase *.res
        -erase *.map
        -erase *.rws

