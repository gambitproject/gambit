## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit

AM_CPPFLAGS = -I$(top_srcdir)/sources

include filelist.in

gambit_LDADD = \
	$(top_builddir)/sources/guishare/libguishare.a \
	$(top_builddir)/sources/nash/libnash.a \
	$(top_builddir)/sources/game/libgame.a \
	$(top_builddir)/sources/poly/libpoly.a \
	$(top_builddir)/sources/pelican/libpelican.a \
	$(top_builddir)/sources/numerical/libnumerical.a \
	$(top_builddir)/sources/math/libmath.a \
	$(top_builddir)/sources/base/libbase.a



