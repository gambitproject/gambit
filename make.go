# $Id$
# Final level of makefiles--this actually builds the code.  Note that you
# must set DEPEND=True for the depend_* targets.
# This should not be edited by the user
# All the site dependent settings are in make.site_$(GUI)_$(HOST)

VPATH= %.o . % ../
.SUFFIXES:
.SUFFIXES: .h .cc .o .yy

#Nasty SCCS hack to work around HPUX get inability to specify output dir
#%:: SCCS/s.%
#	rm -f $@
#	$(GET) -p $< > $@
#%:: s.%
#	rm -f $@
#	$(GET) -p $< > $@

.cc.o:
	$(CC) $(CFLAGS) -c $<
.yy.o:
	bison++ -d -h ../$(*F).h -o ../$(*F).cc $<
	$(CC) $(CFLAGS) -c ../$(*F).cc

CC = g++
CFLAGS = -Wall -fno-implicit-templates -I../

	# Site & platform specific information
ifndef DEPEND
	include ../make.site_$(GUI)_$(HOST)
else
	include make.site_$(GUI)_$(HOST)
endif
	# Generic WxWin information
	WXLIBDIR = -L$(WXDIR)/lib
	WXINCDIR = -I$(WXDIR)/include/base -I$(WXDIR)/include/x 
	WXLIB = -lwx_$(GUI)_$(HOST)

	# Combine the site dependent and independent data

	LIBDIR = $(WXLIBDIR) $(XLIBDIR)
	INCDIR = $(WXINCDIR) $(XINCDIR)

	GUILIBS = $(WXLIB) $(XLIB)


GLIB_SOURCES = gmisc.cc gambitio.cc garray.cc gblock.cc gstring.cc integer.cc \
		rational.cc gnulib.cc gvector.cc gpvector.cc gdpvect.cc \
		grarray.cc gmatrix.cc gclsig.cc glpsolve.cc bfs.cc gwatch.cc \
		gfunc.cc gclstats.cc glist.cc subsolve.cc gfuncmin.cc

GLIB_OBJECTS = gmisc.o gambitio.o garray.o gblock.o gstring.o integer.o \
		rational.o gnulib.o gvector.o gpvector.o gdpvect.o \
		grarray.o gmatrix.o glpsolve.o bfs.o gwatch.o \
		gfunc.o gclstats.o glist.o subsolve.o gfuncmin.o

GAME_SOURCES = readnfg.cc readefg.cc nfg.cc nfgdbl.cc nfgrat.cc nfgutils.cc \
		efg.cc efgdbl.cc efgrat.cc nfstrat.cc efgnfgr.cc efgutils.cc \
		efstrat.cc efgnfgd.cc efdom.cc

GAME_OBJECTS = readnfg.o readefg.o nfg.o nfgdbl.o nfgrat.o nfgutils.o \
		efg.o efgdbl.o efgrat.o nfstrat.o efgnfgr.o efgutils.o \
		efstrat.o efgnfgd.o efdom.o 

EALG_SOURCES =  egobit.cc eliap.cc seqform.cc efgcsum.cc \
		psnesub.cc efgpure.cc lemkesub.cc liapsub.cc simpsub.cc \
		enumsub.cc csumsub.cc behavsol.cc

EALG_OBJECTS =  egobit.o eliap.o seqform.o efgcsum.o \
		psnesub.o efgpure.o lemkesub.o liapsub.cc simpsub.o \
		enumsub.o csumsub.o behavsol.o

NALG_SOURCES = lemke.cc nliap.cc ngobit.cc enum.cc simpdiv.cc tableau.cc \
		ludecomp.cc nfgpure.cc lhtab.cc lemketab.cc grid.cc nfgcsum.cc\
		lpsolve.cc nfdom.cc vertenum.cc mixedsol.cc

NALG_OBJECTS = lemke.o nliap.o ngobit.o enum.o simpdiv.o tableau.o ludecomp.o \
		nfgpure.o lhtab.o lemketab.o grid.o nfgcsum.o lpsolve.o nfdom.o\
		vertenum.o mixedsol.o

AGCL_SOURCES = gcompile.cc gsm.cc gsmfunc.cc gsmoper.cc gsmhash.cc gclmath.cc \
               gsminstr.cc portion.cc nfgfunc.cc efgfunc.cc listfunc.cc \
               algfunc.cc gcl.cc gclsig.cc 

AGCL_OBJECTS = gcompile.o gsm.o gsmfunc.o gsmoper.o gsmhash.o gclmath.o \
               gsminstr.o portion.o nfgfunc.o efgfunc.o listfunc.o \
               algfunc.o gcl.o gclsig.o
# Gui stuff for the extensive form
EGUI_SOURCES = extshow.cc treewin.cc btreewn.cc btreewn1.cc treedraw.cc \
               twflash.cc extsoln.cc btreewni.cc bextshow.cc outcomed.cc \
		efgsolng.cc bsolnsf.cc
		
EGUI_OBJECTS = extshow.o treewin.o btreewn.o btreewn1.o treedraw.o \
               twflash.o extsoln.o btreewni.o bextshow.o outcomed.o \
		efgsolng.o bsolnsf.o

# Gui stuff for the normal form
NGUI_SOURCES = normshow.cc bnormshw.cc normsoln.cc msolnsf.cc
NGUI_OBJECTS = normshow.o bnormshw.o normsoln.o msolnsf.o
# Gui stuff used in both the normal and extensive forms
BGUI_SOURCES = wxmisc.cc wximpl.cc gambdraw.cc wxio.cc spread.cc spreadim.cc \
              gambit.cc accels.cc algdlgs.cc wxstatus.cc efgnfgi.cc general.cc
BGUI_OBJECTS = wxmisc.o wximpl.o gambdraw.o wxio.o spread.o spreadim.o \
              gambit.o accels.o algdlgs.o wxstatus.o efgnfgi.o general.o

# All the code used in the GCL target
GCL_SOURCES = $(GLIB_SOURCES) $(GAME_SOURCES) $(EALG_SOURCES) $(NALG_SOURCES)\
              $(AGCL_SOURCES)
GCL_OBJECTS = $(GLIB_OBJECTS) $(GAME_OBJECTS) $(EALG_OBJECTS) $(NALG_OBJECTS)\
              $(AGCL_OBJECTS)

