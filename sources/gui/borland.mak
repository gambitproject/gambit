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

WXDIR = c:\wx2
WXLIBDIR = $(WXDIR)\lib
WXLIB = wx32 xpm tiff jpeg winpng zlib

EXTRACPPFLAGS = -v -I$(WXDIR)\include -I$(BCCDIR)\include -I.. -D__BCC55__
EXTRALINKFLAGS = 

!include filelist.in
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




