## Process this file with automake to create Makefile.in

lib_LTLIBRARIES = libgambit.la

libgambit_la_SOURCES = \
	integer.cc integer.h \
	rational.cc rational.h \
	array.h list.h map.h \
	vector.cc vector.h vector.imp \
	pvector.cc pvector.h pvector.imp \
	dvector.cc dvector.h dvector.imp \
	recarray.h \
	matrix.cc matrix.h matrix.imp \
	sqmatrix.cc sqmatrix.h sqmatrix.imp \
	number.h \
	game.cc game.h \
	behav.cc behav.h behav.imp \
	behavitr.cc behavitr.h \
	behavspt.cc behavspt.h \
	mixed.cc mixed.h mixed.imp \
	stratitr.cc stratitr.h \
	stratspt.cc stratspt.h \
	file.cc \
	libgambit.h

libgambitincludedir = $(includedir)/libgambit
libgambitinclude_HEADERS = \
	integer.h rational.h \
	array.h list.h map.h \
	vector.h vector.imp \
	pvector.h pvector.imp \
	dvector.h dvector.imp \
	recarray.h
	matrix.h matrix.imp \
	sqmatrix.h sqmatrix.imp \
	number.h \
	game.h \
	behav.h behav.imp \
	behavitr.h behavspt.h \
	mixed.h mixed.imp \
	stratitr.h stratspt.h \
	libgambit.h

libgambit_la_LDFLAGS = -no-undefined -version-info 0:0:0