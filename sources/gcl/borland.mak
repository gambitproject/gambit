#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Makefile for Borland C++ 5.5 for GCL module
#

.AUTODEPEND

!include ..\makedef.bcc

EXTRACPPFLAGS = -v -I$(BCCDIR)\include -I.. -D__BCC55__ -DVERSION=\"0.97.0.1\"
EXTRALINKFLAGS = -Tpe -aa -v -V4.0 -c

libgcl_a_SOURCES = \
	algfunc.cc \
	efgfunc.cc \
	gclinst.cc \
	gcompile.cc \
	gpreproc.cc \
	gsm.cc \
	gsmfunc.cc \
	gsmhash.cc \
	gsmincl.cc \
	gsminstr.cc \
	gsmoper.cc \
	gsmutils.cc \
	listfunc.cc \
	nfgfunc.cc \
	portion.cc \
	solfunc.cc

OBJECTS = $(libgcl_a_SOURCES:.cc=.obj)

CFG = ..\gambit32.cfg

OPT = -Od

.cc.obj:
	bcc32 $(CPPFLAGS) -P -c {$< }

LINKFLAGS= /Tpe /L$(WXLIBDIR);$(BCCDIR)\lib $(EXTRALINKFLAGS)
OPT = -Od
DEBUG_FLAGS= -v

CPPFLAGS= $(WXINC) $(EXTRACPPFLAGS) $(OPT) @$(CFG)

all:  gcllib

gcllib: $(OBJECTS)
        -erase gcl.lib
	tlib gcl /P1024 @&&!
+$(OBJECTS:.obj =.obj +) +$(PERIPH_LIBS:.lib =.lib +)
!

clean:
        -erase *.obj
        -erase *.exe
        -erase *.res
        -erase *.map
        -erase *.rws

