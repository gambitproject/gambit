#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Main makefile for Borland C++
#

all:  base math numerical pelican poly game nash gcl gui wxgcl

base:
	cd base
	make -f borland
	cd ..

math:
	cd math
	make -f borland
	cd ..

numerical:
	cd numerical
	make -f borland
	cd ..

pelican:
	cd pelican
	make -f borland
	cd ..

poly:
	cd poly
	make -f borland
	cd ..

game:
	cd game
	make -f borland
	cd ..

nash:
	cd nash
	make -f borland
	cd ..

gcl:
	cd gcl
	make -f borland
	cd ..

gui:
	cd gui
	make -f borland
	cd ..

wxgcl:
	cd wxgcl
	make -f borland
	cd ..

clean:
	cd base
	make -f borland clean
	cd ..\math
	make -f borland clean
	cd ..\numerical
	make -f borland clean
	cd ..\pelican
	make -f borland clean
	cd ..\poly
	make -f borland clean
	cd ..\game
	make -f borland clean
	cd ..\nash
	make -f borland clean
	cd ..\gcl
	make -f borland clean
	cd ..\gui
	make -f borland clean
	cd ..\wxgcl
	make -f borland clean