# All the code used in the GUI target
GUI_SOURCES = $(GLIB_SOURCES) $(GAME_SOURCES) $(EALG_SOURCES) $(NALG_SOURCES)\
              $(EGUI_SOURCES) $(NGUI_SOURCES) $(BGUI_SOURCES)
GUI_OBJECTS = $(GLIB_OBJECTS) $(GAME_OBJECTS) $(EALG_OBJECTS) $(NALG_OBJECTS)\
              $(EGUI_OBJECTS) $(NGUI_OBJECTS) $(BGUI_OBJECTS)

all:
	@echo Please specify a target: either GUI or GCL
	@echo Edit the appropriate directory info for GUI compilation

gcl_$(HOST):	$(GCL_OBJECTS)
	$(CC) $(GCL_OBJECTS) -o ../gcl_$(HOST) -lm

gui:	
	$(MAKE) -f ../make.go CFLAGS='$(CFLAGS) $(INCDIR) $(WXGUI)  \
			-DGRATIONAL -DNO_GIO ' \
			GUI=$(GUI) HOST=$(HOST) gui_$(GUI)_$(HOST)

gui_$(GUI)_$(HOST): $(GUI_OBJECTS)
	$(CC) $(GUI_OBJECTS) $(LIBDIR) $(GUILIBS) -o ../gui_$(GUI)_$(HOST)

depend_gcl:
	makedepend -f make.go $(GCL_SOURCES) 
depend_gui:
	makedepend -f make.go -I./wx_fake $(GUI_SOURCES) 
clean:
	rm -f *.o core *~ *.bak gambgui gcl *.zip

$(OBJDIR):
	mkdir $(OBJDIR)
# DO NOT DELETE THIS LINE -- make depend depends on it.

