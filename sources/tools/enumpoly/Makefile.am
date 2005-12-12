## Process this file with automake to create Makefile.in

noinst_LIBRARIES = libpelican.a libpoly.a

libpelican_a_SOURCES = \
	pelclhpk.cc \
	pelclhpk.h \
	pelclqhl.cc \
	pelclqhl.h \
	pelclyal.cc \
	pelclyal.h \
	pelconv.cc \
	pelconv.h \
	peleval.cc \
	peleval.h \
	pelgennd.cc \
	pelgennd.h \
	pelgmatr.cc \
	pelgmatr.h \
	pelgntyp.h \
	pelhomot.cc \
	pelhomot.h \
	pelpred.cc \
	pelpred.h \
	pelprgen.cc \
	pelprgen.h \
	pelproc.cc \
	pelproc.h \
	pelpscon.h \
	pelpsys.cc \
	pelpsys.h \
	pelqhull.cc \
	pelqhull.h \
	pelsymbl.cc \
	pelsymbl.h \
	pelutils.cc \
	pelutils.h

libpoly_a_SOURCES = \
	gpartltr.cc \
	gpartltr.h \
	gpartltr.imp \
	gpoly.cc \
	gpoly.h \
	gpoly.imp \
	gpolyctr.h \
	gpolylst.cc \
	gpolylst.h \
	gpolylst.imp \
	gsolver.cc \
	gsolver.h \
	gsolver.imp \
	ideal.cc \
	ideal.h \
	ideal.imp \
	ineqsolv.cc \
	ineqsolv.h \
	ineqsolv.imp \
	interval.cc \
	interval.h \
	interval.imp \
	monomial.cc \
	monomial.h \
	monomial.imp \
	pelclass.cc \
	pelclass.h \
	poly.cc \
	poly.h \
	poly.imp \
	prepoly.cc \
	prepoly.h \
	quiksolv.cc \
	quiksolv.h \
	quiksolv.imp \
	rectangl.cc \
	rectangl.h \
	rectangl.imp

bin_PROGRAMS = gambit-efg-enumpoly gambit-nfg-enumpoly

AM_CPPFLAGS = -I$(top_srcdir)/sources

gambit_nfg_enumpoly_SOURCES = \
	complex.cc complex.h \
	double.cc double.h \
	gtree.h gtree.imp \
	linrcomb.cc linrcomb.h linrcomb.imp \
	nfgensup.cc nfgensup.h \
	odometer.cc odometer.h \
	nfgpoly.cc

gambit_nfg_enumpoly_LDADD = \
	libpoly.a libpelican.a \
	$(top_builddir)/sources/libgambit/libgambit.la

gambit_efg_enumpoly_SOURCES = \
	behavextend.cc behavextend.h \
	complex.cc complex.h \
	double.cc double.h \
	gtree.h gtree.imp \
	linrcomb.cc linrcomb.h linrcomb.imp \
	efgensup.cc efgensup.h \
	gnarray.h gnarray.imp \
	sfg.cc sfg.h sfstrat.cc sfstrat.h \
	odometer.cc odometer.h \
	efgpoly.cc

gambit_efg_enumpoly_LDADD = \
	libpoly.a libpelican.a \
	$(top_builddir)/sources/libgambit/libgambit.la

