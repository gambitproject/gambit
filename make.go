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


GLIB_SOURCES = gmisc.cc gambitio.cc garray.cc gblock.cc gstring.cc integer.cc rational.cc gnulib.cc gvector.cc gpvector.cc gdpvect.cc grarray.cc gmatrix.cc gclsig.cc glpsolve.cc bfs.cc gwatch.cc gmessage.cc gfunct.cc gclstats.cc glist.cc subsolve.cc

GLIB_OBJECTS = gmisc.o gambitio.o garray.o gblock.o gstring.o integer.o rational.o gnulib.o gvector.o gpvector.o gdpvect.o grarray.o gmatrix.o glpsolve.o bfs.o gwatch.o gmessage.o gfunct.o gclstats.o glist.o subsolve.o

GAME_SOURCES = readnfg.cc readefg.cc nfg.cc nfgdbl.cc nfgrat.cc nfgutils.cc \
		efg.cc efgdbl.cc efgrat.cc nfstrat.cc efgnfg.cc efgutils.cc \
		efstrat.cc

GAME_OBJECTS = readnfg.o readefg.o nfg.o nfgdbl.o nfgrat.o nfgutils.o \
		efg.o efgdbl.o efgrat.o nfstrat.o efgnfg.o efgutils.o \
		efstrat.o

EALG_SOURCES = gobit.cc egobit.cc liap.cc eliap.cc seqform.cc

EALG_OBJECTS = gobit.o egobit.o liap.o eliap.o seqform.o

NALG_SOURCES = lemke.cc nliap.cc ngobit.cc enum.cc simpdiv.cc tableau.cc \
		ludecomp.cc purenash.cc lhtab.cc lemketab.cc grid.cc csum.cc \
		lpsolve.cc nfdom.cc vertenum.cc

NALG_OBJECTS = lemke.o nliap.o ngobit.o enum.o simpdiv.o tableau.o ludecomp.o \
		purenash.o lhtab.o lemketab.o grid.o csum.o lpsolve.o nfdom.o \
		vertenum.o

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