gmisc.o: gmisc.h rational.h integer.h /usr/include/math.h
gmisc.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
gmisc.o: /usr/include/stdio.h /usr/include/stdlib.h
gmisc.o: /usr/include/sys/stdtypes.h /usr/include/limits.h
gmisc.o: /usr/include/ctype.h gambitio.h gstring.h /usr/include/string.h
gambitio.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
garray.o: garray.imp /usr/include/stdlib.h /usr/include/sys/stdtypes.h
garray.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
garray.o: gmisc.h gstring.h /usr/include/string.h rational.h integer.h
garray.o: /usr/include/math.h /usr/include/floatingpoint.h
garray.o: /usr/include/sys/ieeefp.h
gblock.o: gmisc.h gblock.imp /usr/include/stdlib.h
gblock.o: /usr/include/sys/stdtypes.h /usr/include/assert.h gblock.h garray.h
gblock.o: gambitio.h /usr/include/stdio.h gstring.h /usr/include/string.h
gblock.o: rational.h integer.h /usr/include/math.h
gblock.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h garray.imp
gstring.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
gstring.o: /usr/include/assert.h /usr/include/ctype.h gambitio.h
gstring.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
integer.o: gambitio.h /usr/include/stdio.h gmisc.h integer.h gnulib.h
integer.o: /usr/include/stddef.h /usr/include/sys/stdtypes.h
integer.o: /usr/include/stdlib.h /usr/include/string.h /usr/include/memory.h
integer.o: /usr/include/errno.h /usr/include/sys/errno.h /usr/include/fcntl.h
integer.o: /usr/include/sys/fcntlcom.h /usr/include/sys/stat.h
integer.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
integer.o: /usr/include/math.h /usr/include/floatingpoint.h
integer.o: /usr/include/sys/ieeefp.h /usr/include/ctype.h float.h
integer.o: /usr/include/limits.h /usr/include/assert.h
rational.o: gambitio.h /usr/include/stdio.h gmisc.h rational.h integer.h
rational.o: /usr/include/math.h /usr/include/floatingpoint.h
rational.o: /usr/include/sys/ieeefp.h gnulib.h /usr/include/stddef.h
rational.o: /usr/include/sys/stdtypes.h /usr/include/stdlib.h
rational.o: /usr/include/string.h /usr/include/memory.h /usr/include/errno.h
rational.o: /usr/include/sys/errno.h /usr/include/fcntl.h
rational.o: /usr/include/sys/fcntlcom.h /usr/include/sys/stat.h
rational.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
rational.o: /usr/include/values.h float.h /usr/include/assert.h
rational.o: /usr/include/ctype.h
gnulib.o: /usr/include/assert.h /usr/include/values.h gnulib.h
gnulib.o: /usr/include/stddef.h /usr/include/sys/stdtypes.h
gnulib.o: /usr/include/stdlib.h /usr/include/string.h /usr/include/memory.h
gnulib.o: /usr/include/stdio.h /usr/include/errno.h /usr/include/sys/errno.h
gnulib.o: /usr/include/fcntl.h /usr/include/sys/fcntlcom.h
gnulib.o: /usr/include/sys/stat.h /usr/include/sys/types.h
gnulib.o: /usr/include/sys/sysmacros.h /usr/include/math.h
gnulib.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
gvector.o: gvector.imp gvector.h gmisc.h gambitio.h /usr/include/stdio.h
gvector.o: garray.h /usr/include/stdlib.h /usr/include/sys/stdtypes.h
gvector.o: /usr/include/assert.h rational.h integer.h /usr/include/math.h
gvector.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
gpvector.o: gpvector.imp gpvector.h gvector.h gmisc.h gambitio.h
gpvector.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
gpvector.o: /usr/include/sys/stdtypes.h /usr/include/assert.h rational.h
gpvector.o: integer.h /usr/include/math.h /usr/include/floatingpoint.h
gpvector.o: /usr/include/sys/ieeefp.h
gdpvect.o: gdpvect.imp gdpvect.h gpvector.h gvector.h gmisc.h gambitio.h
gdpvect.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
gdpvect.o: /usr/include/sys/stdtypes.h /usr/include/assert.h rational.h
gdpvect.o: integer.h /usr/include/math.h /usr/include/floatingpoint.h
gdpvect.o: /usr/include/sys/ieeefp.h
grarray.o: grarray.imp grarray.h gambitio.h /usr/include/stdio.h gmisc.h
grarray.o: /usr/include/assert.h garray.h /usr/include/stdlib.h
grarray.o: /usr/include/sys/stdtypes.h rational.h integer.h
grarray.o: /usr/include/math.h /usr/include/floatingpoint.h
grarray.o: /usr/include/sys/ieeefp.h
gmatrix.o: gmatrix.imp gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h
gmatrix.o: grarray.h gblock.h /usr/include/stdlib.h
gmatrix.o: /usr/include/sys/stdtypes.h /usr/include/assert.h garray.h
gmatrix.o: gvector.h rational.h integer.h /usr/include/math.h
gmatrix.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
gclsig.o: gsignal.h gmisc.h /usr/include/signal.h /usr/include/sys/signal.h
gclsig.o: /usr/include/vm/faultcode.h
glpsolve.o: glpsolve.imp glpsolve.h gtableau.h /usr/include/assert.h gmisc.h
glpsolve.o: gambitio.h /usr/include/stdio.h gmatrix.h grarray.h gblock.h
glpsolve.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h garray.h
glpsolve.o: gvector.h gmap.h bfs.h rational.h integer.h /usr/include/math.h
glpsolve.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
glpsolve.o: gtableau.imp
bfs.o: rational.h integer.h /usr/include/math.h /usr/include/floatingpoint.h
bfs.o: /usr/include/sys/ieeefp.h garray.imp /usr/include/stdlib.h
bfs.o: /usr/include/sys/stdtypes.h /usr/include/assert.h garray.h gambitio.h
bfs.o: /usr/include/stdio.h gmisc.h gblock.imp gblock.h glist.imp glist.h
bfs.o: gmap.imp gmap.h /usr/include/string.h bfs.h
gwatch.o: gwatch.h /usr/include/stdio.h /usr/include/math.h
gwatch.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
gfunc.o: gfunc.h gmisc.h
gclstats.o: gstatus.h gsignal.h gmisc.h gprogres.h gambitio.h
gclstats.o: /usr/include/stdio.h gstring.h /usr/include/string.h
gclstats.o: /usr/include/sys/stdtypes.h /usr/include/signal.h
gclstats.o: /usr/include/sys/signal.h /usr/include/vm/faultcode.h
glist.o: gmisc.h glist.imp glist.h gambitio.h /usr/include/stdio.h
glist.o: /usr/include/assert.h gstring.h /usr/include/string.h
glist.o: /usr/include/sys/stdtypes.h rational.h integer.h /usr/include/math.h
glist.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h garray.h
glist.o: /usr/include/stdlib.h gblock.h
subsolve.o: efg.h gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
subsolve.o: gblock.h /usr/include/stdlib.h /usr/include/assert.h garray.h
subsolve.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h
subsolve.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h
subsolve.o: rational.h integer.h /usr/include/math.h
subsolve.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h node.h
subsolve.o: outcome.h efgutils.h nfg.h nfstrat.h gwatch.h subsolve.imp
subsolve.o: subsolve.h behavsol.h garray.imp glist.imp
gfuncmin.o: /usr/include/math.h /usr/include/floatingpoint.h
gfuncmin.o: /usr/include/sys/ieeefp.h gfunc.h gmisc.h gvector.h gambitio.h
gfuncmin.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
gfuncmin.o: /usr/include/sys/stdtypes.h /usr/include/assert.h gmatrix.h
gfuncmin.o: grarray.h gblock.h
nfg.o: /usr/include/assert.h nfg.h garray.h /usr/include/stdlib.h
nfg.o: /usr/include/sys/stdtypes.h gambitio.h /usr/include/stdio.h gmisc.h
nfg.o: gstring.h /usr/include/string.h gpvector.h gvector.h nfstrat.h
nfg.o: gblock.h nfplayer.h mixed.h garray.imp gblock.imp
nfgdbl.o: rational.h integer.h /usr/include/math.h
nfgdbl.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h nfg.h
nfgdbl.o: garray.h /usr/include/stdlib.h /usr/include/sys/stdtypes.h
nfgdbl.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
nfgdbl.o: gstring.h /usr/include/string.h gpvector.h gvector.h nfg.imp
nfgdbl.o: nfplayer.h nfstrat.h gblock.h nfgiter.h mixed.h nfgiter.imp
nfgdbl.o: nfgciter.imp nfgciter.h readnfg.imp glist.h mixedsol.imp mixedsol.h
nfgdbl.o: garray.imp glist.imp
nfgrat.o: rational.h integer.h /usr/include/math.h
nfgrat.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h nfg.h
nfgrat.o: garray.h /usr/include/stdlib.h /usr/include/sys/stdtypes.h
nfgrat.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
nfgrat.o: gstring.h /usr/include/string.h gpvector.h gvector.h nfg.imp
nfgrat.o: nfplayer.h nfstrat.h gblock.h nfgiter.h mixed.h nfgiter.imp
nfgrat.o: nfgciter.imp nfgciter.h readnfg.imp glist.h mixedsol.imp mixedsol.h
nfgrat.o: garray.imp glist.imp
nfgutils.o: gmisc.h nfg.h garray.h /usr/include/stdlib.h
nfgutils.o: /usr/include/sys/stdtypes.h /usr/include/assert.h gambitio.h
nfgutils.o: /usr/include/stdio.h gstring.h /usr/include/string.h gpvector.h
nfgutils.o: gvector.h rational.h integer.h /usr/include/math.h
nfgutils.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
efg.o: garray.h /usr/include/stdlib.h /usr/include/sys/stdtypes.h
efg.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
efg.o: rational.h integer.h /usr/include/math.h /usr/include/floatingpoint.h
efg.o: /usr/include/sys/ieeefp.h garray.imp gblock.imp gblock.h glist.imp
efg.o: glist.h efg.h gstring.h /usr/include/string.h gpvector.h gvector.h
efg.o: behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h outcome.h
efg.o: efgutils.h
efgdbl.o: rational.h integer.h /usr/include/math.h
efgdbl.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h glist.h
efgdbl.o: gambitio.h /usr/include/stdio.h gmisc.h efg.imp efg.h gstring.h
efgdbl.o: /usr/include/string.h /usr/include/sys/stdtypes.h gblock.h
efgdbl.o: /usr/include/stdlib.h /usr/include/assert.h garray.h gpvector.h
efgdbl.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h
efgdbl.o: outcome.h efgutils.h tnode.h behavsol.imp behavsol.h efgiter.imp
efgdbl.o: efgciter.h efgiter.h efgciter.imp readefg.imp gstack.h glist.imp
efgrat.o: rational.h integer.h /usr/include/math.h
efgrat.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h glist.h
efgrat.o: gambitio.h /usr/include/stdio.h gmisc.h efg.imp efg.h gstring.h
efgrat.o: /usr/include/string.h /usr/include/sys/stdtypes.h gblock.h
efgrat.o: /usr/include/stdlib.h /usr/include/assert.h garray.h gpvector.h
efgrat.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h
efgrat.o: outcome.h efgutils.h tnode.h behavsol.imp behavsol.h efgiter.imp
efgrat.o: efgciter.h efgiter.h efgciter.imp readefg.imp gstack.h glist.imp
nfstrat.o: nfstrat.h gstring.h /usr/include/string.h
nfstrat.o: /usr/include/sys/stdtypes.h gblock.h /usr/include/stdlib.h
nfstrat.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
nfstrat.o: gmisc.h nfplayer.h nfg.h gpvector.h gvector.h
efgnfgr.o: efgnfg.imp tnode.h efg.h gstring.h /usr/include/string.h
efgnfgr.o: /usr/include/sys/stdtypes.h gblock.h /usr/include/stdlib.h
efgnfgr.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
efgnfgr.o: gmisc.h glist.h gpvector.h gvector.h behav.h efstrat.h gdpvect.h
efgnfgr.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h
efgnfgr.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h node.h
efgnfgr.o: outcome.h nfg.h nfplayer.h nfstrat.h nfgiter.h nfgciter.h mixed.h
efgnfgr.o: lexicon.h
efgutils.o: efgutils.h efg.h gstring.h /usr/include/string.h
efgutils.o: /usr/include/sys/stdtypes.h gblock.h /usr/include/stdlib.h
efgutils.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
efgutils.o: gmisc.h glist.h gpvector.h gvector.h behav.h efstrat.h gdpvect.h
efgutils.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h
efgutils.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h node.h
efgutils.o: outcome.h
efstrat.o: efg.h gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
efstrat.o: gblock.h /usr/include/stdlib.h /usr/include/assert.h garray.h
efstrat.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h
efstrat.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h
efstrat.o: rational.h integer.h /usr/include/math.h
efstrat.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h node.h
efstrat.o: outcome.h
efgnfgd.o: efgnfg.imp tnode.h efg.h gstring.h /usr/include/string.h
efgnfgd.o: /usr/include/sys/stdtypes.h gblock.h /usr/include/stdlib.h
efgnfgd.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
efgnfgd.o: gmisc.h glist.h gpvector.h gvector.h behav.h efstrat.h gdpvect.h
efgnfgd.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h
efgnfgd.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h node.h
efgnfgd.o: outcome.h nfg.h nfplayer.h nfstrat.h nfgiter.h nfgciter.h mixed.h
efgnfgd.o: lexicon.h
efdom.o: efg.h gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
efdom.o: gblock.h /usr/include/stdlib.h /usr/include/assert.h garray.h
efdom.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h gvector.h
efdom.o: behav.h efstrat.h gdpvect.h efplayer.h infoset.h rational.h
efdom.o: integer.h /usr/include/math.h /usr/include/floatingpoint.h
efdom.o: /usr/include/sys/ieeefp.h node.h outcome.h efgciter.h
egobit.o: /usr/include/math.h /usr/include/floatingpoint.h
egobit.o: /usr/include/sys/ieeefp.h egobit.h gambitio.h /usr/include/stdio.h
egobit.o: gmisc.h gstatus.h gsignal.h gprogres.h gstring.h
egobit.o: /usr/include/string.h /usr/include/sys/stdtypes.h glist.h efg.h
egobit.o: gblock.h /usr/include/stdlib.h /usr/include/assert.h garray.h
egobit.o: gpvector.h gvector.h behav.h efstrat.h gdpvect.h efplayer.h
egobit.o: infoset.h rational.h integer.h node.h outcome.h behavsol.h gfunc.h
egobit.o: gmatrix.h grarray.h
eliap.o: eliap.h gambitio.h /usr/include/stdio.h gmisc.h gstatus.h gsignal.h
eliap.o: gprogres.h gstring.h /usr/include/string.h
eliap.o: /usr/include/sys/stdtypes.h glist.h efg.h gblock.h
eliap.o: /usr/include/stdlib.h /usr/include/assert.h garray.h gpvector.h
eliap.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h
eliap.o: rational.h integer.h /usr/include/math.h
eliap.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h node.h
eliap.o: outcome.h behavsol.h subsolve.h nfg.h gfunc.h gmatrix.h grarray.h
seqform.o: seqform.imp seqform.h efg.h gstring.h /usr/include/string.h
seqform.o: /usr/include/sys/stdtypes.h gblock.h /usr/include/stdlib.h
seqform.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
seqform.o: gmisc.h glist.h gpvector.h gvector.h behav.h efstrat.h gdpvect.h
seqform.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h
seqform.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h node.h
seqform.o: outcome.h gmatrix.h grarray.h lemketab.h tableau.h ludecomp.h
seqform.o: gwatch.h bfs.h gmap.h gstatus.h gsignal.h gprogres.h behavsol.h
seqform.o: subsolve.h nfg.h
efgcsum.o: efgcsum.imp gwatch.h gpvector.h gvector.h gmisc.h gambitio.h
efgcsum.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
efgcsum.o: /usr/include/sys/stdtypes.h /usr/include/assert.h efgcsum.h efg.h
efgcsum.o: gstring.h /usr/include/string.h gblock.h glist.h behav.h efstrat.h
efgcsum.o: gdpvect.h efplayer.h infoset.h rational.h integer.h
efgcsum.o: /usr/include/math.h /usr/include/floatingpoint.h
efgcsum.o: /usr/include/sys/ieeefp.h node.h outcome.h gstatus.h gsignal.h
efgcsum.o: gprogres.h tableau.h ludecomp.h gmatrix.h grarray.h bfs.h gmap.h
efgcsum.o: lpsolve.h behavsol.h subsolve.h nfg.h
psnesub.o: rational.h integer.h /usr/include/math.h
psnesub.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h psnesub.imp
psnesub.o: psnesub.h subsolve.h efg.h gstring.h /usr/include/string.h
psnesub.o: /usr/include/sys/stdtypes.h gblock.h /usr/include/stdlib.h
psnesub.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
psnesub.o: gmisc.h glist.h gpvector.h gvector.h behav.h efstrat.h gdpvect.h
psnesub.o: efplayer.h infoset.h node.h outcome.h nfg.h behavsol.h nfgpure.h
psnesub.o: mixed.h nfstrat.h mixedsol.h
efgpure.o: efgpure.imp glist.h gambitio.h /usr/include/stdio.h gmisc.h efg.h
efgpure.o: gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
efgpure.o: gblock.h /usr/include/stdlib.h /usr/include/assert.h garray.h
efgpure.o: gpvector.h gvector.h behav.h efstrat.h gdpvect.h efplayer.h
efgpure.o: infoset.h rational.h integer.h /usr/include/math.h
efgpure.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h node.h
efgpure.o: outcome.h efgiter.h efgciter.h behavsol.h efgpure.h subsolve.h
efgpure.o: nfg.h
lemkesub.o: rational.h integer.h /usr/include/math.h
lemkesub.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
lemkesub.o: lemkesub.imp lemkesub.h lemke.h nfg.h garray.h
lemkesub.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
lemkesub.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
lemkesub.o: gstring.h /usr/include/string.h gpvector.h gvector.h mixedsol.h
lemkesub.o: mixed.h nfstrat.h gblock.h glist.h lhtab.h lemketab.h tableau.h
lemkesub.o: ludecomp.h gmatrix.h grarray.h gwatch.h bfs.h gmap.h gstatus.h
lemkesub.o: gsignal.h gprogres.h subsolve.h efg.h behav.h efstrat.h gdpvect.h
lemkesub.o: efplayer.h infoset.h node.h outcome.h behavsol.h
liapsub.o: liapsub.h nliap.h gambitio.h /usr/include/stdio.h gmisc.h
liapsub.o: gstatus.h gsignal.h gprogres.h gstring.h /usr/include/string.h
liapsub.o: /usr/include/sys/stdtypes.h glist.h nfg.h garray.h
liapsub.o: /usr/include/stdlib.h /usr/include/assert.h gpvector.h gvector.h
liapsub.o: mixed.h nfstrat.h gblock.h mixedsol.h subsolve.h efg.h behav.h
liapsub.o: efstrat.h gdpvect.h efplayer.h infoset.h rational.h integer.h
liapsub.o: /usr/include/math.h /usr/include/floatingpoint.h
liapsub.o: /usr/include/sys/ieeefp.h node.h outcome.h behavsol.h
simpsub.o: rational.h integer.h /usr/include/math.h
simpsub.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h simpsub.imp
simpsub.o: simpsub.h simpdiv.h nfg.h garray.h /usr/include/stdlib.h
simpsub.o: /usr/include/sys/stdtypes.h /usr/include/assert.h gambitio.h
simpsub.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
simpsub.o: gpvector.h gvector.h glist.h grarray.h gstatus.h gsignal.h
simpsub.o: gprogres.h mixed.h nfstrat.h gblock.h mixedsol.h subsolve.h efg.h
simpsub.o: behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h outcome.h
simpsub.o: behavsol.h
enumsub.o: rational.h integer.h /usr/include/math.h
enumsub.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h enumsub.imp
enumsub.o: enumsub.h subsolve.h efg.h gstring.h /usr/include/string.h
enumsub.o: /usr/include/sys/stdtypes.h gblock.h /usr/include/stdlib.h
enumsub.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
enumsub.o: gmisc.h glist.h gpvector.h gvector.h behav.h efstrat.h gdpvect.h
enumsub.o: efplayer.h infoset.h node.h outcome.h nfg.h behavsol.h enum.h
enumsub.o: gstatus.h gsignal.h gprogres.h lhtab.h lemketab.h tableau.h
enumsub.o: ludecomp.h gmatrix.h grarray.h gwatch.h bfs.h gmap.h mixed.h
enumsub.o: nfstrat.h mixedsol.h vertenum.h
csumsub.o: rational.h integer.h /usr/include/math.h
csumsub.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h csumsub.imp
csumsub.o: csumsub.h subsolve.h efg.h gstring.h /usr/include/string.h
csumsub.o: /usr/include/sys/stdtypes.h gblock.h /usr/include/stdlib.h
csumsub.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
csumsub.o: gmisc.h glist.h gpvector.h gvector.h behav.h efstrat.h gdpvect.h
csumsub.o: efplayer.h infoset.h node.h outcome.h nfg.h behavsol.h nfgcsum.h
csumsub.o: gstatus.h gsignal.h gprogres.h tableau.h ludecomp.h gmatrix.h
csumsub.o: grarray.h gwatch.h bfs.h gmap.h lpsolve.h mixed.h nfstrat.h
csumsub.o: mixedsol.h
behavsol.o: behavsol.h gmisc.h behav.h gstring.h /usr/include/string.h
behavsol.o: /usr/include/sys/stdtypes.h efstrat.h gblock.h
behavsol.o: /usr/include/stdlib.h /usr/include/assert.h garray.h gambitio.h
behavsol.o: /usr/include/stdio.h efg.h glist.h gpvector.h gvector.h
behavsol.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h
behavsol.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h node.h
behavsol.o: outcome.h gdpvect.h
lemke.o: lemke.imp gwatch.h gpvector.h gvector.h gmisc.h gambitio.h
lemke.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
lemke.o: /usr/include/sys/stdtypes.h /usr/include/assert.h nfg.h gstring.h
lemke.o: /usr/include/string.h lemke.h mixedsol.h mixed.h nfstrat.h gblock.h
lemke.o: glist.h lhtab.h lemketab.h tableau.h rational.h integer.h
lemke.o: /usr/include/math.h /usr/include/floatingpoint.h
lemke.o: /usr/include/sys/ieeefp.h ludecomp.h gmatrix.h grarray.h bfs.h
lemke.o: gmap.h gstatus.h gsignal.h gprogres.h
nliap.o: nliap.h gambitio.h /usr/include/stdio.h gmisc.h gstatus.h gsignal.h
nliap.o: gprogres.h gstring.h /usr/include/string.h
nliap.o: /usr/include/sys/stdtypes.h glist.h nfg.h garray.h
nliap.o: /usr/include/stdlib.h /usr/include/assert.h gpvector.h gvector.h
nliap.o: mixed.h nfstrat.h gblock.h mixedsol.h gfunc.h
ngobit.o: /usr/include/math.h /usr/include/floatingpoint.h
ngobit.o: /usr/include/sys/ieeefp.h ngobit.h gambitio.h /usr/include/stdio.h
ngobit.o: gmisc.h gstatus.h gsignal.h gprogres.h gstring.h
ngobit.o: /usr/include/string.h /usr/include/sys/stdtypes.h glist.h nfg.h
ngobit.o: garray.h /usr/include/stdlib.h /usr/include/assert.h gpvector.h
ngobit.o: gvector.h mixed.h nfstrat.h gblock.h mixedsol.h gfunc.h
enum.o: enum.imp gwatch.h nfg.h garray.h /usr/include/stdlib.h
enum.o: /usr/include/sys/stdtypes.h /usr/include/assert.h gambitio.h
enum.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
enum.o: gpvector.h gvector.h nfgiter.h enum.h glist.h gstatus.h gsignal.h
enum.o: gprogres.h lhtab.h lemketab.h tableau.h rational.h integer.h
enum.o: /usr/include/math.h /usr/include/floatingpoint.h
enum.o: /usr/include/sys/ieeefp.h ludecomp.h gmatrix.h grarray.h gblock.h
enum.o: bfs.h gmap.h mixed.h nfstrat.h mixedsol.h vertenum.h
simpdiv.o: simpdiv.imp gambitio.h /usr/include/stdio.h gmisc.h nfg.h garray.h
simpdiv.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
simpdiv.o: /usr/include/assert.h gstring.h /usr/include/string.h gpvector.h
simpdiv.o: gvector.h grarray.h gwatch.h simpdiv.h glist.h gstatus.h gsignal.h
simpdiv.o: gprogres.h mixed.h nfstrat.h gblock.h mixedsol.h rational.h
simpdiv.o: integer.h /usr/include/math.h /usr/include/floatingpoint.h
simpdiv.o: /usr/include/sys/ieeefp.h
tableau.o: rational.h integer.h /usr/include/math.h
tableau.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h tableau.imp
tableau.o: tableau.h ludecomp.h gmatrix.h gambitio.h /usr/include/stdio.h
tableau.o: gmisc.h grarray.h gblock.h /usr/include/stdlib.h
tableau.o: /usr/include/sys/stdtypes.h /usr/include/assert.h garray.h
tableau.o: gvector.h glist.h gwatch.h bfs.h gmap.h
ludecomp.o: ludecomp.imp ludecomp.h gmatrix.h gambitio.h /usr/include/stdio.h
ludecomp.o: gmisc.h grarray.h gblock.h /usr/include/stdlib.h
ludecomp.o: /usr/include/sys/stdtypes.h /usr/include/assert.h garray.h
ludecomp.o: gvector.h glist.h gwatch.h glist.imp rational.h integer.h
ludecomp.o: /usr/include/math.h /usr/include/floatingpoint.h
ludecomp.o: /usr/include/sys/ieeefp.h
lhtab.o: lhtab.h lemketab.h tableau.h rational.h integer.h
lhtab.o: /usr/include/math.h /usr/include/floatingpoint.h
lhtab.o: /usr/include/sys/ieeefp.h ludecomp.h gmatrix.h gambitio.h
lhtab.o: /usr/include/stdio.h gmisc.h grarray.h gblock.h
lhtab.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
lhtab.o: /usr/include/assert.h garray.h gvector.h glist.h gwatch.h bfs.h
lhtab.o: gmap.h nfg.h gstring.h /usr/include/string.h gpvector.h nfgiter.h
lhtab.o: nfstrat.h
lemketab.o: lemketab.h tableau.h rational.h integer.h /usr/include/math.h
lemketab.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h ludecomp.h
lemketab.o: gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h grarray.h
lemketab.o: gblock.h /usr/include/stdlib.h /usr/include/sys/stdtypes.h
lemketab.o: /usr/include/assert.h garray.h gvector.h glist.h gwatch.h bfs.h
lemketab.o: gmap.h
grid.o: gmisc.h rational.h integer.h /usr/include/math.h
grid.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h gmatrix.h
grid.o: gambitio.h /usr/include/stdio.h grarray.h gblock.h
grid.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
grid.o: /usr/include/assert.h garray.h gvector.h nfg.h gstring.h
grid.o: /usr/include/string.h gpvector.h nfgiter.h nfstrat.h probvect.h
grid.o: gwatch.h grid.h gstatus.h gsignal.h gprogres.h grarray.imp garray.imp
lpsolve.o: lpsolve.imp lpsolve.h tableau.h rational.h integer.h
lpsolve.o: /usr/include/math.h /usr/include/floatingpoint.h
lpsolve.o: /usr/include/sys/ieeefp.h ludecomp.h gmatrix.h gambitio.h
lpsolve.o: /usr/include/stdio.h gmisc.h grarray.h gblock.h
lpsolve.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
lpsolve.o: /usr/include/assert.h garray.h gvector.h glist.h gwatch.h bfs.h
lpsolve.o: gmap.h
nfdom.o: nfg.h garray.h /usr/include/stdlib.h /usr/include/sys/stdtypes.h
nfdom.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
nfdom.o: gstring.h /usr/include/string.h gpvector.h gvector.h nfstrat.h
nfdom.o: gblock.h nfgciter.h rational.h integer.h /usr/include/math.h
nfdom.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
vertenum.o: vertenum.imp vertenum.h tableau.h rational.h integer.h
vertenum.o: /usr/include/math.h /usr/include/floatingpoint.h
vertenum.o: /usr/include/sys/ieeefp.h ludecomp.h gmatrix.h gambitio.h
vertenum.o: /usr/include/stdio.h gmisc.h grarray.h gblock.h
vertenum.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
vertenum.o: /usr/include/assert.h garray.h gvector.h glist.h gwatch.h bfs.h
vertenum.o: gmap.h gstatus.h gsignal.h gprogres.h gstring.h
vertenum.o: /usr/include/string.h
mixedsol.o: mixedsol.h gmisc.h mixed.h nfstrat.h gstring.h
mixedsol.o: /usr/include/string.h /usr/include/sys/stdtypes.h gblock.h
mixedsol.o: /usr/include/stdlib.h /usr/include/assert.h garray.h gambitio.h
mixedsol.o: /usr/include/stdio.h gpvector.h gvector.h
extshow.o: wx.h wx_form.h wxmisc.h /usr/include/stdio.h wx_timer.h efg.h
extshow.o: gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
extshow.o: gblock.h /usr/include/stdlib.h /usr/include/assert.h garray.h
extshow.o: gambitio.h gmisc.h glist.h gpvector.h gvector.h behav.h efstrat.h
extshow.o: gdpvect.h efplayer.h infoset.h rational.h integer.h
extshow.o: /usr/include/math.h /usr/include/floatingpoint.h
extshow.o: /usr/include/sys/ieeefp.h node.h outcome.h efgconst.h gambit.h
extshow.o: treewin.h treedraw.h treecons.h gambdraw.h twflash.h extshow.h
extshow.o: efgnfgi.h accels.h efgsolng.h behavsol.h bsolnsf.h gslist.h
extshow.o: extsoln.h spread.h grblock.h grarray.h normgui.h elimdomd.h
extshow.o: delsolnd.h
btreewn.o: bitmaps/copy.xpm bitmaps/move.xpm
treedraw.o: wx.h wx_form.h wxmisc.h /usr/include/stdio.h wx_timer.h
treedraw.o: treedraw.h gblock.h /usr/include/stdlib.h
treedraw.o: /usr/include/sys/stdtypes.h /usr/include/assert.h garray.h
treedraw.o: gambitio.h gmisc.h treecons.h gambdraw.h gstring.h
treedraw.o: /usr/include/string.h legendc.h legend.h
twflash.o: wx.h twflash.h wx_timer.h
extsoln.o: wx.h wxmisc.h /usr/include/stdio.h wx_timer.h wx_form.h spread.h
extsoln.o: glist.h gambitio.h gmisc.h gblock.h /usr/include/stdlib.h
extsoln.o: /usr/include/sys/stdtypes.h /usr/include/assert.h garray.h
extsoln.o: grblock.h grarray.h gstring.h /usr/include/string.h rational.h
extsoln.o: integer.h /usr/include/math.h /usr/include/floatingpoint.h
extsoln.o: /usr/include/sys/ieeefp.h extsoln.h efg.h gpvector.h gvector.h
extsoln.o: behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h outcome.h
extsoln.o: efgconst.h extshow.h efgnfgi.h gambit.h accels.h efgsolng.h
extsoln.o: behavsol.h bsolnsf.h gslist.h treedraw.h treecons.h gambdraw.h
extsoln.o: treewin.h twflash.h legendc.h
btreewni.o: wx.h wxmisc.h /usr/include/stdio.h wx_timer.h wx_form.h node.h
btreewni.o: rational.h integer.h /usr/include/math.h
btreewni.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h gblock.h
btreewni.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
btreewni.o: /usr/include/assert.h garray.h gambitio.h gmisc.h efg.h gstring.h
btreewni.o: /usr/include/string.h glist.h gpvector.h gvector.h behav.h
btreewni.o: efstrat.h gdpvect.h efplayer.h infoset.h outcome.h treewin.h
btreewni.o: treedraw.h treecons.h gambdraw.h twflash.h efgconst.h glist.imp
bextshow.o: wx.h wx_form.h wx_tbar.h wxmisc.h /usr/include/stdio.h wx_timer.h
bextshow.o: efg.h gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
bextshow.o: gblock.h /usr/include/stdlib.h /usr/include/assert.h garray.h
bextshow.o: gambitio.h gmisc.h glist.h gpvector.h gvector.h behav.h efstrat.h
bextshow.o: gdpvect.h efplayer.h infoset.h rational.h integer.h
bextshow.o: /usr/include/math.h /usr/include/floatingpoint.h
bextshow.o: /usr/include/sys/ieeefp.h node.h outcome.h efgconst.h gambit.h
bextshow.o: treewin.h treedraw.h treecons.h gambdraw.h twflash.h extshow.h
bextshow.o: efgnfgi.h accels.h efgsolng.h behavsol.h bsolnsf.h gslist.h
bextshow.o: extsoln.h spread.h grblock.h grarray.h efgaccl.h
outcomed.o: wx.h wxmisc.h /usr/include/stdio.h wx_timer.h wx_form.h spread.h
outcomed.o: glist.h gambitio.h gmisc.h gblock.h /usr/include/stdlib.h
outcomed.o: /usr/include/sys/stdtypes.h /usr/include/assert.h garray.h
outcomed.o: grblock.h grarray.h gstring.h /usr/include/string.h efg.h
outcomed.o: gpvector.h gvector.h behav.h efstrat.h gdpvect.h efplayer.h
outcomed.o: infoset.h rational.h integer.h /usr/include/math.h
outcomed.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h node.h
outcomed.o: outcome.h treewin.h treedraw.h treecons.h gambdraw.h twflash.h
outcomed.o: efgconst.h outcomed.h
bsolnsf.o: bsolnsf.h garray.h /usr/include/stdlib.h
bsolnsf.o: /usr/include/sys/stdtypes.h /usr/include/assert.h gambitio.h
bsolnsf.o: /usr/include/stdio.h gmisc.h gslist.h glist.h behavsol.h behav.h
bsolnsf.o: gstring.h /usr/include/string.h efstrat.h gblock.h efg.h
bsolnsf.o: gpvector.h gvector.h efplayer.h infoset.h rational.h integer.h
bsolnsf.o: /usr/include/math.h /usr/include/floatingpoint.h
bsolnsf.o: /usr/include/sys/ieeefp.h node.h outcome.h gdpvect.h gsmincl.h
bsolnsf.o: gslist.imp
normshow.o: normshow.h wx.h wxmisc.h /usr/include/stdio.h wx_timer.h
normshow.o: wx_form.h wxio.h /usr/include/assert.h gambitio.h gmisc.h
normshow.o: gambit.h spread.h glist.h gblock.h /usr/include/stdlib.h
normshow.o: /usr/include/sys/stdtypes.h garray.h grblock.h grarray.h
normshow.o: gstring.h /usr/include/string.h normgui.h normdraw.h gambdraw.h
normshow.o: efgnfgi.h accels.h rational.h integer.h /usr/include/math.h
normshow.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h nfg.h
normshow.o: gpvector.h gvector.h nfgiter.h mixedsol.h mixed.h nfstrat.h
normshow.o: normsoln.h msolnsf.h gslist.h nfgconst.h nfplayer.h delsolnd.h
normshow.o: nfgsolvd.h elimdomd.h
bnormshw.o: normshow.h wx.h wxmisc.h /usr/include/stdio.h wx_timer.h
bnormshw.o: wx_form.h wxio.h /usr/include/assert.h gambitio.h gmisc.h
bnormshw.o: gambit.h spread.h glist.h gblock.h /usr/include/stdlib.h
bnormshw.o: /usr/include/sys/stdtypes.h garray.h grblock.h grarray.h
bnormshw.o: gstring.h /usr/include/string.h normgui.h normdraw.h gambdraw.h
bnormshw.o: efgnfgi.h accels.h rational.h integer.h /usr/include/math.h
bnormshw.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h nfg.h
bnormshw.o: gpvector.h gvector.h nfgiter.h mixedsol.h mixed.h nfstrat.h
bnormshw.o: normsoln.h msolnsf.h gslist.h nfplayer.h normaccl.h nfgconst.h
bnormshw.o: sprdaccl.h sprconst.h
normsoln.o: wx.h nfgconst.h normshow.h wxmisc.h /usr/include/stdio.h
normsoln.o: wx_timer.h wx_form.h wxio.h /usr/include/assert.h gambitio.h
normsoln.o: gmisc.h gambit.h spread.h glist.h gblock.h /usr/include/stdlib.h
normsoln.o: /usr/include/sys/stdtypes.h garray.h grblock.h grarray.h
normsoln.o: gstring.h /usr/include/string.h normgui.h normdraw.h gambdraw.h
normsoln.o: efgnfgi.h accels.h rational.h integer.h /usr/include/math.h
normsoln.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h nfg.h
normsoln.o: gpvector.h gvector.h nfgiter.h mixedsol.h mixed.h nfstrat.h
normsoln.o: normsoln.h msolnsf.h gslist.h nfplayer.h
msolnsf.o: msolnsf.h garray.h /usr/include/stdlib.h
msolnsf.o: /usr/include/sys/stdtypes.h /usr/include/assert.h gambitio.h
msolnsf.o: /usr/include/stdio.h gmisc.h gslist.h glist.h mixedsol.h mixed.h
msolnsf.o: nfstrat.h gstring.h /usr/include/string.h gblock.h gpvector.h
msolnsf.o: gvector.h gsmincl.h rational.h integer.h /usr/include/math.h
msolnsf.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h gslist.imp
wxmisc.o: wx.h wx_form.h wx_help.h wxmisc.h /usr/include/stdio.h wx_timer.h
wxmisc.o: general.h /usr/include/stdlib.h /usr/include/sys/stdtypes.h
wxmisc.o: /usr/include/string.h
wximpl.o: garray.imp /usr/include/stdlib.h /usr/include/sys/stdtypes.h
wximpl.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
wximpl.o: gmisc.h glist.imp glist.h
gambdraw.o: wx.h wxmisc.h /usr/include/stdio.h wx_timer.h wx_form.h
gambdraw.o: gambdraw.h gblock.h /usr/include/stdlib.h
gambdraw.o: /usr/include/sys/stdtypes.h /usr/include/assert.h garray.h
gambdraw.o: gambitio.h gmisc.h gstring.h /usr/include/string.h
wxio.o: wx.h wxio.h /usr/include/stdio.h /usr/include/assert.h gambitio.h
wxio.o: gmisc.h
spread.o: /usr/include/stdio.h wx.h wx_mf.h wx_tbar.h general.h
spread.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
spread.o: /usr/include/string.h wxmisc.h wx_timer.h wx_form.h spread.h
spread.o: glist.h gambitio.h gmisc.h gblock.h /usr/include/assert.h garray.h
spread.o: grblock.h grarray.h gstring.h
spreadim.o: wx.h wxmisc.h /usr/include/stdio.h wx_timer.h wx_form.h grblock.h
spreadim.o: grarray.h gambitio.h gmisc.h garray.h /usr/include/stdlib.h
spreadim.o: /usr/include/sys/stdtypes.h /usr/include/assert.h spread.h
spreadim.o: glist.h gblock.h gstring.h /usr/include/string.h rational.h
spreadim.o: integer.h /usr/include/math.h /usr/include/floatingpoint.h
spreadim.o: /usr/include/sys/ieeefp.h glist.imp grarray.imp grblock.imp
spreadim.o: garray.imp
gambit.o: /usr/include/assert.h /usr/include/string.h
gambit.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h wx.h wx_tbar.h
gambit.o: wxio.h /usr/include/stdio.h gambitio.h gmisc.h gambit.h wxmisc.h
gambit.o: wx_timer.h wx_form.h normgui.h gstring.h extgui.h
gambit.o: /usr/include/signal.h /usr/include/sys/signal.h
gambit.o: /usr/include/vm/faultcode.h /usr/include/math.h
gambit.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
accels.o: wx.h wxmisc.h /usr/include/stdio.h wx_timer.h wx_form.h keynames.h
accels.o: gmisc.h glist.imp glist.h gambitio.h /usr/include/assert.h accels.h
accels.o: garray.h /usr/include/stdlib.h /usr/include/sys/stdtypes.h
accels.o: gstring.h /usr/include/string.h garray.imp
wxstatus.o: wx.h wxstatus.h gstatus.h gsignal.h gmisc.h gprogres.h gambitio.h
wxstatus.o: /usr/include/stdio.h gstring.h /usr/include/string.h
wxstatus.o: /usr/include/sys/stdtypes.h
efgnfgi.o: efgnfgi.h rational.h integer.h /usr/include/math.h
efgnfgi.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
general.o: general.h /usr/include/stdio.h /usr/include/stdlib.h
general.o: /usr/include/sys/stdtypes.h /usr/include/string.h
