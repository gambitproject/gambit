#
# FILE: gambit32.mak -- Makefile for Gambit under Borland C++ 32-bit
#
# $Id$
#

.AUTODEPEND

# The following directories must be set or overriden for the target setup
WXDIR = C:\WX2
BCCDIR = C:\BORLAND\BCC55
BISON = C:\GAMBIT\BISONDOS\BISON
	
SOURCE_SUFFIX = .cc
OBJECT_SUFFIX = .obj

# Define __BCC55__ if using Borland 5.5
EXTRACPPFLAGS = -D__BCC55__ -I$(BCCDIR)\include -I..\include -Ibase -Imath
EXTRALINKFLAGS = 

!include make.filelist

CFG = gambit32.cfg
WXLIBDIR = $(WXDIR)\lib
WXLIB = wx32 xpm tiff jpeg winpng zlib
WXINC = -I$(WXDIR)\include

.path.cc = .;base;math;gui

$(SOURCE_SUFFIX).obj:
	bcc32 $(CPPFLAGS) -P -c {$< }

.c.obj:
	bcc32 $(CPPFLAGS) -P- -c {$< }

GUILIBS=$(WXLIB) cw32mt import32 ole2w32
GCLLIBS=cw32mti import32 ole2w32 bfc40 bfcs40


LINKFLAGS= /c /aa /L$(WXLIBDIR);$(BCCDIR)\lib $(EXTRALINKFLAGS)
OPT = -Od
DEBUG_FLAGS=


CPPFLAGS= $(WXINC) $(EXTRACPPFLAGS) $(OPT) @$(CFG)

all: gambit wxgcl

gambit:	$(ALLGUI_OBJECTS) gambit.res
  ilink32 $(LINKFLAGS) @&&!
c0w32.obj $(ALLGUI_OBJECTS)
gambit
nul
$(GUILIBS)

gambit.res
!

gambit.res :      ..\winsrc\res\gambit.rc 
    brc32 -r -fo.\gambit.res /i$(BCCDIR)\include /i$(WXDIR)\include ..\winsrc\res\gambit

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

#
# Notes on building bison parsers on DOS
# gambit\bisondos\bison.exe is a bison-1.28 executable for DOS
# To build the bison parsers, first set the environment variable
# BISON_SIMPLE to the location of the bison.simple file, e.g.,
# SET BISON_SIMPLE=c:\gambit\bisondos\bison.simple
#

gcompile.cc:     gcompile.yy
   $(BISON) -p gcl_yy -o gcompile.cc gcompile.yy

readefg.cc:      readefg.yy
   $(BISON) -p efg_yy -o readefg.cc readefg.yy

readnfg.cc:      readnfg.yy
   $(BISON) -p nfg_yy -o readnfg.cc readnfg.yy

clean:
        -erase *.obj
        -erase *.exe
        -erase *.res
        -erase *.map
        -erase *.rws




