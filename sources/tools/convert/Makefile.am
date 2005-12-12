## Process this file with automake to create Makefile.in

bin_PROGRAMS = gambit-nfg2html gambit-nfg2tex

AM_CPPFLAGS = -I$(top_srcdir)/sources

gambit_nfg2html_SOURCES = nfg2html.cc

gambit_nfg2html_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la


gambit_nfg2tex_SOURCES = nfg2tex.cc

gambit_nfg2tex_LDADD = \
	$(top_builddir)/sources/libgambit/libgambit.la

