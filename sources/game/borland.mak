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
	"efgiter.cc" \
	"efgutils.cc" \
	"game-file.cc" \
	"nfgiter.cc" \
	"sfg.cc" \
	"sfstrat.cc" \
	"table-mixed-double.cc" \
	"table-mixed-mpfloat.cc" \
	"table-mixed-rational.cc" \ 
	"table-contingency.cc" \
        "table-file.cc" \
	"table-game.cc" \
	"table-inst.cc" \
	"table-outcome.cc" \
	"table-player.cc" \
	"tree-behav-double.cc" \
	"tree-behav-mpfloat.cc" \
	"tree-behav-pure.cc" \
	"tree-behav-rational.cc" \
	"tree-contingency.cc" \
	"tree-file.cc" \
	"tree-game.cc" \
	"tree-infoset.cc" \
	"tree-inst.cc" \
	"tree-node.cc" \
	"tree-outcome.cc" \
	"tree-player.cc" 

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
        copy *.obj ..
        
clean:
        -erase *.obj
        -erase *.exe
        -erase *.res
        -erase *.map
        -erase *.rws

