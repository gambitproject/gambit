## Process this file with automake to create Makefile.in

SUBDIRS = base math numerical game pelican poly python nash

EXTRA_DIST = borland.mak gambit32.cfg

lib_LTLIBRARIES = libgambit.la

libgambit_la_SOURCES = dummy.c

libgambit_la_LDFLAGS = -no-undefined -version-info 0:0:0

libgambit_la_LIBADD = \
	base/libgambit_base.la \
	math/libgambit_math.la \
	numerical/libgambit_numerical.la \
	game/libgambit_game.la \
	pelican/libgambit_pelican.la \
	poly/libgambit_poly.la \
	nash/libgambit_nash.la 
