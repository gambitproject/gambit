#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Makefile for Borland C++ 5.5 for graphical user interface
#

.AUTODEPEND

!include makedef.bcc

WXLIBDIR = $(WXDIR)\lib
WXLIB = wx24s_bcc tiff jpeg winpng zlib

EXTRACPPFLAGS = -v -I$(WXDIR)\include -I$(WXDIR)\lib\msw -I$(BCCDIR)\include -I$(GAMBITDIR)\include -I$(GAMBITDIR)\include\gambit -D__BCC55__ -DVERSION=\"0.97.1.6\"
EXTRALINKFLAGS = 

gambit_SOURCES = \
	"control-player.cc" \
	"dialog-about.cc" \
	"gambit.cc" \
	"game-document.cc" \
	"game-frame.cc" \
	"panel-mixed-detail.cc" \
	"panel-mixed-list.cc" \
	"panel-nash.cc" \
	"panel-qre.cc" \
	"sheet.cc" \
	"sheetctl.cc" \
	"sheetsel.cc" \
	"sheetspt.cc" \
	"table-matrix.cc" \
	"table-schelling.cc" \
	"text-window.cc" \
	"thread-nash-mixed.cc" \
	"tree-display.cc" \
        "tree-layout.cc" \
	"tree-print.cc"

OBJECTS = $(gambit_SOURCES:.cc=.obj)

CFG = gambit32.cfg

OPT = -Od

.cc.obj:
	bcc32 $(CPPFLAGS) -P -c {$< }


GUILIBS=$(WXLIB) gambit cw32mt import32 ole2w32

LINKFLAGS= /c /aa /L$(WXLIBDIR);$(BCCDIR)\lib;"$(GAMBITDIR)\lib" $(EXTRALINKFLAGS)
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




