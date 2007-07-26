SWIGFILES = gambit.i

#
# Build the Python wrapper.  Use this as a template for building for
# other languages.
#
all:  gambit_python.cc

gambit_python.cc:	${SWIGFILES}
	swig -c++ -python -I.. gambit.i

