#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Main makefile for Borland C++
#

!include makedef.bcc

all:  base math numerical pelican poly game nash gambit
        
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

libbase_a_OBJECTS = \
	+"garray.obj" \
	+"gblock.obj" \
	+"glist.obj" \
	+"gmisc.obj" \
	+"grarray.obj" \
	+"odometer.obj" 

libmath_a_OBJECTS = \
	+complex.obj \
	+gdpvect.obj \
	+gmatrix.obj \
	+gpvector.obj \
	+gsmatrix.obj \
	+gvector.obj \
	+integer.obj \
	+mathinst.obj \
	+misc.obj \
        +mpfloat.obj \
	+rational.obj
 
libnumerical_a_OBJECTS = \
	+basis.obj \
	+bfs.obj \
	+btableau.obj \
	+gfunc.obj \
	+gfuncmin.obj \
	+lemketab.obj \
	+linrcomb.obj \
	+lpsolve.obj \
	+lptab.obj \
	+ludecomp.obj \
	+tableau.obj \
	+vertenum.obj

libgame_a_OBJECTS = \
	+"game-file.obj" \
	+"sfg.obj" \
	+"sfstrat.obj" \
	+"table-mixed-double.obj" \
	+"table-mixed-mpfloat.obj" \
	+"table-mixed-rational.obj" \ 
	+"table-contingency.obj" \
        +"table-file.obj" \
	+"table-game.obj" \
	+"table-inst.obj" \
	+"table-outcome.obj" \
	+"table-player.obj" \
	+"tree-behav-double.obj" \
	+"tree-behav-mpfloat.obj" \
	+"tree-behav-pure.obj" \
	+"tree-behav-rational.obj" \
	+"tree-contingency.obj" \
	+"tree-file.obj" \
	+"tree-game.obj" \
	+"tree-infoset.obj" \
	+"tree-inst.obj" \
	+"tree-node.obj" \
	+"tree-outcome.obj" \
	+"tree-player.obj" 

libpelican_a_OBJECTS = \
	+pelclhpk.obj \
	+pelclqhl.obj \
	+pelclyal.obj \
	+pelconv.obj \
	+peleval.obj \
	+pelgennd.obj \
	+pelgmatr.obj \
	+pelhomot.obj \
	+pelpred.obj \
	+pelprgen.obj \
	+pelproc.obj \
	+pelpsys.obj \
	+pelqhull.obj \
	+pelsymbl.obj \
	+pelutils.obj

libpoly_a_OBJECTS = \
	+gpartltr.obj \
	+gpoly.obj \
	+gpolylst.obj \
	+gsolver.obj \
	+ideal.obj \
	+ineqsolv.obj \
	+interval.obj \
	+monomial.obj \
	+pelclass.obj \
	+poly.obj \
	+prepoly.obj \
	+quiksolv.obj \
	+rectangl.obj

libnash_a_OBJECTS = \
	+behavextend.obj \
	+clique.obj \
	+efglcp.obj \
	+efgliap.obj \
	+efglogit.obj \
	+efglp.obj \
	+efgpoly.obj \
	+efgpure.obj \
	+lhtab.obj \
        +nfgch.obj \
	+nfglcp.obj \
	+nfgliap.obj \
	+nfglogit.obj \
	+nfglp.obj \
	+nfgmixed.obj \
	+nfgpoly.obj \
	+nfgpure.obj \
	+nfgqregrid.obj \
	+nfgsimpdiv.obj \
        +nfgyamamoto.obj

        
OBJECTS = \
        $(libbase_a_OBJECTS) \
        $(libmath_a_OBJECTS) \
        $(libnumerical_a_OBJECTS) \
        $(libgame_a_OBJECTS) \
        $(libpelican_a_OBJECTS) \
        $(libpoly_a_OBJECTS) \
        $(libnash_a_OBJECTS)


gambit: 
        -erase gambit.lib
	tlib gambit /P1024 @&&!
$(OBJECTS) +$(PERIPH_LIBS:.lib =.lib +)
!

python:
        cd python
        make -f borland 

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
        cd ..
        del *.obj




