# $Id$
# Final level of makefiles--this actually builds the code.  Note that you
# must set DEPEND=True for the depend_* targets.
# This should not be edited by the user
# All the site dependent settings are in make.site_$(GUI)_$(HOST)

VPATH= %.o . % ../
.SUFFIXES:
.SUFFIXES: .h .cc .o .y

.cc.o:
	$(CC) $(CFLAGS) -c $<
.y.o:
	bison++ -d -h ../$(*F).h -o ../$(*F).cc $<
	$(CC) $(CFLAGS) -c ../$(*F).cc

CC = g++
CFLAGS = -Wall -fno-implicit-templates

ifndef DEPEND
	# Site & platform specific information
	include ../make.site_$(GUI)_$(HOST)
	# Generic WxWin information
	WXLIBDIR = -L$(WXDIR)/lib
	WXINCDIR = -I$(WXDIR)/include/base -I$(WXDIR)/include/x 
	WXLIB = -lwx_$(GUI)_$(HOST)

	# Combine the site dependent and independent data

	LIBDIR = $(WXLIBDIR) $(XLIBDIR)
	INCDIR = $(WXINCDIR) $(XINCDIR)

	GUILIBS = $(WXLIB) $(XLIB)
endif

GLIB_SOURCES = gmisc.cc gambitio.cc garray.cc gblock.cc gstring.cc integer.cc rational.cc gnulib.cc gvector.cc gpvector.cc gdpvect.cc grarray.cc gmatrix.cc gclsig.cc glpsolve.cc bfs.cc gwatch.cc gmessage.cc gfunct.cc gclstats.cc glist.cc

GLIB_OBJECTS = gmisc.o gambitio.o garray.o gblock.o gstring.o integer.o rational.o gnulib.o gvector.o gpvector.o gdpvect.o grarray.o gmatrix.o glpsolve.o bfs.o gwatch.o gmessage.o gfunct.o gclstats.o glist.o

GAME_SOURCES = readnfg.cc readefg.cc nfg.cc nfgdbl.cc nfgrat.cc nfgutils.cc \
		efg.cc efgdbl.cc efgrat.cc nfstrat.cc efgnfg.cc efgutils.cc

GAME_OBJECTS = readnfg.o readefg.o nfg.o nfgdbl.o nfgrat.o nfgutils.o \
		efg.o efgdbl.o efgrat.o nfstrat.o efgnfg.o efgutils.o

EALG_SOURCES = gobit.cc egobit.cc liap.cc eliap.cc seqform.cc

EALG_OBJECTS = gobit.o egobit.o liap.o eliap.o seqform.o

NALG_SOURCES = lemke.cc nliap.cc ngobit.cc enum.cc simpdiv.cc tableau.cc ludecomp.cc purenash.cc lhtab.cc lemketab.cc grid.cc csum.cc lpsolve.cc nfdom.cc

NALG_OBJECTS = lemke.o nliap.o ngobit.o enum.o simpdiv.o tableau.o ludecomp.o purenash.o lhtab.o lemketab.o grid.o csum.o lpsolve.o nfdom.o

AGCL_SOURCES = gcompile.cc gsm.cc gsmfunc.cc gsmoper.cc gsmhash.cc gclmath.cc \
               gsminstr.cc portion.cc nfgfunc.cc efgfunc.cc listfunc.cc \
               algfunc.cc gcl.cc gclsig.cc

AGCL_OBJECTS = gcompile.o gsm.o gsmfunc.o gsmoper.o gsmhash.o gclmath.o \
               gsminstr.o portion.o nfgfunc.o efgfunc.o listfunc.o \
               algfunc.o gcl.o gclsig.o
# Gui stuff for the extensive form
EGUI_SOURCES = extshow.cc treewin.cc btreewn.cc btreewn1.cc treedraw.cc \
               twflash.cc extsoln.cc btreewni.cc
EGUI_OBJECTS = extshow.o treewin.o btreewn.o btreewn1.o treedraw.o \
               twflash.o extsoln.o btreewni.o
# Gui stuff for the normal form
NGUI_SOURCES = normshow.cc bnormshw.cc
NGUI_OBJECTS = normshow.o bnormshw.o
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
	$(MAKE) -f ../make.go CFLAGS='$(CFLAGS) $(INCDIR) $(WXGUI)' \
			GUI=$(GUI) HOST=$(HOST) gui_$(GUI)_$(HOST)

