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

EXTRACPPFLAGS = -I..\..\include
EXTRALINKFLAGS = -Tpe -aa -V4.0 -c

SOURCES = garray.cc gblock.cc glist.cc gmisc.cc gpool.cc \
          grarray.cc gstream.cc gtext.cc gwatch.cc
OBJECTS = $(SOURCES:.cc=.obj)

CFG = ..\gambit32.cfg
WXLIBDIR = $(WXDIR)\lib
WXLIB = wx32 xpm
WXINC = -I$(WXDIR)\include

OPT = -Od

$(SOURCE_SUFFIX).obj:
	bcc32 $(CPPFLAGS) -P -c {$< }

GUILIBS=$(WXLIB) cw32mti import32 ole2w32
GCLLIBS=cw32mti import32 ole2w32 bfc40 bfcs40


LINKFLAGS= /Tpe /L$(WXLIBDIR);$(BCCDIR)\lib $(EXTRALINKFLAGS)
OPT = -Od
DEBUG_FLAGS= -v


CPPFLAGS= $(WXINC) $(EXTRACPPFLAGS) $(OPT) @$(CFG)

gambit_base: $(OBJECTS)
        -erase gambit_base.lib
	tlib gambit_base /P1024 @&&!
+$(OBJECTS:.obj =.obj +) +$(PERIPH_LIBS:.lib =.lib +)
!

clean:
        -erase *.obj
        -erase *.exe
        -erase *.res
        -erase *.map
        -erase *.rws

