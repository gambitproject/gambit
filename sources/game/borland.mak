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
	behav.cc \
	efdom.cc \
	efgcont.cc \
	efgensup.cc \
	efgiter.cc \
	efgsupport.cc \
	game.cc \
	gameinst.cc \
        infoset.cc \
	mixed.cc \
	nfdom.cc \
	nfdommix.cc \
	nfgcont.cc \
	nfgensup.cc \
	nfgiter.cc \
	nfgsupport.cc \
        node.cc \
        outcome.cc \
        player.cc \
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

