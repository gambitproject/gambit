#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Makefile for Borland C++ 5.5 for graphical user interface
#

.AUTODEPEND

!include ..\makedef.bcc

WXLIBDIR = $(WXDIR)\lib
WXLIB = wx32 tiff jpeg winpng zlib

EXTRACPPFLAGS = -v -I$(WXDIR)\include -I$(BCCDIR)\include -I.. -D__BCC55__ -DVERSION=\"0.97.0.2\"
EXTRALINKFLAGS = 

gambit_SOURCES = \
        dlabout.cc \
	dleditbehav.cc \
	dleditcont.cc \
	dleditefg.cc \
	dleditmixed.cc \
	dleditmove.cc \
	dleditnode.cc \
	dlefgcolor.cc \
	dlefgdelete.cc \
	dlefglayout.cc \
	dlefglegend.cc \
	dlefgnash.cc \
	dlefgreveal.cc \
	dlelimbehav.cc \
	dlelimmixed.cc \
	dlinsertmove.cc \
	dlnewgame.cc \
	dlnfgnash.cc \
	dlnfgproperties.cc \
	dlnfgqre.cc \
	dlnfgstrategies.cc \
	dlqrefile.cc \
	dlreport.cc \
	dlspinctrl.cc \
	efglayout.cc \
	efgnavigate.cc \
	efgoutcome.cc \
	efgprint.cc \
	efgprofile.cc \
	efgshow.cc \
	efgsupport.cc \
	gambit.cc \
	nfgnavigate.cc \
	nfgoutcome.cc \
	nfgprofile.cc \
	nfgprint.cc \
	nfgshow.cc \
	nfgsupport.cc \
	nfgtable.cc \
        numberedit.cc \
	treedraw.cc \
	treewin.cc \
        valnumber.cc \
        wxstatus.cc 

OBJECTS = $(gambit_SOURCES:.cc=.obj)

CFG = ..\gambit32.cfg

OPT = -Od

.cc.obj:
	bcc32 $(CPPFLAGS) -P -c {$< }


GUILIBS=$(WXLIB) base math game pelican poly numerical nash cw32mt import32 ole2w32

LINKFLAGS= /c /aa /L$(WXLIBDIR);$(BCCDIR)\lib;..\base;..\math;..\game;..\pelican;..\poly;..\numerical;..\nash $(EXTRALINKFLAGS)
OPT = -Od
DEBUG_FLAGS=

CPPFLAGS= $(WXINC) $(EXTRACPPFLAGS) $(OPT) @$(CFG)

gambit:	$(OBJECTS) gambit.res
  ilink32 $(LINKFLAGS) @&&!
c0w32.obj $(OBJECTS)
gambit
nul
$(GUILIBS)

gambit.res
!

gambit.res :      gambit.rc 
    brc32 -r -fo.\gambit.res /i$(BCCDIR)\include /i$(WXDIR)\include gambit

clean:
        -erase *.obj
        -erase *.exe
        -erase *.res
        -erase *.map
        -erase *.rws




