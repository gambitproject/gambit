#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# automake input for base module
#

noinst_LTLIBRARIES = libgambit_base.la

AM_CPPFLAGS = -I$(top_srcdir)/sources

baseincludedir = $(includedir)/gambit-$(VERSION)/gambit/base
baseinclude_HEADERS = \
	base.h \
	garray.h \
	gblock.h \
	glist.h \
	gmap.h \
	gmap.imp \
	gmisc.h \
	gnarray.h \
	gnarray.imp \
	grarray.h \
	grarray.imp \
	grblock.h \
	grblock.imp \
	gstatus.h \
	gtree.h \
	odometer.h

libgambit_base_la_SOURCES = \
	base.h \
	garray.h \
	gblock.h \
	glist.h \
	gmap.h \
	gmap.imp \
	gmisc.cc \
	gmisc.h \
	gnarray.h \
	gnarray.imp \
	grarray.cc \
	grarray.h \
	grarray.imp \
	grblock.h \
	grblock.imp \
	gstatus.h \
	gtree.h \
	odometer.cc \
	odometer.h 

EXTRA_DIST = borland.mak
