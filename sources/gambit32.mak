#
# FILE: gambit32.mak -- Makefile for Gambit under Borland C++ 32-bit
#
# $Id$
#

.AUTODEPEND

# The following directories must be set or overriden for the target setup
WXDIR = E:\WX2
BCCDIR = C:\BC5

SOURCE_SUFFIX = .cc
OBJECT_SUFFIX = .obj

EXTRACPPFLAGS = -v 
EXTRALINKFLAGS = -Tpe -aa -v -V4.0 -c

!include make.filelist

CFG = gambit32.cfg
WXLIBDIR = $(WXDIR)\lib
WXLIB = wx32 xpm
WXINC = -I$(WXDIR)\include

OPT = -Od

$(SOURCE_SUFFIX).obj:
	bcc32 $(CPPFLAGS) -P -c {$< }

.c.obj:
	bcc32 $(CPPFLAGS) -P- -c {$< }

GUILIBS=$(WXLIB) cw32mti import32 ole2w32
GCLLIBS=cw32mti import32 ole2w32 bfc40 bfcs40


LINKFLAGS= /Tpe /L$(WXLIBDIR);$(BCCDIR)\lib $(EXTRALINKFLAGS)
OPT = -Od
DEBUG_FLAGS= -v


CPPFLAGS= -D_AFXDLL $(WXINC) -I..\winsrc -I$(BCCDIR)\include\mfc $(EXTRACPPFLAGS) $(OPT) @$(CFG)

all: gambit gcl

gambit:	$(ALLGUI_OBJECTS) gambit.res
  ilink32 $(LINKFLAGS) @&&!
c0w32.obj $(ALLGUI_OBJECTS)
gambit
nul
$(GUILIBS)

gambit.res
!

gambit.res :      ..\winsrc\res\gambit.rc 
    brc32 -r -fo.\gambit.res /i$(BCCDIR)\include /i$(WXDIR)\include\wx\msw /i$(WXDIR)\include ..\winsrc\res\gambit

gcl:	$(MFCGCL_OBJECTS) gcl.res winedit.res
  ilink32 $(LINKFLAGS) @&&!
c0w32.obj $(MFCGCL_OBJECTS) gwstream.obj
gcl
nul
$(GCLLIBS)

gcl.res winedit.res
!

gcl.res :      ..\winsrc\res\gcl.rc 
    brc32 -r -fo.\gcl.res /i$(BCCDIR)\include /i$(BCCDIR)\include\mfc /i..\winsrc ..\winsrc\res\gcl

winedit.res :      ..\winsrc\res\winedit.rc 
    brc32 -r -fo.\winedit.res /i$(BCCDIR)\include /i$(BCCDIR)\include\mfc /i..\winsrc ..\winsrc\res\winedit

wxgcl:	$(WXGCL_OBJECTS)  wxgcl.res
  ilink32 $(LINKFLAGS) @&&!
c0w32.obj $(WXGCL_OBJECTS) 
wxgcl
nul
$(GUILIBS)

wxgcl.res
!

wxgcl.res :      ..\winsrc\res\wxgcl.rc 
    brc32 -r -fo.\wxgcl.res /i$(BCCDIR)\include /i$(WXDIR)\include\wx\msw /i$(WXDIR)\include ..\winsrc\res\wxgcl

gcompile.cc:     gcompile.yy
   ..\bisondos\bison -d -h gcompile.h -o gcompile.cc gcompile.yy 

readefg.cc:      readefg.yy
   ..\bisondos\bison -d -h readefg.h -o readefg.cc readefg.yy

readnfg.cc:      readnfg.yy
   ..\bisondos\bison -d -h readnfg.h -o readnfg.cc readnfg.yy

clean:
        -erase *.obj
        -erase *.exe
        -erase *.res
        -erase *.map
        -erase *.rws