gmisc.o: gmisc.h gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
gmisc.o: rational.h integer.h /usr/include/math.h /usr/include/stdio.h
gmisc.o: /usr/include/stdlib.h /usr/include/sys/wait.h
gmisc.o: /usr/include/sys/types.h /usr/include/sys/signal.h
gmisc.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gmisc.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gmisc.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gmisc.o: /usr/include/limits.h /usr/include/sys/param.h
gmisc.o: /usr/include/machine/param.h /usr/include/sys/sysmacros.h
gmisc.o: /usr/include/machine/param_shm.h /usr/include/sys/time.h
gmisc.o: /usr/include/machine/spl.h /usr/include/ctype.h
gambitio.o: /usr/include/assert.h /usr/include/sys/stdsyms.h gambitio.h
gambitio.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
garray.o: garray.imp /usr/include/stdlib.h /usr/include/sys/wait.h
garray.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
garray.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
garray.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
garray.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
garray.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
garray.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
garray.o: /usr/include/string.h rational.h integer.h /usr/include/math.h
gblock.o: gmisc.h gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
gblock.o: gblock.imp /usr/include/stdlib.h /usr/include/sys/wait.h
gblock.o: /usr/include/sys/types.h /usr/include/sys/signal.h
gblock.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gblock.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gblock.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gblock.o: /usr/include/assert.h gblock.h garray.h gambitio.h
gblock.o: /usr/include/stdio.h rational.h integer.h /usr/include/math.h
gblock.o: garray.imp
gstring.o: /usr/include/stdlib.h /usr/include/sys/wait.h
gstring.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
gstring.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
gstring.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
gstring.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
gstring.o: /usr/include/machine/frame.h /usr/include/assert.h
gstring.o: /usr/include/ctype.h gambitio.h /usr/include/stdio.h gmisc.h
gstring.o: gstring.h /usr/include/string.h
integer.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
integer.o: /usr/include/string.h /usr/include/sys/stdsyms.h integer.h
integer.o: gnulib.h /usr/include/stddef.h /usr/include/stdlib.h
integer.o: /usr/include/sys/wait.h /usr/include/sys/types.h
integer.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
integer.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
integer.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
integer.o: /usr/include/machine/frame.h /usr/include/memory.h
integer.o: /usr/include/errno.h /usr/include/sys/errno.h /usr/include/fcntl.h
integer.o: /usr/include/sys/fcntl.h /usr/include/math.h /usr/include/ctype.h
integer.o: ./wx_fake/float.h /usr/include/limits.h /usr/include/sys/param.h
integer.o: /usr/include/machine/param.h /usr/include/sys/sysmacros.h
integer.o: /usr/include/machine/param_shm.h /usr/include/sys/time.h
integer.o: /usr/include/machine/spl.h /usr/include/assert.h
rational.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
rational.o: /usr/include/string.h /usr/include/sys/stdsyms.h rational.h
rational.o: integer.h /usr/include/math.h gnulib.h /usr/include/stddef.h
rational.o: /usr/include/stdlib.h /usr/include/sys/wait.h
rational.o: /usr/include/sys/types.h /usr/include/sys/signal.h
rational.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
rational.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
rational.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
rational.o: /usr/include/memory.h /usr/include/errno.h
rational.o: /usr/include/sys/errno.h /usr/include/fcntl.h
rational.o: /usr/include/sys/fcntl.h /usr/include/values.h ./wx_fake/float.h
rational.o: /usr/include/assert.h /usr/include/ctype.h
gnulib.o: /usr/include/assert.h /usr/include/sys/stdsyms.h
gnulib.o: /usr/include/values.h gnulib.h /usr/include/stddef.h
gnulib.o: /usr/include/stdlib.h /usr/include/sys/wait.h
gnulib.o: /usr/include/sys/types.h /usr/include/sys/signal.h
gnulib.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gnulib.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gnulib.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gnulib.o: /usr/include/string.h /usr/include/memory.h /usr/include/stdio.h
gnulib.o: /usr/include/errno.h /usr/include/sys/errno.h /usr/include/fcntl.h
gnulib.o: /usr/include/sys/fcntl.h /usr/include/math.h
gvector.o: gvector.imp gvector.h gmisc.h gstring.h /usr/include/string.h
gvector.o: /usr/include/sys/stdsyms.h gambitio.h /usr/include/stdio.h
gvector.o: garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
gvector.o: /usr/include/sys/types.h /usr/include/sys/signal.h
gvector.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gvector.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gvector.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gvector.o: /usr/include/assert.h rational.h integer.h /usr/include/math.h
gpvector.o: gpvector.imp gpvector.h gvector.h gmisc.h gstring.h
gpvector.o: /usr/include/string.h /usr/include/sys/stdsyms.h gambitio.h
gpvector.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
gpvector.o: /usr/include/sys/wait.h /usr/include/sys/types.h
gpvector.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
gpvector.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
gpvector.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
gpvector.o: /usr/include/machine/frame.h /usr/include/assert.h rational.h
gpvector.o: integer.h /usr/include/math.h
gdpvect.o: gdpvect.imp gdpvect.h gpvector.h gvector.h gmisc.h gstring.h
gdpvect.o: /usr/include/string.h /usr/include/sys/stdsyms.h gambitio.h
gdpvect.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
gdpvect.o: /usr/include/sys/wait.h /usr/include/sys/types.h
gdpvect.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
gdpvect.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
gdpvect.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
gdpvect.o: /usr/include/machine/frame.h /usr/include/assert.h rational.h
gdpvect.o: integer.h /usr/include/math.h
grarray.o: grarray.imp grarray.h gambitio.h /usr/include/stdio.h gmisc.h
grarray.o: gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
grarray.o: /usr/include/assert.h garray.h /usr/include/stdlib.h
grarray.o: /usr/include/sys/wait.h /usr/include/sys/types.h
grarray.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
grarray.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
grarray.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
grarray.o: /usr/include/machine/frame.h rational.h integer.h
grarray.o: /usr/include/math.h
gmatrix.o: gmatrix.imp gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h
gmatrix.o: gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
gmatrix.o: grarray.h gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
gmatrix.o: /usr/include/sys/types.h /usr/include/sys/signal.h
gmatrix.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gmatrix.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gmatrix.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gmatrix.o: /usr/include/assert.h garray.h gvector.h rational.h integer.h
gmatrix.o: /usr/include/math.h
gclsig.o: gsignal.h gmisc.h gstring.h /usr/include/string.h
gclsig.o: /usr/include/sys/stdsyms.h /usr/include/signal.h
gclsig.o: /usr/include/sys/signal.h /usr/include/sys/types.h
gclsig.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gclsig.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gclsig.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
glpsolve.o: glpsolve.imp glpsolve.h gtableau.h /usr/include/assert.h
glpsolve.o: /usr/include/sys/stdsyms.h gmisc.h gstring.h
glpsolve.o: /usr/include/string.h gambitio.h /usr/include/stdio.h gmatrix.h
glpsolve.o: grarray.h gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
glpsolve.o: /usr/include/sys/types.h /usr/include/sys/signal.h
glpsolve.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
glpsolve.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
glpsolve.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h garray.h
glpsolve.o: gvector.h gmap.h gmessage.h bfs.h rational.h integer.h
glpsolve.o: /usr/include/math.h gtableau.imp
bfs.o: rational.h integer.h /usr/include/math.h garray.imp
bfs.o: /usr/include/stdlib.h /usr/include/sys/wait.h
bfs.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
bfs.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
bfs.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
bfs.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
bfs.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h gambitio.h
bfs.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
bfs.o: gblock.imp gblock.h glist.imp glist.h glistit.h gmap.imp gmap.h
bfs.o: gmessage.h gmapiter.h bfs.h gmatrix.h grarray.h gvector.h
gwatch.o: gwatch.h /usr/include/stdio.h /usr/include/math.h
gmessage.o: gmessage.h
gfunct.o: gfunct.imp gfunct.h gmisc.h gstring.h /usr/include/string.h
gfunct.o: /usr/include/sys/stdsyms.h gambitio.h /usr/include/stdio.h
gfunct.o: gvector.h garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
gfunct.o: /usr/include/sys/types.h /usr/include/sys/signal.h
gfunct.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gfunct.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gfunct.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gfunct.o: /usr/include/assert.h gstatus.h gsignal.h gprogres.h
gfunct.o: /usr/include/math.h gmatrix.h grarray.h gblock.h rational.h
gfunct.o: integer.h
gclstats.o: gstatus.h gsignal.h gmisc.h gstring.h /usr/include/string.h
gclstats.o: /usr/include/sys/stdsyms.h gprogres.h gambitio.h
gclstats.o: /usr/include/stdio.h /usr/include/signal.h
gclstats.o: /usr/include/sys/signal.h /usr/include/sys/types.h
gclstats.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gclstats.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gclstats.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
glist.o: gmisc.h gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
glist.o: glist.imp glist.h gambitio.h /usr/include/stdio.h glistit.h
glist.o: /usr/include/assert.h rational.h integer.h /usr/include/math.h
glist.o: garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
glist.o: /usr/include/sys/types.h /usr/include/sys/signal.h
glist.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
glist.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
glist.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h gblock.h
subsolve.o: efg.h gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
subsolve.o: gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
subsolve.o: /usr/include/sys/types.h /usr/include/sys/signal.h
subsolve.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
subsolve.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
subsolve.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
subsolve.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
subsolve.o: gmisc.h behav.h efstrat.h gpvector.h gvector.h gdpvect.h
subsolve.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h
subsolve.o: node.h outcome.h efgutils.h glist.h glist.imp glistit.h
subsolve.o: subsolve.h seqform.h gmatrix.h grarray.h lemketab.h tableau.h
subsolve.o: ludecomp.h bfs.h gmap.h gmessage.h gstatus.h gsignal.h gprogres.h
subsolve.o: eliap.h liap.h nfg.h lemke.h mixed.h nfstrat.h nliap.h simpdiv.h
subsolve.o: enum.h lhtab.h vertenum.h purenash.h csum.h lpsolve.h gwatch.h
subsolve.o: garray.imp
readnfg.o: /usr/include/stdio.h /usr/include/ctype.h gmisc.h gstring.h
readnfg.o: /usr/include/string.h /usr/include/sys/stdsyms.h gambitio.h
readnfg.o: glist.h rational.h integer.h /usr/include/math.h nfg.h garray.h
readnfg.o: /usr/include/stdlib.h /usr/include/sys/wait.h
readnfg.o: /usr/include/sys/types.h /usr/include/sys/signal.h
readnfg.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
readnfg.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
readnfg.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
readnfg.o: /usr/include/assert.h gpvector.h gvector.h
readefg.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/sys/wait.h
readefg.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
readefg.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
readefg.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
readefg.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
readefg.o: /usr/include/machine/frame.h /usr/include/ctype.h gambitio.h
readefg.o: gmisc.h gstring.h /usr/include/string.h rational.h integer.h
readefg.o: /usr/include/math.h ggrstack.h gstack.h glist.h efg.h gblock.h
readefg.o: /usr/include/assert.h garray.h behav.h efstrat.h gpvector.h
readefg.o: gvector.h gdpvect.h efplayer.h infoset.h node.h outcome.h
readefg.o: gstack.imp ggrstack.imp
nfg.o: /usr/include/assert.h /usr/include/sys/stdsyms.h /usr/include/limits.h
nfg.o: /usr/include/sys/param.h /usr/include/sys/types.h
nfg.o: /usr/include/machine/param.h /usr/include/sys/sysmacros.h
nfg.o: /usr/include/machine/param_shm.h /usr/include/sys/time.h
nfg.o: /usr/include/sys/sigevent.h /usr/include/machine/spl.h nfg.h garray.h
nfg.o: /usr/include/stdlib.h /usr/include/sys/wait.h
nfg.o: /usr/include/sys/signal.h /usr/include/sys/siginfo.h
nfg.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
nfg.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h gambitio.h
nfg.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
nfg.o: gpvector.h gvector.h nfg.imp nfplayer.h nfstrat.h gblock.h nfgiter.h
nfg.o: mixed.h rational.h integer.h /usr/include/math.h garray.imp gblock.imp
nfgdbl.o: rational.h integer.h /usr/include/math.h nfg.h garray.h
nfgdbl.o: /usr/include/stdlib.h /usr/include/sys/wait.h
nfgdbl.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
nfgdbl.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
nfgdbl.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
nfgdbl.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
nfgdbl.o: /usr/include/machine/frame.h /usr/include/assert.h gambitio.h
nfgdbl.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
nfgdbl.o: gpvector.h gvector.h nfg.imp nfplayer.h nfstrat.h gblock.h
nfgdbl.o: nfgiter.h mixed.h nfgiter.imp contiter.imp contiter.h readnfg.imp
nfgdbl.o: glist.h readnfg.h garray.imp glist.imp glistit.h
nfgrat.o: rational.h integer.h /usr/include/math.h nfg.h garray.h
nfgrat.o: /usr/include/stdlib.h /usr/include/sys/wait.h
nfgrat.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
nfgrat.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
nfgrat.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
nfgrat.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
nfgrat.o: /usr/include/machine/frame.h /usr/include/assert.h gambitio.h
nfgrat.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
nfgrat.o: gpvector.h gvector.h nfg.imp nfplayer.h nfstrat.h gblock.h
nfgrat.o: nfgiter.h mixed.h nfgiter.imp contiter.imp contiter.h readnfg.imp
nfgrat.o: glist.h readnfg.h garray.imp glist.imp glistit.h
nfgutils.o: gmisc.h gstring.h /usr/include/string.h
nfgutils.o: /usr/include/sys/stdsyms.h nfg.h garray.h /usr/include/stdlib.h
nfgutils.o: /usr/include/sys/wait.h /usr/include/sys/types.h
nfgutils.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
nfgutils.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
nfgutils.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
nfgutils.o: /usr/include/machine/frame.h /usr/include/assert.h gambitio.h
nfgutils.o: /usr/include/stdio.h gpvector.h gvector.h rational.h integer.h
nfgutils.o: /usr/include/math.h
efg.o: garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
efg.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efg.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efg.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efg.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efg.o: /usr/include/machine/frame.h /usr/include/assert.h gambitio.h
efg.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
efg.o: rational.h integer.h /usr/include/math.h garray.imp gblock.imp
efg.o: gblock.h glist.imp glist.h glistit.h efg.h behav.h efstrat.h
efg.o: gpvector.h gvector.h gdpvect.h efplayer.h infoset.h node.h outcome.h
efg.o: efgutils.h
efgdbl.o: rational.h integer.h /usr/include/math.h glist.h gambitio.h
efgdbl.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
efgdbl.o: /usr/include/sys/stdsyms.h glistit.h efg.imp efg.h gblock.h
efgdbl.o: /usr/include/stdlib.h /usr/include/sys/wait.h
efgdbl.o: /usr/include/sys/types.h /usr/include/sys/signal.h
efgdbl.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
efgdbl.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
efgdbl.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
efgdbl.o: /usr/include/assert.h garray.h behav.h efstrat.h gpvector.h
efgdbl.o: gvector.h gdpvect.h efplayer.h infoset.h node.h outcome.h
efgdbl.o: efgutils.h tnode.h readefg.imp ggrstack.h gstack.h readefg.h
efgdbl.o: glist.imp
efgrat.o: rational.h integer.h /usr/include/math.h glist.h gambitio.h
efgrat.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
efgrat.o: /usr/include/sys/stdsyms.h glistit.h efg.imp efg.h gblock.h
efgrat.o: /usr/include/stdlib.h /usr/include/sys/wait.h
efgrat.o: /usr/include/sys/types.h /usr/include/sys/signal.h
efgrat.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
efgrat.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
efgrat.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
efgrat.o: /usr/include/assert.h garray.h behav.h efstrat.h gpvector.h
efgrat.o: gvector.h gdpvect.h efplayer.h infoset.h node.h outcome.h
efgrat.o: efgutils.h tnode.h readefg.imp ggrstack.h gstack.h readefg.h
efgrat.o: glist.imp
nfstrat.o: nfstrat.h gstring.h /usr/include/string.h
nfstrat.o: /usr/include/sys/stdsyms.h gblock.h /usr/include/stdlib.h
nfstrat.o: /usr/include/sys/wait.h /usr/include/sys/types.h
nfstrat.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
nfstrat.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
nfstrat.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
nfstrat.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
nfstrat.o: gambitio.h /usr/include/stdio.h gmisc.h nfplayer.h nfg.h
nfstrat.o: gpvector.h gvector.h
efgnfg.o: efg.h gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
efgnfg.o: gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
efgnfg.o: /usr/include/sys/types.h /usr/include/sys/signal.h
efgnfg.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
efgnfg.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
efgnfg.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
efgnfg.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
efgnfg.o: gmisc.h behav.h efstrat.h gpvector.h gvector.h gdpvect.h efplayer.h
efgnfg.o: infoset.h rational.h integer.h /usr/include/math.h node.h outcome.h
efgnfg.o: nfg.h nfplayer.h nfstrat.h mixed.h glist.h contiter.h nfgiter.h
efgnfg.o: glist.imp glistit.h garray.imp tnode.h
efgutils.o: efgutils.h efg.h gstring.h /usr/include/string.h
efgutils.o: /usr/include/sys/stdsyms.h gblock.h /usr/include/stdlib.h
efgutils.o: /usr/include/sys/wait.h /usr/include/sys/types.h
efgutils.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efgutils.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efgutils.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efgutils.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efgutils.o: gambitio.h /usr/include/stdio.h gmisc.h behav.h efstrat.h
efgutils.o: gpvector.h gvector.h gdpvect.h efplayer.h infoset.h rational.h
efgutils.o: integer.h /usr/include/math.h node.h outcome.h glist.h
efstrat.o: efg.h gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
efstrat.o: gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
efstrat.o: /usr/include/sys/types.h /usr/include/sys/signal.h
efstrat.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
efstrat.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
efstrat.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
efstrat.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
efstrat.o: gmisc.h behav.h efstrat.h gpvector.h gvector.h gdpvect.h
efstrat.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h
efstrat.o: node.h outcome.h
gobit.o: /usr/include/math.h gobit.h gambitio.h /usr/include/stdio.h gmisc.h
gobit.o: gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h gstatus.h
gobit.o: gsignal.h gprogres.h gvector.h garray.h /usr/include/stdlib.h
gobit.o: /usr/include/sys/wait.h /usr/include/sys/types.h
gobit.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
gobit.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
gobit.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
gobit.o: /usr/include/machine/frame.h /usr/include/assert.h gwatch.h
gobit.o: rational.h integer.h
egobit.o: /usr/include/math.h egobit.h efg.h gstring.h /usr/include/string.h
egobit.o: /usr/include/sys/stdsyms.h gblock.h /usr/include/stdlib.h
egobit.o: /usr/include/sys/wait.h /usr/include/sys/types.h
egobit.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
egobit.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
egobit.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
egobit.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
egobit.o: gambitio.h /usr/include/stdio.h gmisc.h behav.h efstrat.h
egobit.o: gpvector.h gvector.h gdpvect.h efplayer.h infoset.h rational.h
egobit.o: integer.h node.h outcome.h gobit.h gstatus.h gsignal.h gprogres.h
egobit.o: glist.h gfunct.h gmatrix.h grarray.h
liap.o: liap.h gambitio.h /usr/include/stdio.h gmisc.h gstring.h
liap.o: /usr/include/string.h /usr/include/sys/stdsyms.h gvector.h garray.h
liap.o: /usr/include/stdlib.h /usr/include/sys/wait.h
liap.o: /usr/include/sys/types.h /usr/include/sys/signal.h
liap.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
liap.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
liap.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
liap.o: /usr/include/assert.h gstatus.h gsignal.h gprogres.h gwatch.h
liap.o: rational.h integer.h /usr/include/math.h
eliap.o: /usr/include/math.h eliap.h efg.h gstring.h /usr/include/string.h
eliap.o: /usr/include/sys/stdsyms.h gblock.h /usr/include/stdlib.h
eliap.o: /usr/include/sys/wait.h /usr/include/sys/types.h
eliap.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
eliap.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
eliap.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
eliap.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
eliap.o: gambitio.h /usr/include/stdio.h gmisc.h behav.h efstrat.h gpvector.h
eliap.o: gvector.h gdpvect.h efplayer.h infoset.h rational.h integer.h node.h
eliap.o: outcome.h liap.h gstatus.h gsignal.h gprogres.h glist.h gfunct.h
eliap.o: gmatrix.h grarray.h
seqform.o: seqform.h efg.h gstring.h /usr/include/string.h
seqform.o: /usr/include/sys/stdsyms.h gblock.h /usr/include/stdlib.h
seqform.o: /usr/include/sys/wait.h /usr/include/sys/types.h
seqform.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
seqform.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
seqform.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
seqform.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
seqform.o: gambitio.h /usr/include/stdio.h gmisc.h behav.h efstrat.h
seqform.o: gpvector.h gvector.h gdpvect.h efplayer.h infoset.h rational.h
seqform.o: integer.h /usr/include/math.h node.h outcome.h glist.h gmatrix.h
seqform.o: grarray.h lemketab.h tableau.h ludecomp.h bfs.h gmap.h gmessage.h
seqform.o: gstatus.h gsignal.h gprogres.h gwatch.h
lemke.o: rational.h integer.h /usr/include/math.h gwatch.h gpvector.h
lemke.o: gvector.h gmisc.h gstring.h /usr/include/string.h
lemke.o: /usr/include/sys/stdsyms.h gambitio.h /usr/include/stdio.h garray.h
lemke.o: /usr/include/stdlib.h /usr/include/sys/wait.h
lemke.o: /usr/include/sys/types.h /usr/include/sys/signal.h
lemke.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
lemke.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
lemke.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
lemke.o: /usr/include/assert.h nfg.h nfgiter.h gtableau.h gmatrix.h grarray.h
lemke.o: gblock.h gmap.h gmessage.h bfs.h lemke.h mixed.h nfstrat.h glist.h
lemke.o: gstatus.h gsignal.h gprogres.h
nliap.o: /usr/include/math.h nliap.h nfg.h garray.h /usr/include/stdlib.h
nliap.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
nliap.o: /usr/include/sys/types.h /usr/include/sys/signal.h
nliap.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
nliap.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
nliap.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
nliap.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
nliap.o: gstring.h /usr/include/string.h gpvector.h gvector.h liap.h
nliap.o: gstatus.h gsignal.h gprogres.h glist.h mixed.h nfstrat.h gblock.h
nliap.o: gfunct.h
ngobit.o: /usr/include/math.h ngobit.h nfg.h garray.h /usr/include/stdlib.h
ngobit.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
ngobit.o: /usr/include/sys/types.h /usr/include/sys/signal.h
ngobit.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
ngobit.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
ngobit.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
ngobit.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
ngobit.o: gstring.h /usr/include/string.h gpvector.h gvector.h gobit.h
ngobit.o: gstatus.h gsignal.h gprogres.h glist.h mixed.h nfstrat.h gblock.h
ngobit.o: gfunct.h rational.h integer.h
enum.o: gwatch.h nfg.h garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
enum.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
enum.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
enum.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
enum.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
enum.o: /usr/include/machine/frame.h /usr/include/assert.h gambitio.h
enum.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
enum.o: gpvector.h gvector.h nfgiter.h enum.h glist.h gstatus.h gsignal.h
enum.o: gprogres.h lhtab.h lemketab.h tableau.h rational.h integer.h
enum.o: /usr/include/math.h ludecomp.h gmatrix.h grarray.h gblock.h bfs.h
enum.o: gmap.h gmessage.h mixed.h nfstrat.h vertenum.h glistit.h
simpdiv.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
simpdiv.o: /usr/include/string.h /usr/include/sys/stdsyms.h nfg.h garray.h
simpdiv.o: /usr/include/stdlib.h /usr/include/sys/wait.h
simpdiv.o: /usr/include/sys/types.h /usr/include/sys/signal.h
simpdiv.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
simpdiv.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
simpdiv.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
simpdiv.o: /usr/include/assert.h gpvector.h gvector.h grarray.h gwatch.h
simpdiv.o: simpdiv.h glist.h gstatus.h gsignal.h gprogres.h mixed.h nfstrat.h
simpdiv.o: gblock.h rational.h integer.h /usr/include/math.h
tableau.o: rational.h integer.h /usr/include/math.h tableau.imp tableau.h
tableau.o: ludecomp.h gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h
tableau.o: gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
tableau.o: grarray.h gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
tableau.o: /usr/include/sys/types.h /usr/include/sys/signal.h
tableau.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
tableau.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
tableau.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
tableau.o: /usr/include/assert.h garray.h gvector.h glist.h bfs.h gmap.h
tableau.o: gmessage.h normal.h gpvector.h normiter.h
ludecomp.o: ludecomp.imp ludecomp.h gmatrix.h gambitio.h /usr/include/stdio.h
ludecomp.o: gmisc.h gstring.h /usr/include/string.h
ludecomp.o: /usr/include/sys/stdsyms.h grarray.h gblock.h
ludecomp.o: /usr/include/stdlib.h /usr/include/sys/wait.h
ludecomp.o: /usr/include/sys/types.h /usr/include/sys/signal.h
ludecomp.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
ludecomp.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
ludecomp.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
ludecomp.o: /usr/include/assert.h garray.h gvector.h glist.h glistit.h
ludecomp.o: glist.imp rational.h integer.h /usr/include/math.h
purenash.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
purenash.o: /usr/include/string.h /usr/include/sys/stdsyms.h nfg.h garray.h
purenash.o: /usr/include/stdlib.h /usr/include/sys/wait.h
purenash.o: /usr/include/sys/types.h /usr/include/sys/signal.h
purenash.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
purenash.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
purenash.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
purenash.o: /usr/include/assert.h gpvector.h gvector.h nfgiter.h contiter.h
purenash.o: gblock.h rational.h integer.h /usr/include/math.h glist.h mixed.h
purenash.o: nfstrat.h
lhtab.o: lhtab.h lemketab.h tableau.h rational.h integer.h
lhtab.o: /usr/include/math.h ludecomp.h gmatrix.h gambitio.h
lhtab.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
lhtab.o: /usr/include/sys/stdsyms.h grarray.h gblock.h /usr/include/stdlib.h
lhtab.o: /usr/include/sys/wait.h /usr/include/sys/types.h
lhtab.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
lhtab.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
lhtab.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
lhtab.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
lhtab.o: gvector.h glist.h bfs.h gmap.h gmessage.h nfg.h gpvector.h nfgiter.h
lhtab.o: nfstrat.h
lemketab.o: lemketab.h tableau.h rational.h integer.h /usr/include/math.h
lemketab.o: ludecomp.h gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h
lemketab.o: gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
lemketab.o: grarray.h gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
lemketab.o: /usr/include/sys/types.h /usr/include/sys/signal.h
lemketab.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
lemketab.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
lemketab.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
lemketab.o: /usr/include/assert.h garray.h gvector.h glist.h bfs.h gmap.h
lemketab.o: gmessage.h
grid.o: gmisc.h gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
grid.o: rational.h integer.h /usr/include/math.h gmatrix.h gambitio.h
grid.o: /usr/include/stdio.h grarray.h gblock.h /usr/include/stdlib.h
grid.o: /usr/include/sys/wait.h /usr/include/sys/types.h
grid.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
grid.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
grid.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
grid.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h gvector.h
grid.o: nfg.h gpvector.h nfgiter.h nfstrat.h probvect.h gwatch.h grid.h
grid.o: gstatus.h gsignal.h gprogres.h grarray.imp garray.imp
csum.o: rational.h integer.h /usr/include/math.h gwatch.h gpvector.h
csum.o: gvector.h gmisc.h gstring.h /usr/include/string.h
csum.o: /usr/include/sys/stdsyms.h gambitio.h /usr/include/stdio.h garray.h
csum.o: /usr/include/stdlib.h /usr/include/sys/wait.h
csum.o: /usr/include/sys/types.h /usr/include/sys/signal.h
csum.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
csum.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
csum.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
csum.o: /usr/include/assert.h nfg.h nfgiter.h csum.h glist.h gstatus.h
csum.o: gsignal.h gprogres.h tableau.h ludecomp.h gmatrix.h grarray.h
csum.o: gblock.h bfs.h gmap.h gmessage.h lpsolve.h mixed.h nfstrat.h
lpsolve.o: lpsolve.imp lpsolve.h tableau.h rational.h integer.h
lpsolve.o: /usr/include/math.h ludecomp.h gmatrix.h gambitio.h
lpsolve.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
lpsolve.o: /usr/include/sys/stdsyms.h grarray.h gblock.h
lpsolve.o: /usr/include/stdlib.h /usr/include/sys/wait.h
lpsolve.o: /usr/include/sys/types.h /usr/include/sys/signal.h
lpsolve.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
lpsolve.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
lpsolve.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
lpsolve.o: /usr/include/assert.h garray.h gvector.h glist.h bfs.h gmap.h
lpsolve.o: gmessage.h
nfdom.o: nfg.h garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
nfdom.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
nfdom.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
nfdom.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
nfdom.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
nfdom.o: /usr/include/machine/frame.h /usr/include/assert.h gambitio.h
nfdom.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
nfdom.o: gpvector.h gvector.h nfstrat.h gblock.h contiter.h rational.h
nfdom.o: integer.h /usr/include/math.h
vertenum.o: vertenum.imp vertenum.h tableau.h rational.h integer.h
vertenum.o: /usr/include/math.h ludecomp.h gmatrix.h gambitio.h
vertenum.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
vertenum.o: /usr/include/sys/stdsyms.h grarray.h gblock.h
vertenum.o: /usr/include/stdlib.h /usr/include/sys/wait.h
vertenum.o: /usr/include/sys/types.h /usr/include/sys/signal.h
vertenum.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
vertenum.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
vertenum.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
vertenum.o: /usr/include/assert.h garray.h gvector.h glist.h bfs.h gmap.h
vertenum.o: gmessage.h gstatus.h gsignal.h gprogres.h
extshow.o: ./wx_fake/wx.h ./wx_fake/wx_form.h wxmisc.h /usr/include/stdio.h
extshow.o: ./wx_fake/wx_timer.h efg.h gstring.h /usr/include/string.h
extshow.o: /usr/include/sys/stdsyms.h gblock.h /usr/include/stdlib.h
extshow.o: /usr/include/sys/wait.h /usr/include/sys/types.h
extshow.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
extshow.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
extshow.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
extshow.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
extshow.o: gambitio.h gmisc.h behav.h efstrat.h gpvector.h gvector.h
extshow.o: gdpvect.h efplayer.h infoset.h rational.h integer.h
extshow.o: /usr/include/math.h node.h outcome.h efgconst.h gambit.h treewin.h
extshow.o: glist.h treedraw.h treecons.h gambdraw.h twflash.h extshow.h
extshow.o: efgnfgi.h accels.h extsoln.h spread.h grblock.h grarray.h
extshow.o: normgui.h wxio.h elimdomd.h delsolnd.h
treedraw.o: ./wx_fake/wx.h ./wx_fake/wx_form.h wxmisc.h /usr/include/stdio.h
treedraw.o: ./wx_fake/wx_timer.h treedraw.h gblock.h /usr/include/stdlib.h
treedraw.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
treedraw.o: /usr/include/sys/types.h /usr/include/sys/signal.h
treedraw.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
treedraw.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
treedraw.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
treedraw.o: /usr/include/assert.h garray.h gambitio.h gmisc.h gstring.h
treedraw.o: /usr/include/string.h treecons.h gambdraw.h legendc.h legend.h
twflash.o: ./wx_fake/wx.h twflash.h ./wx_fake/wx_timer.h
extsoln.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
extsoln.o: ./wx_fake/wx_form.h spread.h glist.h gambitio.h gmisc.h gstring.h
extsoln.o: /usr/include/string.h /usr/include/sys/stdsyms.h gblock.h
extsoln.o: /usr/include/stdlib.h /usr/include/sys/wait.h
extsoln.o: /usr/include/sys/types.h /usr/include/sys/signal.h
extsoln.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
extsoln.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
extsoln.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
extsoln.o: /usr/include/assert.h garray.h grblock.h grarray.h rational.h
extsoln.o: integer.h /usr/include/math.h extsoln.h efg.h behav.h efstrat.h
extsoln.o: gpvector.h gvector.h gdpvect.h efplayer.h infoset.h node.h
extsoln.o: outcome.h efgconst.h extshow.h efgnfgi.h gambit.h accels.h
extsoln.o: treedraw.h treecons.h gambdraw.h treewin.h twflash.h legendc.h
btreewni.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
btreewni.o: ./wx_fake/wx_form.h node.h rational.h integer.h
btreewni.o: /usr/include/math.h gblock.h /usr/include/stdlib.h
btreewni.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
btreewni.o: /usr/include/sys/types.h /usr/include/sys/signal.h
btreewni.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
btreewni.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
btreewni.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
btreewni.o: /usr/include/assert.h garray.h gambitio.h gmisc.h gstring.h
btreewni.o: /usr/include/string.h efg.h behav.h efstrat.h gpvector.h
btreewni.o: gvector.h gdpvect.h efplayer.h infoset.h outcome.h treewin.h
btreewni.o: glist.h treedraw.h treecons.h gambdraw.h twflash.h efgconst.h
btreewni.o: glist.imp glistit.h
bextshow.o: ./wx_fake/wx.h ./wx_fake/wx_form.h ./wx_fake/wx_tbar.h wxmisc.h
bextshow.o: /usr/include/stdio.h ./wx_fake/wx_timer.h efg.h gstring.h
bextshow.o: /usr/include/string.h /usr/include/sys/stdsyms.h gblock.h
bextshow.o: /usr/include/stdlib.h /usr/include/sys/wait.h
bextshow.o: /usr/include/sys/types.h /usr/include/sys/signal.h
bextshow.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
bextshow.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
bextshow.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
bextshow.o: /usr/include/assert.h garray.h gambitio.h gmisc.h behav.h
bextshow.o: efstrat.h gpvector.h gvector.h gdpvect.h efplayer.h infoset.h
bextshow.o: rational.h integer.h /usr/include/math.h node.h outcome.h
bextshow.o: efgconst.h gambit.h treewin.h glist.h treedraw.h treecons.h
bextshow.o: gambdraw.h twflash.h extshow.h efgnfgi.h accels.h extsoln.h
bextshow.o: spread.h grblock.h grarray.h efgaccl.h
normshow.o: normshow.h ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h
normshow.o: ./wx_fake/wx_timer.h ./wx_fake/wx_form.h wxio.h
normshow.o: /usr/include/assert.h /usr/include/sys/stdsyms.h gambitio.h
normshow.o: gmisc.h gstring.h /usr/include/string.h gambit.h spread.h glist.h
normshow.o: gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
normshow.o: /usr/include/sys/types.h /usr/include/sys/signal.h
normshow.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
normshow.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
normshow.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h garray.h
normshow.o: grblock.h grarray.h normgui.h normdraw.h gambdraw.h efgnfgi.h
normshow.o: accels.h rational.h integer.h /usr/include/math.h nfg.h
normshow.o: gpvector.h gvector.h nfgiter.h contiter.h normsoln.h nfgconst.h
normshow.o: nfplayer.h delsolnd.h nfgsolvd.h elimdomd.h nliap.h liap.h
normshow.o: gstatus.h gsignal.h gprogres.h mixed.h nfstrat.h liapprm.h
normshow.o: algdlgs.h simpdiv.h simpprm.h enum.h lhtab.h lemketab.h tableau.h
normshow.o: ludecomp.h gmatrix.h bfs.h gmap.h gmessage.h vertenum.h enumprm.h
normshow.o: csum.h lpsolve.h csumprm.h
bnormshw.o: normshow.h ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h
bnormshw.o: ./wx_fake/wx_timer.h ./wx_fake/wx_form.h wxio.h
bnormshw.o: /usr/include/assert.h /usr/include/sys/stdsyms.h gambitio.h
bnormshw.o: gmisc.h gstring.h /usr/include/string.h gambit.h spread.h glist.h
bnormshw.o: gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
bnormshw.o: /usr/include/sys/types.h /usr/include/sys/signal.h
bnormshw.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
bnormshw.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
bnormshw.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h garray.h
bnormshw.o: grblock.h grarray.h normgui.h normdraw.h gambdraw.h efgnfgi.h
bnormshw.o: accels.h rational.h integer.h /usr/include/math.h nfg.h
bnormshw.o: gpvector.h gvector.h nfgiter.h contiter.h normsoln.h nfplayer.h
bnormshw.o: normaccl.h nfgconst.h sprdaccl.h sprconst.h
wxmisc.o: ./wx_fake/wx.h ./wx_fake/wx_form.h ./wx_fake/wx_help.h wxmisc.h
wxmisc.o: /usr/include/stdio.h ./wx_fake/wx_timer.h general.h
wxmisc.o: /usr/include/stdlib.h /usr/include/sys/wait.h
wxmisc.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
wxmisc.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
wxmisc.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
wxmisc.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
wxmisc.o: /usr/include/machine/frame.h /usr/include/string.h
wximpl.o: garray.imp /usr/include/stdlib.h /usr/include/sys/wait.h
wximpl.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
wximpl.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
wximpl.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
wximpl.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
wximpl.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
wximpl.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
wximpl.o: /usr/include/string.h glist.imp glist.h glistit.h
gambdraw.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
gambdraw.o: ./wx_fake/wx_form.h gambdraw.h gblock.h /usr/include/stdlib.h
gambdraw.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
gambdraw.o: /usr/include/sys/types.h /usr/include/sys/signal.h
gambdraw.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gambdraw.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gambdraw.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gambdraw.o: /usr/include/assert.h garray.h gambitio.h gmisc.h gstring.h
gambdraw.o: /usr/include/string.h
wxio.o: ./wx_fake/wx.h wxio.h /usr/include/stdio.h /usr/include/assert.h
wxio.o: /usr/include/sys/stdsyms.h gambitio.h gmisc.h gstring.h
wxio.o: /usr/include/string.h
spread.o: /usr/include/stdio.h ./wx_fake/wx.h ./wx_fake/wx_mf.h
spread.o: ./wx_fake/wx_tbar.h general.h /usr/include/stdlib.h
spread.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
spread.o: /usr/include/sys/types.h /usr/include/sys/signal.h
spread.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
spread.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
spread.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
spread.o: /usr/include/string.h wxmisc.h ./wx_fake/wx_timer.h
spread.o: ./wx_fake/wx_form.h spread.h glist.h gambitio.h gmisc.h gstring.h
spread.o: gblock.h /usr/include/assert.h garray.h grblock.h grarray.h
spreadim.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
spreadim.o: ./wx_fake/wx_form.h grblock.h grarray.h gambitio.h gmisc.h
spreadim.o: gstring.h /usr/include/string.h /usr/include/sys/stdsyms.h
spreadim.o: garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
spreadim.o: /usr/include/sys/types.h /usr/include/sys/signal.h
spreadim.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
spreadim.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
spreadim.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
spreadim.o: /usr/include/assert.h spread.h glist.h gblock.h rational.h
spreadim.o: integer.h /usr/include/math.h glist.imp glistit.h grarray.imp
spreadim.o: grblock.imp garray.imp
gambit.o: /usr/include/assert.h /usr/include/sys/stdsyms.h
gambit.o: /usr/include/string.h /usr/include/ctype.h ./wx_fake/wx.h
gambit.o: ./wx_fake/wx_tbar.h wxio.h /usr/include/stdio.h gambitio.h gmisc.h
gambit.o: gstring.h gambit.h wxmisc.h ./wx_fake/wx_timer.h
gambit.o: ./wx_fake/wx_form.h normgui.h extgui.h /usr/include/signal.h
gambit.o: /usr/include/sys/signal.h /usr/include/sys/types.h
gambit.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gambit.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gambit.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gambit.o: /usr/include/math.h
accels.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
accels.o: ./wx_fake/wx_form.h keynames.h gmisc.h gstring.h
accels.o: /usr/include/string.h /usr/include/sys/stdsyms.h glist.imp glist.h
accels.o: gambitio.h glistit.h /usr/include/assert.h accels.h garray.h
accels.o: /usr/include/stdlib.h /usr/include/sys/wait.h
accels.o: /usr/include/sys/types.h /usr/include/sys/signal.h
accels.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
accels.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
accels.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h garray.imp
wxstatus.o: ./wx_fake/wx.h wxstatus.h gstatus.h gsignal.h gmisc.h gstring.h
wxstatus.o: /usr/include/string.h /usr/include/sys/stdsyms.h gprogres.h
wxstatus.o: gambitio.h /usr/include/stdio.h
efgnfgi.o: efgnfgi.h rational.h integer.h /usr/include/math.h
general.o: general.h /usr/include/stdio.h /usr/include/stdlib.h
general.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
general.o: /usr/include/sys/types.h /usr/include/sys/signal.h
general.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
general.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
general.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
general.o: /usr/include/string.h
