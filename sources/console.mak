#
# FILE: console.mak -- Makefile for GCL console version
#
# $Id$
#
# NB: The accompanying console.cfg file sets flags for a console application
# for compilation.  You should do a "make clean" prior to building the
# GCL console version.
#

.AUTODEPEND

# The following directories must be set or overriden for the target setup
BCCDIR = C:\BORLAND\BCC55
BISON = C:\GAMBIT\BISONDOS\BISON
	
SOURCE_SUFFIX = .cc
OBJECT_SUFFIX = .obj

# Define __BCC55__ if using Borland 5.5
EXTRACPPFLAGS = -D__BCC55__ -I$(BCCDIR)\include -I..\include -Ibase -Imath
EXTRALINKFLAGS = 

!include make.filelist

CFG = console.cfg

.path.cc = .;base;math

$(SOURCE_SUFFIX).obj:
	bcc32 $(CPPFLAGS) -P -c {$< }

.c.obj:
	bcc32 $(CPPFLAGS) -P- -c {$< }

LINKFLAGS= /c /ap /L$(BCCDIR)\lib $(EXTRALINKFLAGS)
OPT = -Od
DEBUG_FLAGS=


CPPFLAGS= $(WXINC) $(EXTRACPPFLAGS) $(OPT) @$(CFG)

all: gcl

#
# This is screwy; when I used the linker explicitly, I had problems with
# stdin always returning eof.  When I build the program like this, it
# works fine.  So I'll leave this hack in for now.  The explicit
# gcl.obj in the list of objects is there to make sure the resulting
# executable is gcl.exe. -- TLT
#
gcl:   $(MSWGCL_OBJECTS)
  bcc32 /L$(BCCDIR)\lib gcl.obj $(MSWGCL_OBJECTS)

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




