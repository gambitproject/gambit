#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Makefile for Borland C++ 5.5 for nash module
#

.AUTODEPEND

!include ..\makedef.bcc

EXTRACPPFLAGS = -v -I$(BCCDIR)\include -I.. -D__BCC55__
EXTRALINKFLAGS = -Tpe -aa -v -V4.0 -c

libnash_a_SOURCES = \
	algutils.cc \
	behavextend.cc \
	behavsol.cc \
	clique.cc \
	efgalleq.cc \
	efgcsum.cc \
	efgpure.cc \
	efgqre.cc \
	eliap.cc \
	enum.cc \
	epolenum.cc \
	lemke.cc \
	lhtab.cc \
	mixedsol.cc \
	nfgalleq.cc \
	nfgcsum.cc \
	nfgpure.cc \
	nfgqre.cc \
	nfgqregrid.cc \
	nliap.cc \
	polenum.cc \
	seqeq.cc \
	seqform.cc \
	simpdiv.cc \
	subsolve.cc

OBJECTS = $(libnash_a_SOURCES:.cc=.obj)

CFG = ..\gambit32.cfg

OPT = -Od

.cc.obj:
	bcc32 $(CPPFLAGS) -P -c {$< }

LINKFLAGS= /Tpe /L$(WXLIBDIR);$(BCCDIR)\lib $(EXTRALINKFLAGS)
OPT = -Od
DEBUG_FLAGS= -v


CPPFLAGS= $(WXINC) $(EXTRACPPFLAGS) $(OPT) @$(CFG)

nash: $(OBJECTS)
        -erase nash.lib
	tlib nash /P1024 @&&!
+$(OBJECTS:.obj =.obj +) +$(PERIPH_LIBS:.lib =.lib +)
!

clean:
        -erase *.obj
        -erase *.exe
        -erase *.res
        -erase *.map
        -erase *.rws

