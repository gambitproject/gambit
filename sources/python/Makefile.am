## Process this file with automake to create Makefile.in

SWIGFILES = \
	behav.i \
	contingency.i \
	game.i \
	gbt.i \
	infoset.i \
	mixed.i \
	nash.i \
	node.i \
	outcome.i \
	player.i \
	rational.i \
	typemaps.i

EXTRA_DIST = \
	borland.mak \
	gambit.py \
	gbt.py \
	gbt_wrap_python.cc \
	${SWIGFILES}

#
# Build the Python wrapper.  Use this as a template for building for
# other languages.  Note that no support for installation exists in this
# makefile yet.
#

gbt_wrap_python.cc:	${SWIGFILES}
	swig -c++ -python -o gbt_wrap_python.cc gbt.i

gbt_wrap_python.o:	gbt_wrap_python.cc
	g++ -c ${PYTHON_INCLUDES} gbt_wrap_python.cc

python:	gbt_wrap_python.o
	g++ -shared gbt_wrap_python.o -L${libdir} -lgambit -o _gbt.so
