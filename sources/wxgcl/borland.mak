#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Makefile for Borland C++ 5.5 for wxGCL
#

.AUTODEPEND

!include ..\makedef.bcc

WXLIBDIR = $(WXDIR)\lib
WXLIB = wx32 tiff jpeg winpng zlib

EXTRACPPFLAGS = -v -I$(WXDIR)\include -I$(BCCDIR)\include -I.. -D__BCC55__ -DVERSION=\"0.97.1.0\"
EXTRALINKFLAGS = 

wxgcl_SOURCES = dlabout.cc wxgcl.cc wxstatus.cc
OBJECTS = $(wxgcl_SOURCES:.cc=.obj)

CFG = ..\gambit32.cfg

OPT = -Od

.cc.obj:
	bcc32 $(CPPFLAGS) -P -c {$< }


GUILIBS=$(WXLIB) base math game pelican poly numerical nash gcl cw32mt import32 ole2w32

LINKFLAGS= /c /aa /L$(WXLIBDIR);$(BCCDIR)\lib;..\base;..\math;..\game;..\pelican;..\poly;..\numerical;..\nash;..\gcl $(EXTRALINKFLAGS)
OPT = -Od
DEBUG_FLAGS=

CPPFLAGS= $(WXINC) $(EXTRACPPFLAGS) $(OPT) @$(CFG)

wxgcl:	$(OBJECTS) wxgcl.res
  ilink32 $(LINKFLAGS) @&&!
c0w32.obj $(OBJECTS)
wxgcl
nul
$(GUILIBS)

wxgcl.res
!

wxgcl.res :      wxgcl.rc 
    brc32 -r -fo.\wxgcl.res /i$(BCCDIR)\include /i$(WXDIR)\include wxgcl

clean:
        -erase *.obj
        -erase *.exe
        -erase *.res
        -erase *.map
        -erase *.rws




