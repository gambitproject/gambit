#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Main makefile for Borland C++
#

GAMBIT_TOP = d:\gambit

all:  base math numerical pelican poly game nash gcl gui wxgcl

base:
	cd $(GAMBIT_TOP)\sources\base
	make -f borland

math:
	cd $(GAMBIT_TOP)\sources\math
	make -f borland

numerical:
	cd $(GAMBIT_TOP)\sources\numerical
	make -f borland

pelican:
	cd $(GAMBIT_TOP)\sources\pelican
	make -f borland

poly:
	cd $(GAMBIT_TOP)\sources\poly
	make -f borland

game:
	cd $(GAMBIT_TOP)\sources\game
	make -f borland

nash:
	cd $(GAMBIT_TOP)\sources\nash
	make -f borland

gcl:
	cd $(GAMBIT_TOP)\sources\gcl
	make -f borland

gui:
	cd $(GAMBIT_TOP)\sources\gui
	make -f borland

wxgcl:
	cd $(GAMBIT_TOP)\sources\wxgcl
	make -f borland

clean:
	cd $(GAMBIT_TOP)\sources\base
	make -f borland clean
	cd $(GAMBIT_TOP)\sources\math
	make -f borland clean
	cd $(GAMBIT_TOP)\sources\numerical
	make -f borland clean
	cd $(GAMBIT_TOP)\sources\pelican
	make -f borland clean
	cd $(GAMBIT_TOP)\sources\poly
	make -f borland clean
	cd $(GAMBIT_TOP)\sources\game
	make -f borland clean
	cd $(GAMBIT_TOP)\sources\nash
	make -f borland clean
	cd $(GAMBIT_TOP)\sources\gcl
	make -f borland clean
	cd $(GAMBIT_TOP)\sources\gui
	make -f borland clean
	cd $(GAMBIT_TOP)\sources\wxgcl
	make -f borland clean



