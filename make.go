# @(#)make.go	1.2 10/19/95
# Final level of makefiles--this actually builds the code.  Note that you
# must set DEPEND=True for the depend_* targets.
# This should not be edited by the user
# All the site dependent settings are in make.site_$(GUI)_$(HOST)

VPATH= %.o . % ../
.SUFFIXES:
.SUFFIXES: .h .cc .o .y

%:: SCCS/s.%
	rm -f $@
        $(GET) -p $< > $@
.cc.o:
	$(CC) $(CFLAGS) -c $<
.y.o:
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


GLIB_SOURCES = gmisc.cc gambitio.cc garray.cc gblock.cc gstring.cc integer.cc rational.cc gnulib.cc gvector.cc gpvector.cc gdpvect.cc grarray.cc gmatrix.cc gclsig.cc glpsolve.cc bfs.cc gwatch.cc gmessage.cc gfunct.cc gclstats.cc glist.cc

GLIB_OBJECTS = gmisc.o gambitio.o garray.o gblock.o gstring.o integer.o rational.o gnulib.o gvector.o gpvector.o gdpvect.o grarray.o gmatrix.o glpsolve.o bfs.o gwatch.o gmessage.o gfunct.o gclstats.o glist.o

GAME_SOURCES = readnfg.cc readefg.cc nfg.cc nfgdbl.cc nfgrat.cc nfgutils.cc \
		efg.cc efgdbl.cc efgrat.cc nfstrat.cc efgnfg.cc efgutils.cc \
		efstrat.cc

GAME_OBJECTS = readnfg.o readefg.o nfg.o nfgdbl.o nfgrat.o nfgutils.o \
		efg.o efgdbl.o efgrat.o nfstrat.o efgnfg.o efgutils.o \
		efstrat.o

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
               twflash.cc extsoln.cc btreewni.cc bextshow.cc
EGUI_OBJECTS = extshow.o treewin.o btreewn.o btreewn1.o treedraw.o \
               twflash.o extsoln.o btreewni.o bextshow.o
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
	$(MAKE) -f ../make.go CFLAGS='$(CFLAGS) $(INCDIR) $(WXGUI) -DGRATIONAL' \
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

