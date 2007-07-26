## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit-liap

AM_CPPFLAGS = -I$(top_srcdir)/sources

gambit_liap_SOURCES = \
	funcmin.cc funcmin.h \
	efgliap.cc nfgliap.cc liap.cc

gambit_liap_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la
