#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Makefile for Borland C++ 5.5 for Python interface
#

.AUTODEPEND

!include ..\makedef.bcc

PYTHONDIR = c:\python23

EXTRACPPFLAGS = -v -I$(PYTHONDIR)\include -I$(BCCDIR)\include -I.. -D__BCC55__
EXTRALINKFLAGS = 

CFG = ..\gambit32.cfg

OPT = -Od

.cxx.obj:
	bcc32 $(CPPFLAGS) -P -c {$< }

LIBS=gambit $(PYTHONDIR)\libs\python23_bcpp.lib import32 cw32mt

LINKFLAGS= /Tpd /Gn /q /x  /L$(BCCDIR)\lib;.. $(EXTRALINKFLAGS)
OPT = -Od
DEBUG_FLAGS=

CPPFLAGS= $(WXINC) $(EXTRACPPFLAGS) $(OPT) @$(CFG)

all:  gbt

gbt:    gbt_wrap.obj 
  ilink32 $(LINKFLAGS) @&&!
c0d32.obj gbt_wrap.obj
_gbt.pyd
nul
$(LIBS)
gbt.def

!

SWIGFILES = \
    behav.i \
    game.i \
    gbt.i \
    infoset.i \
    mixed.i \
    nash.i \
    node.i \
    outcome.i \
    player.i \
    rational.i

gbt_wrap.cxx:   $(SWIGFILES)
        swig -c++ -python gbt.i


clean:
        -erase *.obj
        -erase *.exe
        -erase *.res
        -erase *.map
        -erase *.rws




