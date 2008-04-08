SWIGFILES = libgambit.i

#
# Build the Python wrapper.  Use this as a template for building for
# other languages.
#
all:  gambit/libgambit_wrap.cxx
	python setup.py build

gambit/libgambit_wrap.cxx:	${SWIGFILES}
	swig -c++ -python -I.. libgambit.i
	mv libgambit_wrap.cxx libgambit.py gambit


install:	all
	python setup.py install