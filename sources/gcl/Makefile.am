## Process this file with automake to create Makefile.in

noinst_LIBRARIES = libgcl.a

bin_PROGRAMS = gcl

AM_CPPFLAGS = -I$(top_srcdir)/sources

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

gcl_SOURCES = gcl.cc gclstatus.cc gcmdline.cc gio.cc

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