gmisc.o: gmisc.h gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
gmisc.o: rational.h integer.h /usr/include/math.h /usr/include/stdio.h
gmisc.o: /usr/include/stdlib.h /usr/include/limits.h /usr/include/sys/param.h
gmisc.o: /usr/include/sys/types.h /usr/include/machine/param.h
gmisc.o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
gmisc.o: /usr/include/sys/sigevent.h /usr/include/machine/spl.h
gmisc.o: /usr/include/ctype.h
gambitio.o: /usr/include/assert.h /usr/include/sys/stdsyms.h gambitio.h
gambitio.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
garray.o: garray.imp /usr/include/stdlib.h /usr/include/assert.h
garray.o: /usr/include/sys/stdsyms.h garray.h gambitio.h /usr/include/stdio.h
garray.o: gmisc.h gstring.h /usr/include/string.h rational.h integer.h
garray.o: /usr/include/math.h
gblock.o: gmisc.h gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
gblock.o: gblock.imp /usr/include/stdlib.h /usr/include/assert.h gblock.h
gblock.o: garray.h gambitio.h /usr/include/stdio.h rational.h integer.h
gblock.o: /usr/include/math.h garray.imp
gstring.o: /usr/include/stdlib.h /usr/include/assert.h
gstring.o: /usr/include/sys/stdsyms.h /usr/include/ctype.h gambitio.h
gstring.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
integer.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
integer.o: /usr/include/string.h /usr/include/sys/stdsyms.h integer.h
integer.o: gnulib.h /usr/include/stddef.h /usr/include/stdlib.h
integer.o: /usr/include/memory.h /usr/include/errno.h
integer.o: /usr/include/sys/errno.h /usr/include/fcntl.h
integer.o: /usr/include/sys/fcntl.h /usr/include/sys/types.h
integer.o: /usr/include/math.h /usr/include/ctype.h /usr/include/float.h
integer.o: /usr/include/limits.h /usr/include/sys/param.h
integer.o: /usr/include/machine/param.h /usr/include/sys/sysmacros.h
integer.o: /usr/include/sys/time.h /usr/include/sys/sigevent.h
integer.o: /usr/include/machine/spl.h /usr/include/assert.h
rational.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
rational.o: /usr/include/string.h /usr/include/sys/stdsyms.h rational.h
rational.o: integer.h /usr/include/math.h gnulib.h /usr/include/stddef.h
rational.o: /usr/include/stdlib.h /usr/include/memory.h /usr/include/errno.h
rational.o: /usr/include/sys/errno.h /usr/include/fcntl.h
rational.o: /usr/include/sys/fcntl.h /usr/include/sys/types.h
rational.o: /usr/include/values.h /usr/include/float.h /usr/include/assert.h
rational.o: /usr/include/ctype.h
gnulib.o: /usr/include/assert.h /usr/include/sys/stdsyms.h
gnulib.o: /usr/include/values.h gnulib.h /usr/include/stddef.h
gnulib.o: /usr/include/stdlib.h /usr/include/string.h /usr/include/memory.h
gnulib.o: /usr/include/stdio.h /usr/include/errno.h /usr/include/sys/errno.h
gnulib.o: /usr/include/fcntl.h /usr/include/sys/fcntl.h
gnulib.o: /usr/include/sys/types.h /usr/include/math.h
gvector.o: gvector.imp gvector.h gmisc.h gstring.h /usr/include/string.h
gvector.o: /usr/include/sys/stdsyms.h gambitio.h /usr/include/stdio.h
gvector.o: garray.h /usr/include/stdlib.h /usr/include/assert.h rational.h
gvector.o: integer.h /usr/include/math.h
gpvector.o: gpvector.imp gpvector.h gvector.h gmisc.h gstring.h
gpvector.o: /usr/include/string.h /usr/include/sys/stdsyms.h gambitio.h
gpvector.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
gpvector.o: /usr/include/assert.h rational.h integer.h /usr/include/math.h
gdpvect.o: gdpvect.imp gdpvect.h gpvector.h gvector.h gmisc.h gstring.h
gdpvect.o: /usr/include/string.h /usr/include/sys/stdsyms.h gambitio.h
gdpvect.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
gdpvect.o: /usr/include/assert.h rational.h integer.h /usr/include/math.h
grarray.o: grarray.imp grarray.h gambitio.h /usr/include/stdio.h gmisc.h
grarray.o: gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
grarray.o: /usr/include/assert.h garray.h /usr/include/stdlib.h rational.h
grarray.o: integer.h /usr/include/math.h
gmatrix.o: gmatrix.imp gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h
gmatrix.o: gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
gmatrix.o: grarray.h gblock.h /usr/include/stdlib.h /usr/include/assert.h
gmatrix.o: garray.h gvector.h rational.h integer.h /usr/include/math.h
gclsig.o: gsignal.h gmisc.h gstring.h /usr/include/string.h
gclsig.o: /usr/include/sys/stdsyms.h /usr/include/signal.h
gclsig.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
gclsig.o: /usr/include/sys/siginfo.h /usr/include/sys/types.h
gclsig.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gclsig.o: /usr/include/machine/save_state.h
glpsolve.o: glpsolve.imp glpsolve.h gtableau.h /usr/include/assert.h
glpsolve.o: /usr/include/sys/stdsyms.h gmisc.h gstring.h
glpsolve.o: /usr/include/string.h gambitio.h /usr/include/stdio.h gmatrix.h
glpsolve.o: grarray.h gblock.h /usr/include/stdlib.h garray.h gvector.h
glpsolve.o: gmap.h gmessage.h bfs.h rational.h integer.h /usr/include/math.h
glpsolve.o: gtableau.imp
bfs.o: rational.h integer.h /usr/include/math.h garray.imp
bfs.o: /usr/include/stdlib.h /usr/include/assert.h /usr/include/sys/stdsyms.h
bfs.o: garray.h gambitio.h /usr/include/stdio.h gmisc.h gstring.h
bfs.o: /usr/include/string.h gblock.imp gblock.h gmap.imp gmap.h gmessage.h
bfs.o: gmapiter.h bfs.h gmatrix.h grarray.h gvector.h
gwatch.o: gwatch.h /usr/include/stdio.h /usr/include/math.h
gmessage.o: gmessage.h
gfunct.o: gfunct.imp gfunct.h gmisc.h gstring.h /usr/include/string.h
gfunct.o: /usr/include/sys/stdsyms.h gambitio.h /usr/include/stdio.h
gfunct.o: gvector.h garray.h /usr/include/stdlib.h /usr/include/assert.h
gfunct.o: /usr/include/math.h gmatrix.h grarray.h gblock.h rational.h
gfunct.o: integer.h
gclstats.o: gstatus.h gsignal.h gmisc.h gstring.h /usr/include/string.h
gclstats.o: /usr/include/sys/stdsyms.h gprogres.h gambitio.h
gclstats.o: /usr/include/stdio.h /usr/include/signal.h
gclstats.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
gclstats.o: /usr/include/sys/siginfo.h /usr/include/sys/types.h
gclstats.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gclstats.o: /usr/include/machine/save_state.h
glist.o: gmisc.h gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
glist.o: glist.imp glist.h gambitio.h /usr/include/stdio.h glistit.h
glist.o: /usr/include/assert.h rational.h integer.h /usr/include/math.h
readnfg.o: /usr/include/stdio.h /usr/include/ctype.h gmisc.h gstring.h
readnfg.o: /usr/include/string.h /usr/include/sys/stdsyms.h gambitio.h
readnfg.o: glist.h rational.h integer.h /usr/include/math.h nfg.h garray.h
readnfg.o: /usr/include/stdlib.h /usr/include/assert.h gpvector.h gvector.h
readefg.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/ctype.h
readefg.o: gambitio.h gmisc.h gstring.h /usr/include/string.h
readefg.o: /usr/include/sys/stdsyms.h rational.h integer.h
readefg.o: /usr/include/math.h ggrstack.h gstack.h glist.h efg.h gblock.h
readefg.o: /usr/include/assert.h garray.h behav.h efstrat.h gpvector.h
readefg.o: gvector.h gdpvect.h efplayer.h infoset.h node.h outcome.h
readefg.o: gstack.imp ggrstack.imp
nfg.o: /usr/include/assert.h /usr/include/sys/stdsyms.h /usr/include/limits.h
nfg.o: /usr/include/sys/param.h /usr/include/sys/types.h
nfg.o: /usr/include/machine/param.h /usr/include/sys/sysmacros.h
nfg.o: /usr/include/sys/time.h /usr/include/sys/sigevent.h
nfg.o: /usr/include/machine/spl.h nfg.h garray.h /usr/include/stdlib.h
nfg.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
nfg.o: /usr/include/string.h gpvector.h gvector.h nfg.imp nfplayer.h
nfg.o: nfstrat.h gblock.h nfgiter.h mixed.h rational.h integer.h
nfg.o: /usr/include/math.h garray.imp gblock.imp
nfgdbl.o: rational.h integer.h /usr/include/math.h nfg.h garray.h
nfgdbl.o: /usr/include/stdlib.h /usr/include/assert.h
nfgdbl.o: /usr/include/sys/stdsyms.h gambitio.h /usr/include/stdio.h gmisc.h
nfgdbl.o: gstring.h /usr/include/string.h gpvector.h gvector.h nfg.imp
nfgdbl.o: nfplayer.h nfstrat.h gblock.h nfgiter.h mixed.h nfgiter.imp
nfgdbl.o: contiter.imp contiter.h readnfg.imp glist.h readnfg.h garray.imp
nfgdbl.o: glist.imp glistit.h
nfgrat.o: rational.h integer.h /usr/include/math.h nfg.h garray.h
nfgrat.o: /usr/include/stdlib.h /usr/include/assert.h
nfgrat.o: /usr/include/sys/stdsyms.h gambitio.h /usr/include/stdio.h gmisc.h
nfgrat.o: gstring.h /usr/include/string.h gpvector.h gvector.h nfg.imp
nfgrat.o: nfplayer.h nfstrat.h gblock.h nfgiter.h mixed.h nfgiter.imp
nfgrat.o: contiter.imp contiter.h readnfg.imp glist.h readnfg.h garray.imp
nfgrat.o: glist.imp glistit.h
nfgutils.o: gmisc.h gstring.h /usr/include/string.h
nfgutils.o: /usr/include/sys/stdsyms.h nfg.h garray.h /usr/include/stdlib.h
nfgutils.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gpvector.h
nfgutils.o: gvector.h rational.h integer.h /usr/include/math.h
efg.o: garray.h /usr/include/stdlib.h /usr/include/assert.h
efg.o: /usr/include/sys/stdsyms.h gambitio.h /usr/include/stdio.h gmisc.h
efg.o: gstring.h /usr/include/string.h rational.h integer.h
efg.o: /usr/include/math.h garray.imp gblock.imp gblock.h glist.imp glist.h
efg.o: glistit.h efg.h behav.h efstrat.h gpvector.h gvector.h gdpvect.h
efg.o: efplayer.h infoset.h node.h outcome.h
efgdbl.o: rational.h integer.h /usr/include/math.h glist.h gambitio.h
efgdbl.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
efgdbl.o: /usr/include/sys/stdsyms.h glistit.h efg.imp efg.h gblock.h
efgdbl.o: /usr/include/stdlib.h /usr/include/assert.h garray.h behav.h
efgdbl.o: efstrat.h gpvector.h gvector.h gdpvect.h efplayer.h infoset.h
efgdbl.o: node.h outcome.h efgutils.h tnode.h readefg.imp ggrstack.h gstack.h
efgdbl.o: readefg.h glist.imp
efgrat.o: rational.h integer.h /usr/include/math.h glist.h gambitio.h
efgrat.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
efgrat.o: /usr/include/sys/stdsyms.h glistit.h efg.imp efg.h gblock.h
efgrat.o: /usr/include/stdlib.h /usr/include/assert.h garray.h behav.h
efgrat.o: efstrat.h gpvector.h gvector.h gdpvect.h efplayer.h infoset.h
efgrat.o: node.h outcome.h efgutils.h tnode.h readefg.imp ggrstack.h gstack.h
efgrat.o: readefg.h glist.imp
nfstrat.o: nfstrat.h gstring.h /usr/include/string.h
nfstrat.o: /usr/include/sys/stdsyms.h gblock.h /usr/include/stdlib.h
nfstrat.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
nfstrat.o: gmisc.h nfplayer.h nfg.h gpvector.h gvector.h
efgnfg.o: efg.h gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
efgnfg.o: gblock.h /usr/include/stdlib.h /usr/include/assert.h garray.h
efgnfg.o: gambitio.h /usr/include/stdio.h gmisc.h behav.h efstrat.h
efgnfg.o: gpvector.h gvector.h gdpvect.h efplayer.h infoset.h rational.h
efgnfg.o: integer.h /usr/include/math.h node.h outcome.h nfg.h nfplayer.h
efgnfg.o: nfstrat.h mixed.h glist.h contiter.h nfgiter.h glist.imp glistit.h
efgnfg.o: garray.imp tnode.h
efgutils.o: efgutils.h efg.h gstring.h /usr/include/string.h
efgutils.o: /usr/include/sys/stdsyms.h gblock.h /usr/include/stdlib.h
efgutils.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
efgutils.o: gmisc.h behav.h efstrat.h gpvector.h gvector.h gdpvect.h
efgutils.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h
efgutils.o: node.h outcome.h glist.h
efstrat.o: efg.h gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
efstrat.o: gblock.h /usr/include/stdlib.h /usr/include/assert.h garray.h
efstrat.o: gambitio.h /usr/include/stdio.h gmisc.h behav.h efstrat.h
efstrat.o: gpvector.h gvector.h gdpvect.h efplayer.h infoset.h rational.h
efstrat.o: integer.h /usr/include/math.h node.h outcome.h
gobit.o: /usr/include/math.h gobit.h gambitio.h /usr/include/stdio.h gmisc.h
gobit.o: gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h gstatus.h
gobit.o: gsignal.h gprogres.h gvector.h garray.h /usr/include/stdlib.h
gobit.o: /usr/include/assert.h gwatch.h /usr/include/values.h rational.h
gobit.o: integer.h
egobit.o: /usr/include/math.h egobit.h efg.h gstring.h /usr/include/string.h
egobit.o: /usr/include/sys/stdsyms.h gblock.h /usr/include/stdlib.h
egobit.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
egobit.o: gmisc.h behav.h efstrat.h gpvector.h gvector.h gdpvect.h efplayer.h
egobit.o: infoset.h rational.h integer.h node.h outcome.h gobit.h gstatus.h
egobit.o: gsignal.h gprogres.h glist.h gfunct.h gmatrix.h grarray.h
liap.o: liap.h gambitio.h /usr/include/stdio.h gmisc.h gstring.h
liap.o: /usr/include/string.h /usr/include/sys/stdsyms.h gvector.h garray.h
liap.o: /usr/include/stdlib.h /usr/include/assert.h gwatch.h rational.h
liap.o: integer.h /usr/include/math.h
eliap.o: /usr/include/math.h eliap.h efg.h gstring.h /usr/include/string.h
eliap.o: /usr/include/sys/stdsyms.h gblock.h /usr/include/stdlib.h
eliap.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
eliap.o: gmisc.h behav.h efstrat.h gpvector.h gvector.h gdpvect.h efplayer.h
eliap.o: infoset.h rational.h integer.h node.h outcome.h liap.h glist.h
eliap.o: gfunct.h gmatrix.h grarray.h
seqform.o: seqform.h efg.h gstring.h /usr/include/string.h
seqform.o: /usr/include/sys/stdsyms.h gblock.h /usr/include/stdlib.h
seqform.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
seqform.o: gmisc.h behav.h efstrat.h gpvector.h gvector.h gdpvect.h
seqform.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h
seqform.o: node.h outcome.h glist.h gmatrix.h grarray.h lemketab.h tableau.h
seqform.o: ludecomp.h bfs.h gmap.h gmessage.h gwatch.h
nliap.o: /usr/include/math.h nliap.h nfg.h garray.h /usr/include/stdlib.h
nliap.o: /usr/include/assert.h /usr/include/sys/stdsyms.h gambitio.h
nliap.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
nliap.o: gpvector.h gvector.h liap.h glist.h mixed.h nfstrat.h gblock.h
nliap.o: gfunct.h
ngobit.o: /usr/include/math.h ngobit.h nfg.h garray.h /usr/include/stdlib.h
ngobit.o: /usr/include/assert.h /usr/include/sys/stdsyms.h gambitio.h
ngobit.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
ngobit.o: gpvector.h gvector.h gobit.h gstatus.h gsignal.h gprogres.h glist.h
ngobit.o: mixed.h nfstrat.h gblock.h gfunct.h rational.h integer.h
enum.o: gwatch.h nfg.h garray.h /usr/include/stdlib.h /usr/include/assert.h
enum.o: /usr/include/sys/stdsyms.h gambitio.h /usr/include/stdio.h gmisc.h
enum.o: gstring.h /usr/include/string.h gpvector.h gvector.h nfgiter.h enum.h
enum.o: glist.h gstatus.h gsignal.h gprogres.h lhtab.h lemketab.h tableau.h
enum.o: rational.h integer.h /usr/include/math.h ludecomp.h gmatrix.h
enum.o: grarray.h gblock.h bfs.h gmap.h gmessage.h mixed.h nfstrat.h
simpdiv.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
simpdiv.o: /usr/include/string.h /usr/include/sys/stdsyms.h nfg.h garray.h
simpdiv.o: /usr/include/stdlib.h /usr/include/assert.h gpvector.h gvector.h
simpdiv.o: grarray.h gwatch.h simpdiv.h glist.h gstatus.h gsignal.h
simpdiv.o: gprogres.h mixed.h nfstrat.h gblock.h rational.h integer.h
simpdiv.o: /usr/include/math.h
tableau.o: rational.h integer.h /usr/include/math.h tableau.imp tableau.h
tableau.o: ludecomp.h gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h
tableau.o: gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
tableau.o: grarray.h gblock.h /usr/include/stdlib.h /usr/include/assert.h
tableau.o: garray.h gvector.h glist.h bfs.h gmap.h gmessage.h normal.h
tableau.o: gpvector.h normiter.h
ludecomp.o: ludecomp.imp ludecomp.h gmatrix.h gambitio.h /usr/include/stdio.h
ludecomp.o: gmisc.h gstring.h /usr/include/string.h
ludecomp.o: /usr/include/sys/stdsyms.h grarray.h gblock.h
ludecomp.o: /usr/include/stdlib.h /usr/include/assert.h garray.h gvector.h
ludecomp.o: glist.h glistit.h glist.imp rational.h integer.h
ludecomp.o: /usr/include/math.h
purenash.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
purenash.o: /usr/include/string.h /usr/include/sys/stdsyms.h nfg.h garray.h
purenash.o: /usr/include/stdlib.h /usr/include/assert.h gpvector.h gvector.h
purenash.o: nfgiter.h contiter.h gblock.h rational.h integer.h
purenash.o: /usr/include/math.h glist.h mixed.h nfstrat.h
lhtab.o: lhtab.h lemketab.h tableau.h rational.h integer.h
lhtab.o: /usr/include/math.h ludecomp.h gmatrix.h gambitio.h
lhtab.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
lhtab.o: /usr/include/sys/stdsyms.h grarray.h gblock.h /usr/include/stdlib.h
lhtab.o: /usr/include/assert.h garray.h gvector.h glist.h bfs.h gmap.h
lhtab.o: gmessage.h nfg.h gpvector.h nfgiter.h nfstrat.h
lemketab.o: lemketab.h tableau.h rational.h integer.h /usr/include/math.h
lemketab.o: ludecomp.h gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h
lemketab.o: gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
lemketab.o: grarray.h gblock.h /usr/include/stdlib.h /usr/include/assert.h
lemketab.o: garray.h gvector.h glist.h bfs.h gmap.h gmessage.h
grid.o: gmisc.h gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
grid.o: rational.h integer.h /usr/include/math.h gmatrix.h gambitio.h
grid.o: /usr/include/stdio.h grarray.h gblock.h /usr/include/stdlib.h
grid.o: /usr/include/assert.h garray.h gvector.h nfg.h gpvector.h nfgiter.h
grid.o: nfstrat.h probvect.h gwatch.h grid.h gstatus.h gsignal.h gprogres.h
grid.o: grarray.imp garray.imp
csum.o: rational.h integer.h /usr/include/math.h gwatch.h gpvector.h
csum.o: gvector.h gmisc.h gstring.h /usr/include/string.h
csum.o: /usr/include/sys/stdsyms.h gambitio.h /usr/include/stdio.h garray.h
csum.o: /usr/include/stdlib.h /usr/include/assert.h nfg.h nfgiter.h csum.h
csum.o: glist.h gstatus.h gsignal.h gprogres.h tableau.h ludecomp.h gmatrix.h
csum.o: grarray.h gblock.h bfs.h gmap.h gmessage.h lpsolve.h mixed.h
csum.o: nfstrat.h
lpsolve.o: lpsolve.imp lpsolve.h tableau.h rational.h integer.h
lpsolve.o: /usr/include/math.h ludecomp.h gmatrix.h gambitio.h
lpsolve.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
lpsolve.o: /usr/include/sys/stdsyms.h grarray.h gblock.h
lpsolve.o: /usr/include/stdlib.h /usr/include/assert.h garray.h gvector.h
lpsolve.o: glist.h bfs.h gmap.h gmessage.h
nfdom.o: nfg.h garray.h /usr/include/stdlib.h /usr/include/assert.h
nfdom.o: /usr/include/sys/stdsyms.h gambitio.h /usr/include/stdio.h gmisc.h
nfdom.o: gstring.h /usr/include/string.h gpvector.h gvector.h nfstrat.h
nfdom.o: gblock.h contiter.h rational.h integer.h /usr/include/math.h
extshow.o: ./wx_fake/wx.h ./wx_fake/wx_form.h wxmisc.h /usr/include/stdio.h
extshow.o: ./wx_fake/wx_timer.h efg.h gstring.h /usr/include/string.h
extshow.o: /usr/include/sys/stdsyms.h gblock.h /usr/include/stdlib.h
extshow.o: /usr/include/assert.h garray.h gambitio.h gmisc.h behav.h
extshow.o: efstrat.h gpvector.h gvector.h gdpvect.h efplayer.h infoset.h
extshow.o: rational.h integer.h /usr/include/math.h node.h outcome.h
extshow.o: efgconst.h gambit.h treewin.h glist.h treedraw.h treecons.h
extshow.o: gambdraw.h twflash.h extshow.h efgnfgi.h accels.h spread.h
extshow.o: grblock.h grarray.h extsoln.h normgui.h wxio.h delsolnd.h eliap.h
extshow.o: liap.h liapprm.h algdlgs.h seqform.h gmatrix.h lemketab.h
extshow.o: tableau.h ludecomp.h bfs.h gmap.h gmessage.h seqfprm.h
treedraw.o: ./wx_fake/wx.h ./wx_fake/wx_form.h wxmisc.h /usr/include/stdio.h
treedraw.o: ./wx_fake/wx_timer.h treedraw.h gblock.h /usr/include/stdlib.h
treedraw.o: /usr/include/assert.h /usr/include/sys/stdsyms.h garray.h
treedraw.o: gambitio.h gmisc.h gstring.h /usr/include/string.h treecons.h
treedraw.o: gambdraw.h legendc.h legend.h
twflash.o: ./wx_fake/wx.h twflash.h ./wx_fake/wx_timer.h
extsoln.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
extsoln.o: ./wx_fake/wx_form.h spread.h glist.h gambitio.h gmisc.h gstring.h
extsoln.o: /usr/include/string.h /usr/include/sys/stdsyms.h gblock.h
extsoln.o: /usr/include/stdlib.h /usr/include/assert.h garray.h grblock.h
extsoln.o: grarray.h rational.h integer.h /usr/include/math.h extsoln.h efg.h
extsoln.o: behav.h efstrat.h gpvector.h gvector.h gdpvect.h efplayer.h
extsoln.o: infoset.h node.h outcome.h efgconst.h extshow.h efgnfgi.h gambit.h
extsoln.o: accels.h
btreewni.o: glist.imp glist.h gambitio.h /usr/include/stdio.h gmisc.h
btreewni.o: gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
btreewni.o: glistit.h /usr/include/assert.h
bextshow.o: ./wx_fake/wx.h ./wx_fake/wx_form.h ./wx_fake/wx_tbar.h wxmisc.h
bextshow.o: /usr/include/stdio.h ./wx_fake/wx_timer.h efg.h gstring.h
bextshow.o: /usr/include/string.h /usr/include/sys/stdsyms.h gblock.h
bextshow.o: /usr/include/stdlib.h /usr/include/assert.h garray.h gambitio.h
bextshow.o: gmisc.h behav.h efstrat.h gpvector.h gvector.h gdpvect.h
bextshow.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h
bextshow.o: node.h outcome.h efgconst.h gambit.h treewin.h glist.h treedraw.h
bextshow.o: treecons.h gambdraw.h twflash.h extshow.h efgnfgi.h accels.h
bextshow.o: spread.h grblock.h grarray.h efgaccl.h
normshow.o: normshow.h ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h
normshow.o: ./wx_fake/wx_timer.h ./wx_fake/wx_form.h wxio.h
normshow.o: /usr/include/assert.h /usr/include/sys/stdsyms.h gambitio.h
normshow.o: gmisc.h gstring.h /usr/include/string.h gambit.h spread.h glist.h
normshow.o: gblock.h /usr/include/stdlib.h garray.h grblock.h grarray.h
normshow.o: normgui.h normdraw.h gambdraw.h efgnfgi.h accels.h rational.h
normshow.o: integer.h /usr/include/math.h nfg.h gpvector.h gvector.h
normshow.o: nfgiter.h contiter.h normsoln.h nfgconst.h nfplayer.h nliap.h
normshow.o: liap.h mixed.h nfstrat.h liapprm.h algdlgs.h simpdiv.h gstatus.h
normshow.o: gsignal.h gprogres.h simpprm.h enum.h lhtab.h lemketab.h
normshow.o: tableau.h ludecomp.h gmatrix.h bfs.h gmap.h gmessage.h enumprm.h
normshow.o: csum.h lpsolve.h csumprm.h
bnormshw.o: normshow.h ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h
bnormshw.o: ./wx_fake/wx_timer.h ./wx_fake/wx_form.h wxio.h
bnormshw.o: /usr/include/assert.h /usr/include/sys/stdsyms.h gambitio.h
bnormshw.o: gmisc.h gstring.h /usr/include/string.h gambit.h spread.h glist.h
bnormshw.o: gblock.h /usr/include/stdlib.h garray.h grblock.h grarray.h
bnormshw.o: normgui.h normdraw.h gambdraw.h efgnfgi.h accels.h rational.h
bnormshw.o: integer.h /usr/include/math.h nfg.h gpvector.h gvector.h
bnormshw.o: nfgiter.h contiter.h normsoln.h nfplayer.h normaccl.h nfgconst.h
bnormshw.o: sprdaccl.h sprconst.h
wxmisc.o: ./wx_fake/wx.h ./wx_fake/wx_form.h ./wx_fake/wx_help.h wxmisc.h
wxmisc.o: /usr/include/stdio.h ./wx_fake/wx_timer.h general.h
wxmisc.o: /usr/include/stdlib.h /usr/include/string.h
wxmisc.o: /usr/include/sys/stdsyms.h
wximpl.o: garray.imp /usr/include/stdlib.h /usr/include/assert.h
wximpl.o: /usr/include/sys/stdsyms.h garray.h gambitio.h /usr/include/stdio.h
wximpl.o: gmisc.h gstring.h /usr/include/string.h glist.imp glist.h glistit.h
gambdraw.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
gambdraw.o: ./wx_fake/wx_form.h gambdraw.h gblock.h /usr/include/stdlib.h
gambdraw.o: /usr/include/assert.h /usr/include/sys/stdsyms.h garray.h
gambdraw.o: gambitio.h gmisc.h gstring.h /usr/include/string.h
wxio.o: wxio.h /usr/include/stdio.h /usr/include/assert.h
wxio.o: /usr/include/sys/stdsyms.h ./wx_fake/wx.h gambitio.h gmisc.h
wxio.o: gstring.h /usr/include/string.h
spread.o: /usr/include/stdio.h ./wx_fake/wx.h ./wx_fake/wx_mf.h general.h
spread.o: /usr/include/stdlib.h /usr/include/string.h
spread.o: /usr/include/sys/stdsyms.h wxmisc.h ./wx_fake/wx_timer.h
spread.o: ./wx_fake/wx_form.h spread.h glist.h gambitio.h gmisc.h gstring.h
spread.o: gblock.h /usr/include/assert.h garray.h grblock.h grarray.h
spreadim.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
spreadim.o: ./wx_fake/wx_form.h grblock.h grarray.h gambitio.h gmisc.h
spreadim.o: gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
spreadim.o: garray.h /usr/include/stdlib.h /usr/include/assert.h spread.h
spreadim.o: glist.h gblock.h rational.h integer.h /usr/include/math.h
spreadim.o: glist.imp glistit.h grarray.imp grblock.imp garray.imp
gambit.o: /usr/include/assert.h /usr/include/sys/stdsyms.h ./wx_fake/wx.h
gambit.o: ./wx_fake/wx_tbar.h wxio.h /usr/include/stdio.h gambitio.h gmisc.h
gambit.o: gstring.h /usr/include/string.h gambit.h wxmisc.h
gambit.o: ./wx_fake/wx_timer.h ./wx_fake/wx_form.h normgui.h extgui.h
gambit.o: /usr/include/signal.h /usr/include/sys/signal.h
gambit.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gambit.o: /usr/include/sys/types.h /usr/include/sys/syscall.h
gambit.o: /usr/include/machine/frame.h /usr/include/machine/save_state.h
accels.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
accels.o: ./wx_fake/wx_form.h keynames.h gmisc.h gstring.h
accels.o: /usr/include/string.h /usr/include/sys/stdsyms.h glist.imp glist.h
accels.o: gambitio.h glistit.h /usr/include/assert.h accels.h garray.h
accels.o: /usr/include/stdlib.h garray.imp
wxstatus.o: ./wx_fake/wx.h wxstatus.h gstatus.h gsignal.h gmisc.h gstring.h
wxstatus.o: /usr/include/string.h /usr/include/sys/stdsyms.h gprogres.h
wxstatus.o: gambitio.h /usr/include/stdio.h
efgnfgi.o: efgnfgi.h rational.h integer.h /usr/include/math.h
general.o: general.h /usr/include/stdio.h /usr/include/stdlib.h
general.o: /usr/include/string.h /usr/include/sys/stdsyms.h