gui_$(GUI)_$(HOST): $(GUI_OBJECTS)
	$(CC) $(GUI_OBJECTS) $(LIBDIR) $(GUILIBS) -o ../gui_$(GUI)_$(HOST)

depend_gcl:
	makedepend -f make.go $(GCL_SOURCES)  >& /dev/null
depend_gui:
	makedepend -f make.go $(GUI_SOURCES) >& /dev/null
clean:
	rm -f *.o core *~ *.bak gambgui gcl *.zip

$(OBJDIR):
	mkdir $(OBJDIR)
# DO NOT DELETE THIS LINE -- make depend depends on it.

gmisc.o: gmisc.h gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
gmisc.o: rational.h integer.h /usr/include/math.h
gmisc.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
gmisc.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/limits.h
gmisc.o: /usr/include/ctype.h
gambitio.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
gambitio.o: gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
garray.o: garray.imp /usr/include/stdlib.h /usr/include/sys/stdtypes.h
garray.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
garray.o: gmisc.h gstring.h /usr/include/string.h rational.h integer.h
garray.o: /usr/include/math.h /usr/include/floatingpoint.h
garray.o: /usr/include/sys/ieeefp.h
gblock.o: gmisc.h gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
gblock.o: gblock.imp /usr/include/stdlib.h /usr/include/assert.h gblock.h
gblock.o: garray.h gambitio.h /usr/include/stdio.h rational.h integer.h
gblock.o: /usr/include/math.h /usr/include/floatingpoint.h
gblock.o: /usr/include/sys/ieeefp.h garray.imp
gstring.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
gstring.o: /usr/include/assert.h /usr/include/ctype.h gambitio.h
gstring.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
integer.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
integer.o: /usr/include/string.h /usr/include/sys/stdtypes.h integer.h
integer.o: gnulib.h /usr/include/stddef.h /usr/include/stdlib.h
integer.o: /usr/include/memory.h /usr/include/errno.h
integer.o: /usr/include/sys/errno.h /usr/include/fcntl.h
integer.o: /usr/include/sys/fcntlcom.h /usr/include/sys/stat.h
integer.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
integer.o: /usr/include/math.h /usr/include/floatingpoint.h
integer.o: /usr/include/sys/ieeefp.h /usr/include/ctype.h
integer.o: /usr/include/limits.h /usr/include/assert.h
rational.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
rational.o: /usr/include/string.h /usr/include/sys/stdtypes.h rational.h
rational.o: integer.h /usr/include/math.h /usr/include/floatingpoint.h
rational.o: /usr/include/sys/ieeefp.h gnulib.h /usr/include/stddef.h
rational.o: /usr/include/stdlib.h /usr/include/memory.h /usr/include/errno.h
rational.o: /usr/include/sys/errno.h /usr/include/fcntl.h
rational.o: /usr/include/sys/fcntlcom.h /usr/include/sys/stat.h
rational.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
rational.o: /usr/include/values.h /usr/include/assert.h /usr/include/ctype.h
gnulib.o: /usr/include/assert.h /usr/include/values.h gnulib.h
gnulib.o: /usr/include/stddef.h /usr/include/sys/stdtypes.h
gnulib.o: /usr/include/stdlib.h /usr/include/string.h /usr/include/memory.h
gnulib.o: /usr/include/stdio.h /usr/include/errno.h /usr/include/sys/errno.h
gnulib.o: /usr/include/fcntl.h /usr/include/sys/fcntlcom.h
gnulib.o: /usr/include/sys/stat.h /usr/include/sys/types.h
gnulib.o: /usr/include/sys/sysmacros.h /usr/include/math.h
gnulib.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
gvector.o: gvector.imp gvector.h gmisc.h gstring.h /usr/include/string.h
gvector.o: /usr/include/sys/stdtypes.h gambitio.h /usr/include/stdio.h
gvector.o: garray.h /usr/include/stdlib.h /usr/include/assert.h rational.h
gvector.o: integer.h /usr/include/math.h /usr/include/floatingpoint.h
gvector.o: /usr/include/sys/ieeefp.h
gpvector.o: gpvector.imp gpvector.h gvector.h gmisc.h gstring.h
gpvector.o: /usr/include/string.h /usr/include/sys/stdtypes.h gambitio.h
gpvector.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
gpvector.o: /usr/include/assert.h rational.h integer.h /usr/include/math.h
gpvector.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
gdpvect.o: gdpvect.imp gdpvect.h gpvector.h gvector.h gmisc.h gstring.h
gdpvect.o: /usr/include/string.h /usr/include/sys/stdtypes.h gambitio.h
gdpvect.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
gdpvect.o: /usr/include/assert.h rational.h integer.h /usr/include/math.h
gdpvect.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
grarray.o: grarray.imp grarray.h gambitio.h /usr/include/stdio.h gmisc.h
grarray.o: gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
grarray.o: /usr/include/assert.h garray.h /usr/include/stdlib.h rational.h
grarray.o: integer.h /usr/include/math.h /usr/include/floatingpoint.h
grarray.o: /usr/include/sys/ieeefp.h
gmatrix.o: gmatrix.imp gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h
gmatrix.o: gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
gmatrix.o: grarray.h gblock.h /usr/include/stdlib.h /usr/include/assert.h
gmatrix.o: garray.h gvector.h rational.h integer.h /usr/include/math.h
gmatrix.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
gclsig.o: gsignal.h gmisc.h gstring.h /usr/include/string.h
gclsig.o: /usr/include/sys/stdtypes.h /usr/include/signal.h
gclsig.o: /usr/include/sys/signal.h /usr/include/vm/faultcode.h
glpsolve.o: glpsolve.imp glpsolve.h gtableau.h /usr/include/assert.h gmisc.h
glpsolve.o: gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
glpsolve.o: gambitio.h /usr/include/stdio.h gmatrix.h grarray.h gblock.h
glpsolve.o: /usr/include/stdlib.h garray.h gvector.h gmap.h gmessage.h bfs.h
glpsolve.o: rational.h integer.h /usr/include/math.h
glpsolve.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
glpsolve.o: gtableau.imp
bfs.o: rational.h integer.h /usr/include/math.h /usr/include/floatingpoint.h
bfs.o: /usr/include/sys/ieeefp.h garray.imp /usr/include/stdlib.h
bfs.o: /usr/include/sys/stdtypes.h /usr/include/assert.h garray.h gambitio.h
bfs.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
bfs.o: gblock.imp gblock.h gmap.imp gmap.h gmessage.h gmapiter.h bfs.h
bfs.o: gmatrix.h grarray.h gvector.h
gwatch.o: gwatch.h /usr/include/stdio.h /usr/include/math.h
gwatch.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
gmessage.o: gmessage.h
gfunct.o: gfunct.imp gfunct.h gmisc.h gstring.h /usr/include/string.h
gfunct.o: /usr/include/sys/stdtypes.h gambitio.h /usr/include/stdio.h
gfunct.o: gvector.h garray.h /usr/include/stdlib.h /usr/include/assert.h
gfunct.o: /usr/include/math.h /usr/include/floatingpoint.h
gfunct.o: /usr/include/sys/ieeefp.h gmatrix.h grarray.h gblock.h rational.h
gfunct.o: integer.h
gclstats.o: gstatus.h gsignal.h gmisc.h gstring.h /usr/include/string.h
gclstats.o: /usr/include/sys/stdtypes.h gprogres.h gambitio.h
gclstats.o: /usr/include/stdio.h /usr/include/signal.h
gclstats.o: /usr/include/sys/signal.h /usr/include/vm/faultcode.h
glist.o: gmisc.h gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
glist.o: glist.imp glist.h gambitio.h /usr/include/stdio.h glistit.h
glist.o: /usr/include/assert.h rational.h integer.h /usr/include/math.h
glist.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
readnfg.o: /usr/include/stdio.h /usr/include/ctype.h gmisc.h gstring.h
readnfg.o: /usr/include/string.h /usr/include/sys/stdtypes.h gambitio.h
readnfg.o: glist.h rational.h integer.h /usr/include/math.h
readnfg.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h nfg.h
readnfg.o: garray.h /usr/include/stdlib.h /usr/include/assert.h gpvector.h
readnfg.o: gvector.h
readefg.o: /usr/include/stdio.h /usr/include/stdlib.h
readefg.o: /usr/include/sys/stdtypes.h /usr/include/ctype.h gambitio.h
readefg.o: gmisc.h gstring.h /usr/include/string.h rational.h integer.h
readefg.o: /usr/include/math.h /usr/include/floatingpoint.h
readefg.o: /usr/include/sys/ieeefp.h ggrstack.h gstack.h glist.h efg.h
readefg.o: gblock.h /usr/include/assert.h garray.h behav.h gdpvect.h
readefg.o: gpvector.h gvector.h efplayer.h infoset.h node.h outcome.h
readefg.o: gstack.imp ggrstack.imp
nfg.o: /usr/include/assert.h /usr/include/limits.h nfg.h garray.h
nfg.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h gambitio.h
nfg.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
nfg.o: gpvector.h gvector.h nfg.imp nfplayer.h nfstrat.h gblock.h nfgiter.h
nfg.o: mixed.h rational.h integer.h /usr/include/math.h
nfg.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h garray.imp
nfg.o: gblock.imp
nfgdbl.o: rational.h integer.h /usr/include/math.h
nfgdbl.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h nfg.h
nfgdbl.o: garray.h /usr/include/stdlib.h /usr/include/sys/stdtypes.h
nfgdbl.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
nfgdbl.o: gstring.h /usr/include/string.h gpvector.h gvector.h nfg.imp
nfgdbl.o: nfplayer.h nfstrat.h gblock.h nfgiter.h mixed.h nfgiter.imp
nfgdbl.o: contiter.imp contiter.h readnfg.imp glist.h readnfg.h garray.imp
nfgdbl.o: glist.imp glistit.h
nfgrat.o: rational.h integer.h /usr/include/math.h
nfgrat.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h nfg.h
nfgrat.o: garray.h /usr/include/stdlib.h /usr/include/sys/stdtypes.h
nfgrat.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
nfgrat.o: gstring.h /usr/include/string.h gpvector.h gvector.h nfg.imp
nfgrat.o: nfplayer.h nfstrat.h gblock.h nfgiter.h mixed.h nfgiter.imp
nfgrat.o: contiter.imp contiter.h readnfg.imp glist.h readnfg.h garray.imp
nfgrat.o: glist.imp glistit.h
nfgutils.o: gmisc.h gstring.h /usr/include/string.h
nfgutils.o: /usr/include/sys/stdtypes.h nfg.h garray.h /usr/include/stdlib.h
nfgutils.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gpvector.h
nfgutils.o: gvector.h rational.h integer.h /usr/include/math.h
nfgutils.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
efg.o: garray.h /usr/include/stdlib.h /usr/include/sys/stdtypes.h
efg.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
efg.o: gstring.h /usr/include/string.h rational.h integer.h
efg.o: /usr/include/math.h /usr/include/floatingpoint.h
efg.o: /usr/include/sys/ieeefp.h garray.imp gblock.imp gblock.h glist.imp
efg.o: glist.h glistit.h efg.h behav.h gdpvect.h gpvector.h gvector.h
efg.o: efplayer.h infoset.h node.h outcome.h
efgdbl.o: rational.h integer.h /usr/include/math.h
efgdbl.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h glist.h
efgdbl.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
efgdbl.o: /usr/include/string.h /usr/include/sys/stdtypes.h glistit.h efg.imp
efgdbl.o: efg.h gblock.h /usr/include/stdlib.h /usr/include/assert.h garray.h
efgdbl.o: behav.h gdpvect.h gpvector.h gvector.h efplayer.h infoset.h node.h
efgdbl.o: outcome.h efgutils.h tnode.h readefg.imp ggrstack.h gstack.h
efgdbl.o: readefg.h glist.imp
efgrat.o: rational.h integer.h /usr/include/math.h
efgrat.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h glist.h
efgrat.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
efgrat.o: /usr/include/string.h /usr/include/sys/stdtypes.h glistit.h efg.imp
efgrat.o: efg.h gblock.h /usr/include/stdlib.h /usr/include/assert.h garray.h
efgrat.o: behav.h gdpvect.h gpvector.h gvector.h efplayer.h infoset.h node.h
efgrat.o: outcome.h efgutils.h tnode.h readefg.imp ggrstack.h gstack.h
efgrat.o: readefg.h glist.imp
nfstrat.o: nfstrat.h gstring.h /usr/include/string.h
nfstrat.o: /usr/include/sys/stdtypes.h gblock.h /usr/include/stdlib.h
nfstrat.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
nfstrat.o: gmisc.h nfplayer.h nfg.h gpvector.h gvector.h
efgnfg.o: efg.h gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
efgnfg.o: gblock.h /usr/include/stdlib.h /usr/include/assert.h garray.h
efgnfg.o: gambitio.h /usr/include/stdio.h gmisc.h behav.h gdpvect.h
efgnfg.o: gpvector.h gvector.h efplayer.h infoset.h rational.h integer.h
efgnfg.o: /usr/include/math.h /usr/include/floatingpoint.h
efgnfg.o: /usr/include/sys/ieeefp.h node.h outcome.h nfg.h nfplayer.h
efgnfg.o: nfstrat.h mixed.h glist.h contiter.h nfgiter.h glist.imp glistit.h
efgnfg.o: garray.imp tnode.h
efgutils.o: efgutils.h efg.h gstring.h /usr/include/string.h
efgutils.o: /usr/include/sys/stdtypes.h gblock.h /usr/include/stdlib.h
efgutils.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
efgutils.o: gmisc.h behav.h gdpvect.h gpvector.h gvector.h efplayer.h
efgutils.o: infoset.h rational.h integer.h /usr/include/math.h
efgutils.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h node.h
efgutils.o: outcome.h glist.h
gobit.o: /usr/include/math.h /usr/include/floatingpoint.h
gobit.o: /usr/include/sys/ieeefp.h gobit.h gambitio.h /usr/include/stdio.h
gobit.o: gmisc.h gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
gobit.o: gstatus.h gsignal.h gprogres.h gvector.h garray.h
gobit.o: /usr/include/stdlib.h /usr/include/assert.h gwatch.h
gobit.o: /usr/include/values.h rational.h integer.h
egobit.o: /usr/include/math.h /usr/include/floatingpoint.h
egobit.o: /usr/include/sys/ieeefp.h egobit.h efg.h gstring.h
egobit.o: /usr/include/string.h /usr/include/sys/stdtypes.h gblock.h
egobit.o: /usr/include/stdlib.h /usr/include/assert.h garray.h gambitio.h
egobit.o: /usr/include/stdio.h gmisc.h behav.h gdpvect.h gpvector.h gvector.h
egobit.o: efplayer.h infoset.h rational.h integer.h node.h outcome.h gobit.h
egobit.o: gstatus.h gsignal.h gprogres.h glist.h gfunct.h gmatrix.h grarray.h
liap.o: liap.h gambitio.h /usr/include/stdio.h gmisc.h gstring.h
liap.o: /usr/include/string.h /usr/include/sys/stdtypes.h gvector.h garray.h
liap.o: /usr/include/stdlib.h /usr/include/assert.h gwatch.h rational.h
liap.o: integer.h /usr/include/math.h /usr/include/floatingpoint.h
liap.o: /usr/include/sys/ieeefp.h
eliap.o: /usr/include/math.h /usr/include/floatingpoint.h
eliap.o: /usr/include/sys/ieeefp.h eliap.h efg.h gstring.h
eliap.o: /usr/include/string.h /usr/include/sys/stdtypes.h gblock.h
eliap.o: /usr/include/stdlib.h /usr/include/assert.h garray.h gambitio.h
eliap.o: /usr/include/stdio.h gmisc.h behav.h gdpvect.h gpvector.h gvector.h
eliap.o: efplayer.h infoset.h rational.h integer.h node.h outcome.h liap.h
eliap.o: glist.h gfunct.h gmatrix.h grarray.h
seqform.o: seqform.h efg.h gstring.h /usr/include/string.h
seqform.o: /usr/include/sys/stdtypes.h gblock.h /usr/include/stdlib.h
seqform.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
seqform.o: gmisc.h behav.h gdpvect.h gpvector.h gvector.h efplayer.h
seqform.o: infoset.h rational.h integer.h /usr/include/math.h
seqform.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h node.h
seqform.o: outcome.h glist.h gmatrix.h grarray.h lemketab.h tableau.h
seqform.o: ludecomp.h bfs.h gmap.h gmessage.h gwatch.h
lemke.o: rational.h integer.h /usr/include/math.h
lemke.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h gwatch.h
lemke.o: gpvector.h gvector.h gmisc.h gstring.h /usr/include/string.h
lemke.o: /usr/include/sys/stdtypes.h gambitio.h /usr/include/stdio.h garray.h
lemke.o: /usr/include/stdlib.h /usr/include/assert.h nfg.h nfgiter.h
lemke.o: gtableau.h gmatrix.h grarray.h gblock.h gmap.h gmessage.h bfs.h
lemke.o: lemke.h mixed.h nfstrat.h glist.h
nliap.o: /usr/include/math.h /usr/include/floatingpoint.h
nliap.o: /usr/include/sys/ieeefp.h nliap.h nfg.h garray.h
nliap.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
nliap.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
nliap.o: gstring.h /usr/include/string.h gpvector.h gvector.h liap.h glist.h
nliap.o: mixed.h nfstrat.h gblock.h gfunct.h
ngobit.o: /usr/include/math.h /usr/include/floatingpoint.h
ngobit.o: /usr/include/sys/ieeefp.h ngobit.h nfg.h garray.h
ngobit.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
ngobit.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
ngobit.o: gstring.h /usr/include/string.h gpvector.h gvector.h gobit.h
ngobit.o: gstatus.h gsignal.h gprogres.h glist.h mixed.h nfstrat.h gblock.h
ngobit.o: gfunct.h rational.h integer.h
enum.o: gwatch.h nfg.h garray.h /usr/include/stdlib.h
enum.o: /usr/include/sys/stdtypes.h /usr/include/assert.h gambitio.h
enum.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
enum.o: gpvector.h gvector.h nfgiter.h enum.h glist.h gstatus.h gsignal.h
enum.o: gprogres.h lhtab.h lemketab.h tableau.h rational.h integer.h
enum.o: /usr/include/math.h /usr/include/floatingpoint.h
enum.o: /usr/include/sys/ieeefp.h ludecomp.h gmatrix.h grarray.h gblock.h
enum.o: bfs.h gmap.h gmessage.h mixed.h nfstrat.h
simpdiv.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
simpdiv.o: /usr/include/string.h /usr/include/sys/stdtypes.h nfg.h garray.h
simpdiv.o: /usr/include/stdlib.h /usr/include/assert.h gpvector.h gvector.h
simpdiv.o: grarray.h gwatch.h simpdiv.h glist.h gstatus.h gsignal.h
simpdiv.o: gprogres.h mixed.h nfstrat.h gblock.h rational.h integer.h
simpdiv.o: /usr/include/math.h /usr/include/floatingpoint.h
simpdiv.o: /usr/include/sys/ieeefp.h
tableau.o: rational.h integer.h /usr/include/math.h
tableau.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h tableau.imp
tableau.o: tableau.h ludecomp.h gmatrix.h gambitio.h /usr/include/stdio.h
tableau.o: gmisc.h gstring.h /usr/include/string.h
tableau.o: /usr/include/sys/stdtypes.h grarray.h gblock.h
tableau.o: /usr/include/stdlib.h /usr/include/assert.h garray.h gvector.h
tableau.o: glist.h bfs.h gmap.h gmessage.h normal.h gpvector.h normiter.h
ludecomp.o: ludecomp.imp ludecomp.h gmatrix.h gambitio.h /usr/include/stdio.h
ludecomp.o: gmisc.h gstring.h /usr/include/string.h
ludecomp.o: /usr/include/sys/stdtypes.h grarray.h gblock.h
ludecomp.o: /usr/include/stdlib.h /usr/include/assert.h garray.h gvector.h
ludecomp.o: glist.h glistit.h glist.imp rational.h integer.h
ludecomp.o: /usr/include/math.h /usr/include/floatingpoint.h
ludecomp.o: /usr/include/sys/ieeefp.h
purenash.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
purenash.o: /usr/include/string.h /usr/include/sys/stdtypes.h nfg.h garray.h
purenash.o: /usr/include/stdlib.h /usr/include/assert.h gpvector.h gvector.h
purenash.o: nfgiter.h contiter.h gblock.h rational.h integer.h
purenash.o: /usr/include/math.h /usr/include/floatingpoint.h
purenash.o: /usr/include/sys/ieeefp.h glist.h mixed.h nfstrat.h
lhtab.o: lhtab.h lemketab.h tableau.h rational.h integer.h
lhtab.o: /usr/include/math.h /usr/include/floatingpoint.h
lhtab.o: /usr/include/sys/ieeefp.h ludecomp.h gmatrix.h gambitio.h
lhtab.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
lhtab.o: /usr/include/sys/stdtypes.h grarray.h gblock.h /usr/include/stdlib.h
lhtab.o: /usr/include/assert.h garray.h gvector.h glist.h bfs.h gmap.h
lhtab.o: gmessage.h nfg.h gpvector.h nfgiter.h nfstrat.h
lemketab.o: lemketab.h tableau.h rational.h integer.h /usr/include/math.h
lemketab.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h ludecomp.h
lemketab.o: gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h gstring.h
lemketab.o: /usr/include/string.h /usr/include/sys/stdtypes.h grarray.h
lemketab.o: gblock.h /usr/include/stdlib.h /usr/include/assert.h garray.h
lemketab.o: gvector.h glist.h bfs.h gmap.h gmessage.h
grid.o: gmisc.h gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
grid.o: rational.h integer.h /usr/include/math.h /usr/include/floatingpoint.h
grid.o: /usr/include/sys/ieeefp.h gmatrix.h gambitio.h /usr/include/stdio.h
grid.o: grarray.h gblock.h /usr/include/stdlib.h /usr/include/assert.h
grid.o: garray.h gvector.h nfg.h gpvector.h nfgiter.h nfstrat.h probvect.h
grid.o: gwatch.h grid.h gstatus.h gsignal.h gprogres.h grarray.imp garray.imp
csum.o: rational.h integer.h /usr/include/math.h /usr/include/floatingpoint.h
csum.o: /usr/include/sys/ieeefp.h gwatch.h gpvector.h gvector.h gmisc.h
csum.o: gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
csum.o: gambitio.h /usr/include/stdio.h garray.h /usr/include/stdlib.h
csum.o: /usr/include/assert.h nfg.h nfgiter.h csum.h glist.h gstatus.h
csum.o: gsignal.h gprogres.h tableau.h ludecomp.h gmatrix.h grarray.h
csum.o: gblock.h bfs.h gmap.h gmessage.h lpsolve.h mixed.h nfstrat.h
lpsolve.o: lpsolve.imp lpsolve.h tableau.h rational.h integer.h
lpsolve.o: /usr/include/math.h /usr/include/floatingpoint.h
lpsolve.o: /usr/include/sys/ieeefp.h ludecomp.h gmatrix.h gambitio.h
lpsolve.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
lpsolve.o: /usr/include/sys/stdtypes.h grarray.h gblock.h
lpsolve.o: /usr/include/stdlib.h /usr/include/assert.h garray.h gvector.h
lpsolve.o: glist.h bfs.h gmap.h gmessage.h
nfdom.o: nfg.h garray.h /usr/include/stdlib.h /usr/include/sys/stdtypes.h
nfdom.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
nfdom.o: gstring.h /usr/include/string.h gpvector.h gvector.h nfstrat.h
nfdom.o: gblock.h contiter.h rational.h integer.h /usr/include/math.h
nfdom.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
extshow.o: wxmisc.h /usr/include/stdio.h efg.h gstring.h
extshow.o: /usr/include/string.h /usr/include/sys/stdtypes.h gblock.h
extshow.o: /usr/include/stdlib.h /usr/include/assert.h garray.h gambitio.h
extshow.o: gmisc.h behav.h gdpvect.h gpvector.h gvector.h efplayer.h
extshow.o: infoset.h rational.h integer.h /usr/include/math.h
extshow.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h node.h
extshow.o: outcome.h efgconst.h gambit.h treewin.h glist.h treedraw.h
extshow.o: treecons.h gambdraw.h twflash.h extshow.h efgnfgi.h accels.h
extshow.o: spread.h grblock.h grarray.h extsoln.h normgui.h wxio.h efgaccl.h
extshow.o: delsolnd.h eliap.h liap.h liapprm.h algdlgs.h seqform.h gmatrix.h
extshow.o: lemketab.h tableau.h ludecomp.h bfs.h gmap.h gmessage.h seqfprm.h
extshow.o: extgui.h
treedraw.o: wxmisc.h /usr/include/stdio.h treedraw.h gblock.h
treedraw.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
treedraw.o: /usr/include/assert.h garray.h gambitio.h gmisc.h gstring.h
treedraw.o: /usr/include/string.h treecons.h gambdraw.h legendc.h legend.h
twflash.o: twflash.h
extsoln.o: wxmisc.h /usr/include/stdio.h spread.h glist.h gambitio.h gmisc.h
extsoln.o: gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
extsoln.o: gblock.h /usr/include/stdlib.h /usr/include/assert.h garray.h
extsoln.o: grblock.h grarray.h rational.h integer.h /usr/include/math.h
extsoln.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h extsoln.h
extsoln.o: efg.h behav.h gdpvect.h gpvector.h gvector.h efplayer.h infoset.h
extsoln.o: node.h outcome.h efgconst.h extshow.h efgnfgi.h gambit.h accels.h
btreewni.o: glist.imp glist.h gambitio.h /usr/include/stdio.h gmisc.h
btreewni.o: gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
btreewni.o: glistit.h /usr/include/assert.h
normshow.o: normshow.h wxmisc.h /usr/include/stdio.h wxio.h
normshow.o: /usr/include/assert.h gambitio.h gmisc.h gstring.h
normshow.o: /usr/include/string.h /usr/include/sys/stdtypes.h gambit.h
normshow.o: spread.h glist.h gblock.h /usr/include/stdlib.h garray.h
normshow.o: grblock.h grarray.h normgui.h normdraw.h gambdraw.h efgnfgi.h
normshow.o: accels.h rational.h integer.h /usr/include/math.h
normshow.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h nfg.h
normshow.o: gpvector.h gvector.h nfgiter.h contiter.h normsoln.h nfgconst.h
normshow.o: nfplayer.h delsolnd.h nfgsolvd.h nliap.h liap.h mixed.h nfstrat.h
normshow.o: liapprm.h algdlgs.h simpdiv.h gstatus.h gsignal.h gprogres.h
normshow.o: simpprm.h enum.h lhtab.h lemketab.h tableau.h ludecomp.h
normshow.o: gmatrix.h bfs.h gmap.h gmessage.h enumprm.h csum.h lpsolve.h
normshow.o: csumprm.h
bnormshw.o: normshow.h wxmisc.h /usr/include/stdio.h wxio.h
bnormshw.o: /usr/include/assert.h gambitio.h gmisc.h gstring.h
bnormshw.o: /usr/include/string.h /usr/include/sys/stdtypes.h gambit.h
bnormshw.o: spread.h glist.h gblock.h /usr/include/stdlib.h garray.h
bnormshw.o: grblock.h grarray.h normgui.h normdraw.h gambdraw.h efgnfgi.h
bnormshw.o: accels.h rational.h integer.h /usr/include/math.h
bnormshw.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h nfg.h
bnormshw.o: gpvector.h gvector.h nfgiter.h contiter.h normsoln.h nfplayer.h
bnormshw.o: normaccl.h nfgconst.h sprdaccl.h sprconst.h
wxmisc.o: wxmisc.h /usr/include/stdio.h general.h /usr/include/stdlib.h
wxmisc.o: /usr/include/sys/stdtypes.h /usr/include/string.h
wximpl.o: garray.imp /usr/include/stdlib.h /usr/include/sys/stdtypes.h
wximpl.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
wximpl.o: gmisc.h gstring.h /usr/include/string.h glist.imp glist.h glistit.h
gambdraw.o: wxmisc.h /usr/include/stdio.h gambdraw.h gblock.h
gambdraw.o: /usr/include/stdlib.h /usr/include/sys/stdtypes.h
gambdraw.o: /usr/include/assert.h garray.h gambitio.h gmisc.h gstring.h
gambdraw.o: /usr/include/string.h
wxio.o: wxio.h /usr/include/stdio.h /usr/include/assert.h gambitio.h gmisc.h
wxio.o: gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
spread.o: /usr/include/stdio.h general.h /usr/include/stdlib.h
spread.o: /usr/include/sys/stdtypes.h /usr/include/string.h wxmisc.h spread.h
spread.o: glist.h gambitio.h gmisc.h gstring.h gblock.h /usr/include/assert.h
spread.o: garray.h grblock.h grarray.h
spreadim.o: wxmisc.h /usr/include/stdio.h grblock.h grarray.h gambitio.h
spreadim.o: gmisc.h gstring.h /usr/include/string.h
spreadim.o: /usr/include/sys/stdtypes.h garray.h /usr/include/stdlib.h
spreadim.o: /usr/include/assert.h spread.h glist.h gblock.h rational.h
spreadim.o: integer.h /usr/include/math.h /usr/include/floatingpoint.h
spreadim.o: /usr/include/sys/ieeefp.h glist.imp glistit.h grarray.imp
spreadim.o: grblock.imp garray.imp
gambit.o: /usr/include/assert.h wxio.h /usr/include/stdio.h gambitio.h
gambit.o: gmisc.h gstring.h /usr/include/string.h /usr/include/sys/stdtypes.h
gambit.o: gambit.h wxmisc.h normgui.h extgui.h /usr/include/signal.h
gambit.o: /usr/include/sys/signal.h /usr/include/vm/faultcode.h
accels.o: wxmisc.h /usr/include/stdio.h keynames.h gmisc.h gstring.h
accels.o: /usr/include/string.h /usr/include/sys/stdtypes.h glist.imp glist.h
accels.o: gambitio.h glistit.h /usr/include/assert.h accels.h garray.h
accels.o: /usr/include/stdlib.h garray.imp
wxstatus.o: wxstatus.h gstatus.h gsignal.h gmisc.h gstring.h
wxstatus.o: /usr/include/string.h /usr/include/sys/stdtypes.h gprogres.h
wxstatus.o: gambitio.h /usr/include/stdio.h
efgnfgi.o: efgnfgi.h rational.h integer.h /usr/include/math.h
efgnfgi.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
general.o: general.h /usr/include/stdio.h /usr/include/stdlib.h
general.o: /usr/include/sys/stdtypes.h /usr/include/string.h
