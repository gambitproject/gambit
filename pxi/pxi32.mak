#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Makefile for PXI for Borland C++
#

.AUTODEPEND

# The following directories must be set or overriden for the target setup
WXDIR = C:\WX2
BCCDIR = C:\BORLAND\BCC55
	
.path.cc = .;../sources/base;../sources/math

SOURCE_SUFFIX = .cc
OBJECT_SUFFIX = .obj

# Define __BCC55__ if using Borland 5.5
EXTRACPPFLAGS = -D__WXMSW__ -D__BCC55__ -D__WIN95__ -D__WINDOWS__ -I$(BCCDIR)\include -I..\include
EXTRALINKFLAGS = 

SOURCES = axis.cc dlgpxi.cc equdata.cc equtrac.cc expdata.cc gmisc.cc probvect.cc pxi.cc pxicanvs.cc wxmisc.cc gstream.cc garray.cc gblock.cc gtext.cc 

OBJECTS = $(SOURCES:.cc=$(OBJECT_SUFFIX))

CFG = pxi32.cfg
WXLIBDIR = $(WXDIR)\lib
WXLIB = wx32 xpm tiff jpeg winpng zlib
WXINC = -I$(WXDIR)\include

$(SOURCE_SUFFIX).obj:
	bcc32 $(CPPFLAGS) -P -c {$< }

.c.obj:
	bcc32 $(CPPFLAGS) -P- -c {$< }

GUILIBS=$(WXLIB) cw32mti import32 ole2w32


LINKFLAGS= /v /c /aa -L$(WXLIBDIR);$(BCCDIR)\lib $(EXTRALINKFLAGS)
OPT = -Od
DEBUG_FLAGS= -v


CPPFLAGS= $(WXINC) $(EXTRACPPFLAGS) $(OPT) @$(CFG)

all: pxi

pxi:	$(OBJECTS) pxi.res
  ilink32 $(LINKFLAGS) @&&!
c0w32.obj $(OBJECTS)
pxi
nul
$(GUILIBS)

pxi.res
!

pxi.res :      pxi.rc
    brc32 -r -fo.\pxi.res /i$(BCCDIR)\include /i$(WXDIR)\include pxi

clean:
        -erase *.obj
        -erase *.exe
        -erase *.res
        -erase *.map
        -erase *.rws




