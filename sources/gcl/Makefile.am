## Process this file with automake to create Makefile.in

noinst_LIBRARIES = libgcl.a

bin_PROGRAMS = gcl

AM_CPPFLAGS = -I$(top_srcdir)/sources

libgcl_a_SOURCES = \
	algfunc.cc \
	command.h \
	efgfunc.cc \
	gclinst.cc \
	gclstatus.h \
	gcmdline.h \
	gcompile.cc \
	gpreproc.cc \
	gpreproc.h \
	gsm.cc \
	gsm.h \
	gsmconsole.h \
	gsmfunc.cc \
	gsmfunc.h \
	gsmhash.cc \
	gsmhash.h \
	gsmincl.cc \
	gsmincl.h \
	gsminstr.cc \
	gsminstr.h \
	gsmoper.cc \
	gsmoper.h \
	gsmutils.cc \
	listfunc.cc \
	listfunc.h \
	nfgfunc.cc \
	portion.cc \
	portion.h \
	solfunc.cc

gcl_SOURCES = gcl.cc gclstatus.cc gcmdline.cc 

gcl_LDADD = \
	libgcl.a \
	$(top_builddir)/sources/nash/libnash.a \
	$(top_builddir)/sources/numerical/libnumerical.a \
	$(top_builddir)/sources/game/libgame.a \
	$(top_builddir)/sources/poly/libpoly.a \
	$(top_builddir)/sources/pelican/libpelican.a \
	$(top_builddir)/sources/math/libmath.a \
	$(top_builddir)/sources/base/libbase.a

gcompile.cc:  gcompile.yy
	bison -p gcl_yy -o gcompile.cc gcompile.yy

EXTRA_DIST = borland.mak gcompile.yy

