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
		psnesub.o efgpure.o lemkesub.o liapsub.o simpsub.o \
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
gmisc.o: /usr/include/sys/stdsyms.h /usr/include/stdio.h
gmisc.o: /usr/include/stdlib.h /usr/include/sys/wait.h
gmisc.o: /usr/include/sys/types.h /usr/include/sys/signal.h
gmisc.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gmisc.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gmisc.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gmisc.o: /usr/include/limits.h /usr/include/sys/param.h
gmisc.o: /usr/include/machine/param.h /usr/include/sys/sysmacros.h
gmisc.o: /usr/include/machine/param_shm.h /usr/include/sys/time.h
gmisc.o: /usr/include/machine/spl.h /usr/include/ctype.h gambitio.h gstring.h
gmisc.o: /usr/include/string.h
gambitio.o: /usr/include/assert.h /usr/include/sys/stdsyms.h gambitio.h
gambitio.o: /usr/include/stdio.h gmisc.h
garray.o: garray.imp /usr/include/stdlib.h /usr/include/sys/wait.h
garray.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
garray.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
garray.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
garray.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
garray.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
garray.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
garray.o: /usr/include/string.h rational.h integer.h /usr/include/math.h
gblock.o: gmisc.h gblock.imp /usr/include/stdlib.h /usr/include/sys/wait.h
gblock.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
gblock.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
gblock.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
gblock.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
gblock.o: /usr/include/machine/frame.h /usr/include/assert.h gblock.h
gblock.o: garray.h gambitio.h /usr/include/stdio.h gstring.h
gblock.o: /usr/include/string.h rational.h integer.h /usr/include/math.h
gblock.o: garray.imp
gstring.o: /usr/include/stdlib.h /usr/include/sys/wait.h
gstring.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
gstring.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
gstring.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
gstring.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
gstring.o: /usr/include/machine/frame.h /usr/include/assert.h
gstring.o: /usr/include/ctype.h gambitio.h /usr/include/stdio.h gmisc.h
gstring.o: gstring.h /usr/include/string.h
integer.o: gambitio.h /usr/include/stdio.h gmisc.h integer.h gnulib.h
integer.o: /usr/include/stddef.h /usr/include/sys/stdsyms.h
integer.o: /usr/include/stdlib.h /usr/include/sys/wait.h
integer.o: /usr/include/sys/types.h /usr/include/sys/signal.h
integer.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
integer.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
integer.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
integer.o: /usr/include/string.h /usr/include/memory.h /usr/include/errno.h
integer.o: /usr/include/sys/errno.h /usr/include/fcntl.h
integer.o: /usr/include/sys/fcntl.h /usr/include/math.h /usr/include/ctype.h
integer.o: ./wx_fake/float.h /usr/include/limits.h /usr/include/sys/param.h
integer.o: /usr/include/machine/param.h /usr/include/sys/sysmacros.h
integer.o: /usr/include/machine/param_shm.h /usr/include/sys/time.h
integer.o: /usr/include/machine/spl.h /usr/include/assert.h
rational.o: gambitio.h /usr/include/stdio.h gmisc.h rational.h integer.h
rational.o: /usr/include/math.h /usr/include/sys/stdsyms.h gnulib.h
rational.o: /usr/include/stddef.h /usr/include/stdlib.h
rational.o: /usr/include/sys/wait.h /usr/include/sys/types.h
rational.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
rational.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
rational.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
rational.o: /usr/include/machine/frame.h /usr/include/string.h
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
gvector.o: gvector.imp gvector.h gmisc.h gambitio.h /usr/include/stdio.h
gvector.o: garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
gvector.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
gvector.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
gvector.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
gvector.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
gvector.o: /usr/include/machine/frame.h /usr/include/assert.h rational.h
gvector.o: integer.h /usr/include/math.h
gpvector.o: gpvector.imp gpvector.h gvector.h gmisc.h gambitio.h
gpvector.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
gpvector.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
gpvector.o: /usr/include/sys/types.h /usr/include/sys/signal.h
gpvector.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gpvector.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gpvector.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gpvector.o: /usr/include/assert.h rational.h integer.h /usr/include/math.h
gdpvect.o: gdpvect.imp gdpvect.h gpvector.h gvector.h gmisc.h gambitio.h
gdpvect.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
gdpvect.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
gdpvect.o: /usr/include/sys/types.h /usr/include/sys/signal.h
gdpvect.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gdpvect.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gdpvect.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gdpvect.o: /usr/include/assert.h rational.h integer.h /usr/include/math.h
grarray.o: grarray.imp grarray.h gambitio.h /usr/include/stdio.h gmisc.h
grarray.o: /usr/include/assert.h /usr/include/sys/stdsyms.h garray.h
grarray.o: /usr/include/stdlib.h /usr/include/sys/wait.h
grarray.o: /usr/include/sys/types.h /usr/include/sys/signal.h
grarray.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
grarray.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
grarray.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h rational.h
grarray.o: integer.h /usr/include/math.h
gmatrix.o: gmatrix.imp gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h
gmatrix.o: grarray.h gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
gmatrix.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
gmatrix.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
gmatrix.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
gmatrix.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
gmatrix.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
gmatrix.o: gvector.h rational.h integer.h /usr/include/math.h
gclsig.o: gsignal.h gmisc.h /usr/include/signal.h /usr/include/sys/signal.h
gclsig.o: /usr/include/sys/types.h /usr/include/sys/stdsyms.h
gclsig.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gclsig.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gclsig.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
glpsolve.o: glpsolve.imp glpsolve.h gtableau.h /usr/include/assert.h
glpsolve.o: /usr/include/sys/stdsyms.h gmisc.h gambitio.h
glpsolve.o: /usr/include/stdio.h gmatrix.h grarray.h gblock.h
glpsolve.o: /usr/include/stdlib.h /usr/include/sys/wait.h
glpsolve.o: /usr/include/sys/types.h /usr/include/sys/signal.h
glpsolve.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
glpsolve.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
glpsolve.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h garray.h
glpsolve.o: gvector.h gmap.h bfs.h rational.h integer.h /usr/include/math.h
glpsolve.o: gtableau.imp
bfs.o: rational.h integer.h /usr/include/math.h /usr/include/sys/stdsyms.h
bfs.o: garray.imp /usr/include/stdlib.h /usr/include/sys/wait.h
bfs.o: /usr/include/sys/types.h /usr/include/sys/signal.h
bfs.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
bfs.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
bfs.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
bfs.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h gmisc.h
bfs.o: gblock.imp gblock.h glist.imp glist.h gmap.imp gmap.h
bfs.o: /usr/include/string.h bfs.h
gwatch.o: gwatch.h /usr/include/stdio.h /usr/include/math.h
gwatch.o: /usr/include/sys/stdsyms.h
gfunc.o: gfunc.h gmisc.h
gclstats.o: gstatus.h gsignal.h gmisc.h gprogres.h gambitio.h
gclstats.o: /usr/include/stdio.h gstring.h /usr/include/string.h
gclstats.o: /usr/include/signal.h /usr/include/sys/signal.h
gclstats.o: /usr/include/sys/types.h /usr/include/sys/stdsyms.h
gclstats.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gclstats.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gclstats.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
glist.o: gmisc.h glist.imp glist.h gambitio.h /usr/include/stdio.h
glist.o: /usr/include/assert.h /usr/include/sys/stdsyms.h gstring.h
glist.o: /usr/include/string.h rational.h integer.h /usr/include/math.h
glist.o: garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
glist.o: /usr/include/sys/types.h /usr/include/sys/signal.h
glist.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
glist.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
glist.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h gblock.h
subsolve.o: efg.h gstring.h /usr/include/string.h gblock.h
subsolve.o: /usr/include/stdlib.h /usr/include/sys/wait.h
subsolve.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
subsolve.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
subsolve.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
subsolve.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
subsolve.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
subsolve.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h
subsolve.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h
subsolve.o: rational.h integer.h /usr/include/math.h node.h outcome.h
subsolve.o: efgutils.h nfg.h nfstrat.h gwatch.h subsolve.imp subsolve.h
subsolve.o: behavsol.h garray.imp glist.imp
gfuncmin.o: /usr/include/math.h /usr/include/sys/stdsyms.h gfunc.h gmisc.h
gfuncmin.o: gvector.h gambitio.h /usr/include/stdio.h garray.h
gfuncmin.o: /usr/include/stdlib.h /usr/include/sys/wait.h
gfuncmin.o: /usr/include/sys/types.h /usr/include/sys/signal.h
gfuncmin.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gfuncmin.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gfuncmin.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gfuncmin.o: /usr/include/assert.h gmatrix.h grarray.h gblock.h
readnfg.o: /usr/include/stdio.h /usr/include/ctype.h gmisc.h gambitio.h
readnfg.o: glist.h rational.h integer.h /usr/include/math.h
readnfg.o: /usr/include/sys/stdsyms.h nfg.h garray.h /usr/include/stdlib.h
readnfg.o: /usr/include/sys/wait.h /usr/include/sys/types.h
readnfg.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
readnfg.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
readnfg.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
readnfg.o: /usr/include/machine/frame.h /usr/include/assert.h gstring.h
readnfg.o: /usr/include/string.h gpvector.h gvector.h
readefg.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/sys/wait.h
readefg.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
readefg.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
readefg.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
readefg.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
readefg.o: /usr/include/machine/frame.h /usr/include/ctype.h gambitio.h
readefg.o: gmisc.h gstring.h /usr/include/string.h rational.h integer.h
readefg.o: /usr/include/math.h gstack.h glist.h efg.h gblock.h
readefg.o: /usr/include/assert.h garray.h gpvector.h gvector.h behav.h
readefg.o: efstrat.h gdpvect.h efplayer.h infoset.h node.h outcome.h
readefg.o: gstack.imp
nfg.o: /usr/include/assert.h /usr/include/sys/stdsyms.h nfg.h garray.h
nfg.o: /usr/include/stdlib.h /usr/include/sys/wait.h /usr/include/sys/types.h
nfg.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
nfg.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
nfg.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
nfg.o: /usr/include/machine/frame.h gambitio.h /usr/include/stdio.h gmisc.h
nfg.o: gstring.h /usr/include/string.h gpvector.h gvector.h nfstrat.h
nfg.o: gblock.h nfplayer.h mixed.h garray.imp gblock.imp
nfgdbl.o: rational.h integer.h /usr/include/math.h /usr/include/sys/stdsyms.h
nfgdbl.o: nfg.h garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
nfgdbl.o: /usr/include/sys/types.h /usr/include/sys/signal.h
nfgdbl.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
nfgdbl.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
nfgdbl.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
nfgdbl.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
nfgdbl.o: gstring.h /usr/include/string.h gpvector.h gvector.h nfg.imp
nfgdbl.o: nfplayer.h nfstrat.h gblock.h nfgiter.h mixed.h nfgiter.imp
nfgdbl.o: nfgciter.imp nfgciter.h readnfg.imp glist.h readnfg.h mixedsol.imp
nfgdbl.o: mixedsol.h garray.imp glist.imp
nfgrat.o: rational.h integer.h /usr/include/math.h /usr/include/sys/stdsyms.h
nfgrat.o: nfg.h garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
nfgrat.o: /usr/include/sys/types.h /usr/include/sys/signal.h
nfgrat.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
nfgrat.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
nfgrat.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
nfgrat.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
nfgrat.o: gstring.h /usr/include/string.h gpvector.h gvector.h nfg.imp
nfgrat.o: nfplayer.h nfstrat.h gblock.h nfgiter.h mixed.h nfgiter.imp
nfgrat.o: nfgciter.imp nfgciter.h readnfg.imp glist.h readnfg.h mixedsol.imp
nfgrat.o: mixedsol.h garray.imp glist.imp
nfgutils.o: gmisc.h nfg.h garray.h /usr/include/stdlib.h
nfgutils.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
nfgutils.o: /usr/include/sys/types.h /usr/include/sys/signal.h
nfgutils.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
nfgutils.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
nfgutils.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
nfgutils.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gstring.h
nfgutils.o: /usr/include/string.h gpvector.h gvector.h rational.h integer.h
nfgutils.o: /usr/include/math.h
efg.o: garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
efg.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efg.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efg.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efg.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efg.o: /usr/include/machine/frame.h /usr/include/assert.h gambitio.h
efg.o: /usr/include/stdio.h gmisc.h rational.h integer.h /usr/include/math.h
efg.o: garray.imp gblock.imp gblock.h glist.imp glist.h efg.h gstring.h
efg.o: /usr/include/string.h gpvector.h gvector.h behav.h efstrat.h gdpvect.h
efg.o: efplayer.h infoset.h node.h outcome.h efgutils.h
efgdbl.o: rational.h integer.h /usr/include/math.h /usr/include/sys/stdsyms.h
efgdbl.o: glist.h gambitio.h /usr/include/stdio.h gmisc.h efg.imp efg.h
efgdbl.o: gstring.h /usr/include/string.h gblock.h /usr/include/stdlib.h
efgdbl.o: /usr/include/sys/wait.h /usr/include/sys/types.h
efgdbl.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efgdbl.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efgdbl.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efgdbl.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efgdbl.o: gpvector.h gvector.h behav.h efstrat.h gdpvect.h efplayer.h
efgdbl.o: infoset.h node.h outcome.h efgutils.h tnode.h behavsol.imp
efgdbl.o: behavsol.h efgiter.imp efgciter.h efgiter.h efgciter.imp
efgdbl.o: readefg.imp gstack.h readefg.h glist.imp
efgrat.o: rational.h integer.h /usr/include/math.h /usr/include/sys/stdsyms.h
efgrat.o: glist.h gambitio.h /usr/include/stdio.h gmisc.h efg.imp efg.h
efgrat.o: gstring.h /usr/include/string.h gblock.h /usr/include/stdlib.h
efgrat.o: /usr/include/sys/wait.h /usr/include/sys/types.h
efgrat.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efgrat.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efgrat.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efgrat.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efgrat.o: gpvector.h gvector.h behav.h efstrat.h gdpvect.h efplayer.h
efgrat.o: infoset.h node.h outcome.h efgutils.h tnode.h behavsol.imp
efgrat.o: behavsol.h efgiter.imp efgciter.h efgiter.h efgciter.imp
efgrat.o: readefg.imp gstack.h readefg.h glist.imp
nfstrat.o: nfstrat.h gstring.h /usr/include/string.h gblock.h
nfstrat.o: /usr/include/stdlib.h /usr/include/sys/wait.h
nfstrat.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
nfstrat.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
nfstrat.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
nfstrat.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
nfstrat.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
nfstrat.o: gambitio.h /usr/include/stdio.h gmisc.h nfplayer.h nfg.h
nfstrat.o: gpvector.h gvector.h
efgnfgr.o: efgnfg.imp tnode.h efg.h gstring.h /usr/include/string.h gblock.h
efgnfgr.o: /usr/include/stdlib.h /usr/include/sys/wait.h
efgnfgr.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efgnfgr.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efgnfgr.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efgnfgr.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efgnfgr.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efgnfgr.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h
efgnfgr.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h
efgnfgr.o: rational.h integer.h /usr/include/math.h node.h outcome.h nfg.h
efgnfgr.o: nfplayer.h nfstrat.h nfgiter.h nfgciter.h mixed.h lexicon.h
efgutils.o: efgutils.h efg.h gstring.h /usr/include/string.h gblock.h
efgutils.o: /usr/include/stdlib.h /usr/include/sys/wait.h
efgutils.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efgutils.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efgutils.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efgutils.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efgutils.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efgutils.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h
efgutils.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h
efgutils.o: rational.h integer.h /usr/include/math.h node.h outcome.h
efstrat.o: efg.h gstring.h /usr/include/string.h gblock.h
efstrat.o: /usr/include/stdlib.h /usr/include/sys/wait.h
efstrat.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efstrat.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efstrat.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efstrat.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efstrat.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efstrat.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h
efstrat.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h
efstrat.o: rational.h integer.h /usr/include/math.h node.h outcome.h
efgnfgd.o: efgnfg.imp tnode.h efg.h gstring.h /usr/include/string.h gblock.h
efgnfgd.o: /usr/include/stdlib.h /usr/include/sys/wait.h
efgnfgd.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efgnfgd.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efgnfgd.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efgnfgd.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efgnfgd.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efgnfgd.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h
efgnfgd.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h
efgnfgd.o: rational.h integer.h /usr/include/math.h node.h outcome.h nfg.h
efgnfgd.o: nfplayer.h nfstrat.h nfgiter.h nfgciter.h mixed.h lexicon.h
efdom.o: efg.h gstring.h /usr/include/string.h gblock.h /usr/include/stdlib.h
efdom.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
efdom.o: /usr/include/sys/types.h /usr/include/sys/signal.h
efdom.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
efdom.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
efdom.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
efdom.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
efdom.o: gmisc.h glist.h gpvector.h gvector.h behav.h efstrat.h gdpvect.h
efdom.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h node.h
efdom.o: outcome.h efgciter.h
egobit.o: /usr/include/math.h /usr/include/sys/stdsyms.h egobit.h gambitio.h
egobit.o: /usr/include/stdio.h gmisc.h gstatus.h gsignal.h gprogres.h
egobit.o: gstring.h /usr/include/string.h glist.h efg.h gblock.h
egobit.o: /usr/include/stdlib.h /usr/include/sys/wait.h
egobit.o: /usr/include/sys/types.h /usr/include/sys/signal.h
egobit.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
egobit.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
egobit.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
egobit.o: /usr/include/assert.h garray.h gpvector.h gvector.h behav.h
egobit.o: efstrat.h gdpvect.h efplayer.h infoset.h rational.h integer.h
egobit.o: node.h outcome.h behavsol.h gfunc.h gmatrix.h grarray.h
eliap.o: eliap.h gambitio.h /usr/include/stdio.h gmisc.h gstatus.h gsignal.h
eliap.o: gprogres.h gstring.h /usr/include/string.h glist.h efg.h gblock.h
eliap.o: /usr/include/stdlib.h /usr/include/sys/wait.h
eliap.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
eliap.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
eliap.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
eliap.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
eliap.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
eliap.o: gpvector.h gvector.h behav.h efstrat.h gdpvect.h efplayer.h
eliap.o: infoset.h rational.h integer.h /usr/include/math.h node.h outcome.h
eliap.o: behavsol.h subsolve.h nfg.h gfunc.h gmatrix.h grarray.h
seqform.o: seqform.imp seqform.h efg.h gstring.h /usr/include/string.h
seqform.o: gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
seqform.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
seqform.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
seqform.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
seqform.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
seqform.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
seqform.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h
seqform.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h
seqform.o: rational.h integer.h /usr/include/math.h node.h outcome.h
seqform.o: gmatrix.h grarray.h lemketab.h tableau.h ludecomp.h gwatch.h bfs.h
seqform.o: gmap.h gstatus.h gsignal.h gprogres.h behavsol.h subsolve.h nfg.h
efgcsum.o: efgcsum.imp gwatch.h gpvector.h gvector.h gmisc.h gambitio.h
efgcsum.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
efgcsum.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
efgcsum.o: /usr/include/sys/types.h /usr/include/sys/signal.h
efgcsum.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
efgcsum.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
efgcsum.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
efgcsum.o: /usr/include/assert.h efgcsum.h efg.h gstring.h
efgcsum.o: /usr/include/string.h gblock.h glist.h behav.h efstrat.h gdpvect.h
efgcsum.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h
efgcsum.o: node.h outcome.h gstatus.h gsignal.h gprogres.h tableau.h
efgcsum.o: ludecomp.h gmatrix.h grarray.h bfs.h gmap.h lpsolve.h behavsol.h
efgcsum.o: subsolve.h nfg.h
psnesub.o: rational.h integer.h /usr/include/math.h
psnesub.o: /usr/include/sys/stdsyms.h psnesub.imp psnesub.h subsolve.h efg.h
psnesub.o: gstring.h /usr/include/string.h gblock.h /usr/include/stdlib.h
psnesub.o: /usr/include/sys/wait.h /usr/include/sys/types.h
psnesub.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
psnesub.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
psnesub.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
psnesub.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
psnesub.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h
psnesub.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h
psnesub.o: outcome.h nfg.h behavsol.h nfgpure.h mixed.h nfstrat.h mixedsol.h
efgpure.o: efgpure.imp glist.h gambitio.h /usr/include/stdio.h gmisc.h efg.h
efgpure.o: gstring.h /usr/include/string.h gblock.h /usr/include/stdlib.h
efgpure.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
efgpure.o: /usr/include/sys/types.h /usr/include/sys/signal.h
efgpure.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
efgpure.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
efgpure.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
efgpure.o: /usr/include/assert.h garray.h gpvector.h gvector.h behav.h
efgpure.o: efstrat.h gdpvect.h efplayer.h infoset.h rational.h integer.h
efgpure.o: /usr/include/math.h node.h outcome.h efgiter.h efgciter.h
efgpure.o: behavsol.h efgpure.h subsolve.h nfg.h
lemkesub.o: rational.h integer.h /usr/include/math.h
lemkesub.o: /usr/include/sys/stdsyms.h lemkesub.imp lemkesub.h lemke.h nfg.h
lemkesub.o: garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
lemkesub.o: /usr/include/sys/types.h /usr/include/sys/signal.h
lemkesub.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
lemkesub.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
lemkesub.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
lemkesub.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
lemkesub.o: gstring.h /usr/include/string.h gpvector.h gvector.h mixedsol.h
lemkesub.o: mixed.h nfstrat.h gblock.h glist.h lhtab.h lemketab.h tableau.h
lemkesub.o: ludecomp.h gmatrix.h grarray.h gwatch.h bfs.h gmap.h gstatus.h
lemkesub.o: gsignal.h gprogres.h subsolve.h efg.h behav.h efstrat.h gdpvect.h
lemkesub.o: efplayer.h infoset.h node.h outcome.h behavsol.h
liapsub.o: liapsub.h nliap.h gambitio.h /usr/include/stdio.h gmisc.h
liapsub.o: gstatus.h gsignal.h gprogres.h gstring.h /usr/include/string.h
liapsub.o: glist.h nfg.h garray.h /usr/include/stdlib.h
liapsub.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
liapsub.o: /usr/include/sys/types.h /usr/include/sys/signal.h
liapsub.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
liapsub.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
liapsub.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
liapsub.o: /usr/include/assert.h gpvector.h gvector.h mixed.h nfstrat.h
liapsub.o: gblock.h mixedsol.h subsolve.h efg.h behav.h efstrat.h gdpvect.h
liapsub.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h
liapsub.o: node.h outcome.h behavsol.h
simpsub.o: rational.h integer.h /usr/include/math.h
simpsub.o: /usr/include/sys/stdsyms.h simpsub.imp simpsub.h simpdiv.h nfg.h
simpsub.o: garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
simpsub.o: /usr/include/sys/types.h /usr/include/sys/signal.h
simpsub.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
simpsub.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
simpsub.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
simpsub.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
simpsub.o: gstring.h /usr/include/string.h gpvector.h gvector.h glist.h
simpsub.o: grarray.h gstatus.h gsignal.h gprogres.h mixed.h nfstrat.h
simpsub.o: gblock.h mixedsol.h subsolve.h efg.h behav.h efstrat.h gdpvect.h
simpsub.o: efplayer.h infoset.h node.h outcome.h behavsol.h
enumsub.o: rational.h integer.h /usr/include/math.h
enumsub.o: /usr/include/sys/stdsyms.h enumsub.imp enumsub.h subsolve.h efg.h
enumsub.o: gstring.h /usr/include/string.h gblock.h /usr/include/stdlib.h
enumsub.o: /usr/include/sys/wait.h /usr/include/sys/types.h
enumsub.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
enumsub.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
enumsub.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
enumsub.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
enumsub.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h
enumsub.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h
enumsub.o: outcome.h nfg.h behavsol.h enum.h gstatus.h gsignal.h gprogres.h
enumsub.o: lhtab.h lemketab.h tableau.h ludecomp.h gmatrix.h grarray.h
enumsub.o: gwatch.h bfs.h gmap.h mixed.h nfstrat.h mixedsol.h vertenum.h
csumsub.o: rational.h integer.h /usr/include/math.h
csumsub.o: /usr/include/sys/stdsyms.h csumsub.imp csumsub.h subsolve.h efg.h
csumsub.o: gstring.h /usr/include/string.h gblock.h /usr/include/stdlib.h
csumsub.o: /usr/include/sys/wait.h /usr/include/sys/types.h
csumsub.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
csumsub.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
csumsub.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
csumsub.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
csumsub.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h
csumsub.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h
csumsub.o: outcome.h nfg.h behavsol.h nfgcsum.h gstatus.h gsignal.h
csumsub.o: gprogres.h tableau.h ludecomp.h gmatrix.h grarray.h gwatch.h bfs.h
csumsub.o: gmap.h lpsolve.h mixed.h nfstrat.h mixedsol.h
behavsol.o: behavsol.h gmisc.h behav.h gstring.h /usr/include/string.h
behavsol.o: efstrat.h gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
behavsol.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
behavsol.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
behavsol.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
behavsol.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
behavsol.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
behavsol.o: gambitio.h /usr/include/stdio.h efg.h glist.h gpvector.h
behavsol.o: gvector.h efplayer.h infoset.h rational.h integer.h
behavsol.o: /usr/include/math.h node.h outcome.h gdpvect.h
lemke.o: lemke.imp gwatch.h gpvector.h gvector.h gmisc.h gambitio.h
lemke.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
lemke.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
lemke.o: /usr/include/sys/types.h /usr/include/sys/signal.h
lemke.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
lemke.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
lemke.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
lemke.o: /usr/include/assert.h nfg.h gstring.h /usr/include/string.h lemke.h
lemke.o: mixedsol.h mixed.h nfstrat.h gblock.h glist.h lhtab.h lemketab.h
lemke.o: tableau.h rational.h integer.h /usr/include/math.h ludecomp.h
lemke.o: gmatrix.h grarray.h bfs.h gmap.h gstatus.h gsignal.h gprogres.h
nliap.o: nliap.h gambitio.h /usr/include/stdio.h gmisc.h gstatus.h gsignal.h
nliap.o: gprogres.h gstring.h /usr/include/string.h glist.h nfg.h garray.h
nliap.o: /usr/include/stdlib.h /usr/include/sys/wait.h
nliap.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
nliap.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
nliap.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
nliap.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
nliap.o: /usr/include/machine/frame.h /usr/include/assert.h gpvector.h
nliap.o: gvector.h mixed.h nfstrat.h gblock.h mixedsol.h gfunc.h
ngobit.o: /usr/include/math.h /usr/include/sys/stdsyms.h ngobit.h gambitio.h
ngobit.o: /usr/include/stdio.h gmisc.h gstatus.h gsignal.h gprogres.h
ngobit.o: gstring.h /usr/include/string.h glist.h nfg.h garray.h
ngobit.o: /usr/include/stdlib.h /usr/include/sys/wait.h
ngobit.o: /usr/include/sys/types.h /usr/include/sys/signal.h
ngobit.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
ngobit.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
ngobit.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
ngobit.o: /usr/include/assert.h gpvector.h gvector.h mixed.h nfstrat.h
ngobit.o: gblock.h mixedsol.h gfunc.h
enum.o: enum.imp gwatch.h nfg.h garray.h /usr/include/stdlib.h
enum.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
enum.o: /usr/include/sys/types.h /usr/include/sys/signal.h
enum.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
enum.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
enum.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
enum.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
enum.o: gstring.h /usr/include/string.h gpvector.h gvector.h nfgiter.h enum.h
enum.o: glist.h gstatus.h gsignal.h gprogres.h lhtab.h lemketab.h tableau.h
enum.o: rational.h integer.h /usr/include/math.h ludecomp.h gmatrix.h
enum.o: grarray.h gblock.h bfs.h gmap.h mixed.h nfstrat.h mixedsol.h
enum.o: vertenum.h
simpdiv.o: simpdiv.imp gambitio.h /usr/include/stdio.h gmisc.h nfg.h garray.h
simpdiv.o: /usr/include/stdlib.h /usr/include/sys/wait.h
simpdiv.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
simpdiv.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
simpdiv.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
simpdiv.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
simpdiv.o: /usr/include/machine/frame.h /usr/include/assert.h gstring.h
simpdiv.o: /usr/include/string.h gpvector.h gvector.h grarray.h gwatch.h
simpdiv.o: simpdiv.h glist.h gstatus.h gsignal.h gprogres.h mixed.h nfstrat.h
simpdiv.o: gblock.h mixedsol.h rational.h integer.h /usr/include/math.h
tableau.o: rational.h integer.h /usr/include/math.h
tableau.o: /usr/include/sys/stdsyms.h tableau.imp tableau.h ludecomp.h
tableau.o: gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h grarray.h
tableau.o: gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
tableau.o: /usr/include/sys/types.h /usr/include/sys/signal.h
tableau.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
tableau.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
tableau.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
tableau.o: /usr/include/assert.h garray.h gvector.h glist.h gwatch.h bfs.h
tableau.o: gmap.h
ludecomp.o: ludecomp.imp ludecomp.h gmatrix.h gambitio.h /usr/include/stdio.h
ludecomp.o: gmisc.h grarray.h gblock.h /usr/include/stdlib.h
ludecomp.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
ludecomp.o: /usr/include/sys/types.h /usr/include/sys/signal.h
ludecomp.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
ludecomp.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
ludecomp.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
ludecomp.o: /usr/include/assert.h garray.h gvector.h glist.h gwatch.h
ludecomp.o: glist.imp rational.h integer.h /usr/include/math.h
nfgpure.o: nfgpure.imp nfgpure.h nfg.h garray.h /usr/include/stdlib.h
nfgpure.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
nfgpure.o: /usr/include/sys/types.h /usr/include/sys/signal.h
nfgpure.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
nfgpure.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
nfgpure.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
nfgpure.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
nfgpure.o: gstring.h /usr/include/string.h gpvector.h gvector.h mixed.h
nfgpure.o: nfstrat.h gblock.h mixedsol.h glist.h nfgiter.h nfgciter.h
nfgpure.o: rational.h integer.h /usr/include/math.h
lhtab.o: lhtab.h lemketab.h tableau.h rational.h integer.h
lhtab.o: /usr/include/math.h /usr/include/sys/stdsyms.h ludecomp.h gmatrix.h
lhtab.o: gambitio.h /usr/include/stdio.h gmisc.h grarray.h gblock.h
lhtab.o: /usr/include/stdlib.h /usr/include/sys/wait.h
lhtab.o: /usr/include/sys/types.h /usr/include/sys/signal.h
lhtab.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
lhtab.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
lhtab.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
lhtab.o: /usr/include/assert.h garray.h gvector.h glist.h gwatch.h bfs.h
lhtab.o: gmap.h nfg.h gstring.h /usr/include/string.h gpvector.h nfgiter.h
lhtab.o: nfstrat.h
lemketab.o: lemketab.h tableau.h rational.h integer.h /usr/include/math.h
lemketab.o: /usr/include/sys/stdsyms.h ludecomp.h gmatrix.h gambitio.h
lemketab.o: /usr/include/stdio.h gmisc.h grarray.h gblock.h
lemketab.o: /usr/include/stdlib.h /usr/include/sys/wait.h
lemketab.o: /usr/include/sys/types.h /usr/include/sys/signal.h
lemketab.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
lemketab.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
lemketab.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
lemketab.o: /usr/include/assert.h garray.h gvector.h glist.h gwatch.h bfs.h
lemketab.o: gmap.h
grid.o: gmisc.h rational.h integer.h /usr/include/math.h
grid.o: /usr/include/sys/stdsyms.h gmatrix.h gambitio.h /usr/include/stdio.h
grid.o: grarray.h gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
grid.o: /usr/include/sys/types.h /usr/include/sys/signal.h
grid.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
grid.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
grid.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
grid.o: /usr/include/assert.h garray.h gvector.h nfg.h gstring.h
grid.o: /usr/include/string.h gpvector.h nfgiter.h nfstrat.h probvect.h
grid.o: gwatch.h grid.h gstatus.h gsignal.h gprogres.h grarray.imp garray.imp
nfgcsum.o: nfgcsum.imp gwatch.h gpvector.h gvector.h gmisc.h gambitio.h
nfgcsum.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
nfgcsum.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
nfgcsum.o: /usr/include/sys/types.h /usr/include/sys/signal.h
nfgcsum.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
nfgcsum.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
nfgcsum.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
nfgcsum.o: /usr/include/assert.h nfg.h gstring.h /usr/include/string.h
nfgcsum.o: nfgiter.h nfgcsum.h rational.h integer.h /usr/include/math.h
nfgcsum.o: glist.h gstatus.h gsignal.h gprogres.h tableau.h ludecomp.h
nfgcsum.o: gmatrix.h grarray.h gblock.h bfs.h gmap.h lpsolve.h mixed.h
nfgcsum.o: nfstrat.h mixedsol.h
lpsolve.o: lpsolve.imp lpsolve.h tableau.h rational.h integer.h
lpsolve.o: /usr/include/math.h /usr/include/sys/stdsyms.h ludecomp.h
lpsolve.o: gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h grarray.h
lpsolve.o: gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
lpsolve.o: /usr/include/sys/types.h /usr/include/sys/signal.h
lpsolve.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
lpsolve.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
lpsolve.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
lpsolve.o: /usr/include/assert.h garray.h gvector.h glist.h gwatch.h bfs.h
lpsolve.o: gmap.h
nfdom.o: nfg.h garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
nfdom.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
nfdom.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
nfdom.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
nfdom.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
nfdom.o: /usr/include/machine/frame.h /usr/include/assert.h gambitio.h
nfdom.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
nfdom.o: gpvector.h gvector.h nfstrat.h gblock.h nfgciter.h rational.h
nfdom.o: integer.h /usr/include/math.h
vertenum.o: vertenum.imp vertenum.h tableau.h rational.h integer.h
vertenum.o: /usr/include/math.h /usr/include/sys/stdsyms.h ludecomp.h
vertenum.o: gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h grarray.h
vertenum.o: gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
vertenum.o: /usr/include/sys/types.h /usr/include/sys/signal.h
vertenum.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
vertenum.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
vertenum.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
vertenum.o: /usr/include/assert.h garray.h gvector.h glist.h gwatch.h bfs.h
vertenum.o: gmap.h gstatus.h gsignal.h gprogres.h gstring.h
vertenum.o: /usr/include/string.h
mixedsol.o: mixedsol.h gmisc.h mixed.h nfstrat.h gstring.h
mixedsol.o: /usr/include/string.h gblock.h /usr/include/stdlib.h
mixedsol.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
mixedsol.o: /usr/include/sys/types.h /usr/include/sys/signal.h
mixedsol.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
mixedsol.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
mixedsol.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
mixedsol.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
mixedsol.o: gpvector.h gvector.h
extshow.o: ./wx_fake/wx.h ./wx_fake/wx_form.h wxmisc.h /usr/include/stdio.h
extshow.o: ./wx_fake/wx_timer.h efg.h gstring.h /usr/include/string.h
extshow.o: gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
extshow.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
extshow.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
extshow.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
extshow.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
extshow.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
extshow.o: gambitio.h gmisc.h glist.h gpvector.h gvector.h behav.h efstrat.h
extshow.o: gdpvect.h efplayer.h infoset.h rational.h integer.h
extshow.o: /usr/include/math.h node.h outcome.h efgconst.h gambit.h treewin.h
extshow.o: treedraw.h treecons.h gambdraw.h twflash.h extshow.h efgnfgi.h
extshow.o: accels.h efgsolng.h behavsol.h bsolnsf.h gslist.h extsoln.h
extshow.o: spread.h grblock.h grarray.h normgui.h elimdomd.h delsolnd.h
btreewn.o: bitmaps/copy.xpm bitmaps/move.xpm
treedraw.o: ./wx_fake/wx.h ./wx_fake/wx_form.h wxmisc.h /usr/include/stdio.h
treedraw.o: ./wx_fake/wx_timer.h treedraw.h gblock.h /usr/include/stdlib.h
treedraw.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
treedraw.o: /usr/include/sys/types.h /usr/include/sys/signal.h
treedraw.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
treedraw.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
treedraw.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
treedraw.o: /usr/include/assert.h garray.h gambitio.h gmisc.h treecons.h
treedraw.o: gambdraw.h gstring.h /usr/include/string.h legendc.h legend.h
twflash.o: ./wx_fake/wx.h twflash.h ./wx_fake/wx_timer.h
extsoln.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
extsoln.o: ./wx_fake/wx_form.h spread.h glist.h gambitio.h gmisc.h gblock.h
extsoln.o: /usr/include/stdlib.h /usr/include/sys/wait.h
extsoln.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
extsoln.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
extsoln.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
extsoln.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
extsoln.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
extsoln.o: grblock.h grarray.h gstring.h /usr/include/string.h rational.h
extsoln.o: integer.h /usr/include/math.h extsoln.h efg.h gpvector.h gvector.h
extsoln.o: behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h outcome.h
extsoln.o: efgconst.h extshow.h efgnfgi.h gambit.h accels.h efgsolng.h
extsoln.o: behavsol.h bsolnsf.h gslist.h treedraw.h treecons.h gambdraw.h
extsoln.o: treewin.h twflash.h legendc.h
btreewni.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
btreewni.o: ./wx_fake/wx_form.h node.h rational.h integer.h
btreewni.o: /usr/include/math.h /usr/include/sys/stdsyms.h gblock.h
btreewni.o: /usr/include/stdlib.h /usr/include/sys/wait.h
btreewni.o: /usr/include/sys/types.h /usr/include/sys/signal.h
btreewni.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
btreewni.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
btreewni.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
btreewni.o: /usr/include/assert.h garray.h gambitio.h gmisc.h efg.h gstring.h
btreewni.o: /usr/include/string.h glist.h gpvector.h gvector.h behav.h
btreewni.o: efstrat.h gdpvect.h efplayer.h infoset.h outcome.h treewin.h
btreewni.o: treedraw.h treecons.h gambdraw.h twflash.h efgconst.h glist.imp
bextshow.o: ./wx_fake/wx.h ./wx_fake/wx_form.h ./wx_fake/wx_tbar.h wxmisc.h
bextshow.o: /usr/include/stdio.h ./wx_fake/wx_timer.h efg.h gstring.h
bextshow.o: /usr/include/string.h gblock.h /usr/include/stdlib.h
bextshow.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
bextshow.o: /usr/include/sys/types.h /usr/include/sys/signal.h
bextshow.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
bextshow.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
bextshow.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
bextshow.o: /usr/include/assert.h garray.h gambitio.h gmisc.h glist.h
bextshow.o: gpvector.h gvector.h behav.h efstrat.h gdpvect.h efplayer.h
bextshow.o: infoset.h rational.h integer.h /usr/include/math.h node.h
bextshow.o: outcome.h efgconst.h gambit.h treewin.h treedraw.h treecons.h
bextshow.o: gambdraw.h twflash.h extshow.h efgnfgi.h accels.h efgsolng.h
bextshow.o: behavsol.h bsolnsf.h gslist.h extsoln.h spread.h grblock.h
bextshow.o: grarray.h efgaccl.h
outcomed.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
outcomed.o: ./wx_fake/wx_form.h spread.h glist.h gambitio.h gmisc.h gblock.h
outcomed.o: /usr/include/stdlib.h /usr/include/sys/wait.h
outcomed.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
outcomed.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
outcomed.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
outcomed.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
outcomed.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
outcomed.o: grblock.h grarray.h gstring.h /usr/include/string.h efg.h
outcomed.o: gpvector.h gvector.h behav.h efstrat.h gdpvect.h efplayer.h
outcomed.o: infoset.h rational.h integer.h /usr/include/math.h node.h
outcomed.o: outcome.h treewin.h treedraw.h treecons.h gambdraw.h twflash.h
outcomed.o: efgconst.h outcomed.h
efgsolng.o: ./wx_fake/wx.h ./wx_fake/wx_form.h wxmisc.h /usr/include/stdio.h
efgsolng.o: ./wx_fake/wx_timer.h wxstatus.h gstatus.h gsignal.h gmisc.h
efgsolng.o: gprogres.h gambitio.h gstring.h /usr/include/string.h subsolve.h
efgsolng.o: efg.h gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
efgsolng.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efgsolng.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efgsolng.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efgsolng.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efgsolng.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efgsolng.o: glist.h gpvector.h gvector.h behav.h efstrat.h gdpvect.h
efgsolng.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h
efgsolng.o: node.h outcome.h nfg.h behavsol.h gfunc.h efgsolng.h nfgconst.h
efgsolng.o: gslist.h elimdomd.h nliap.h mixed.h nfstrat.h mixedsol.h eliap.h
efgsolng.o: liapsub.h liapprm.h algdlgs.h seqform.h gmatrix.h grarray.h
efgsolng.o: lemketab.h tableau.h ludecomp.h gwatch.h bfs.h gmap.h seqfprm.h
efgsolng.o: lemkesub.h lemke.h lhtab.h lemkeprm.h psnesub.h nfgpure.h
efgsolng.o: purenprm.h efgpure.h enumsub.h enum.h vertenum.h enumprm.h
efgsolng.o: csumsub.h nfgcsum.h lpsolve.h efgcsum.h csumprm.h simpsub.h
efgsolng.o: simpdiv.h simpprm.h egobit.h ngobit.h gobitprm.h grid.h gridprm.h
bsolnsf.o: bsolnsf.h garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
bsolnsf.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
bsolnsf.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
bsolnsf.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
bsolnsf.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
bsolnsf.o: /usr/include/machine/frame.h /usr/include/assert.h gambitio.h
bsolnsf.o: /usr/include/stdio.h gmisc.h gslist.h glist.h behavsol.h behav.h
bsolnsf.o: gstring.h /usr/include/string.h efstrat.h gblock.h efg.h
bsolnsf.o: gpvector.h gvector.h efplayer.h infoset.h rational.h integer.h
bsolnsf.o: /usr/include/math.h node.h outcome.h gdpvect.h gsmincl.h
bsolnsf.o: gslist.imp
normshow.o: ./wx_fake/wx.h normshow.h wxmisc.h /usr/include/stdio.h
normshow.o: ./wx_fake/wx_timer.h ./wx_fake/wx_form.h wxio.h
normshow.o: /usr/include/assert.h /usr/include/sys/stdsyms.h gambitio.h
normshow.o: gmisc.h gambit.h spread.h glist.h gblock.h /usr/include/stdlib.h
normshow.o: /usr/include/sys/wait.h /usr/include/sys/types.h
normshow.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
normshow.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
normshow.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
normshow.o: /usr/include/machine/frame.h garray.h grblock.h grarray.h
normshow.o: gstring.h /usr/include/string.h normgui.h normdraw.h gambdraw.h
normshow.o: efgnfgi.h accels.h rational.h integer.h /usr/include/math.h nfg.h
normshow.o: gpvector.h gvector.h nfgiter.h mixedsol.h mixed.h nfstrat.h
normshow.o: nfgsolvd.h nfgsolng.h gslist.h msolnsf.h normsoln.h nfgconst.h
normshow.o: nfplayer.h
bnormshw.o: normshow.h ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h
bnormshw.o: ./wx_fake/wx_timer.h ./wx_fake/wx_form.h wxio.h
bnormshw.o: /usr/include/assert.h /usr/include/sys/stdsyms.h gambitio.h
bnormshw.o: gmisc.h gambit.h spread.h glist.h gblock.h /usr/include/stdlib.h
bnormshw.o: /usr/include/sys/wait.h /usr/include/sys/types.h
bnormshw.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
bnormshw.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
bnormshw.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
bnormshw.o: /usr/include/machine/frame.h garray.h grblock.h grarray.h
bnormshw.o: gstring.h /usr/include/string.h normgui.h normdraw.h gambdraw.h
bnormshw.o: efgnfgi.h accels.h rational.h integer.h /usr/include/math.h nfg.h
bnormshw.o: gpvector.h gvector.h nfgiter.h mixedsol.h mixed.h nfstrat.h
bnormshw.o: nfgsolvd.h nfgsolng.h gslist.h msolnsf.h nfplayer.h normaccl.h
bnormshw.o: nfgconst.h sprdaccl.h sprconst.h
normsoln.o: ./wx_fake/wx.h nfgconst.h normshow.h wxmisc.h
normsoln.o: /usr/include/stdio.h ./wx_fake/wx_timer.h ./wx_fake/wx_form.h
normsoln.o: wxio.h /usr/include/assert.h /usr/include/sys/stdsyms.h
normsoln.o: gambitio.h gmisc.h gambit.h spread.h glist.h gblock.h
normsoln.o: /usr/include/stdlib.h /usr/include/sys/wait.h
normsoln.o: /usr/include/sys/types.h /usr/include/sys/signal.h
normsoln.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
normsoln.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
normsoln.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h garray.h
normsoln.o: grblock.h grarray.h gstring.h /usr/include/string.h normgui.h
normsoln.o: normdraw.h gambdraw.h efgnfgi.h accels.h rational.h integer.h
normsoln.o: /usr/include/math.h nfg.h gpvector.h gvector.h nfgiter.h
normsoln.o: mixedsol.h mixed.h nfstrat.h nfgsolvd.h nfgsolng.h gslist.h
normsoln.o: msolnsf.h normsoln.h nfplayer.h
msolnsf.o: msolnsf.h garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
msolnsf.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
msolnsf.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
msolnsf.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
msolnsf.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
msolnsf.o: /usr/include/machine/frame.h /usr/include/assert.h gambitio.h
msolnsf.o: /usr/include/stdio.h gmisc.h gslist.h glist.h mixedsol.h mixed.h
msolnsf.o: nfstrat.h gstring.h /usr/include/string.h gblock.h gpvector.h
msolnsf.o: gvector.h gsmincl.h rational.h integer.h /usr/include/math.h
msolnsf.o: gslist.imp
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
wximpl.o: gambitio.h /usr/include/stdio.h gmisc.h glist.imp glist.h
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
wxio.o: /usr/include/sys/stdsyms.h gambitio.h gmisc.h
spread.o: /usr/include/stdio.h ./wx_fake/wx.h ./wx_fake/wx_mf.h
spread.o: ./wx_fake/wx_tbar.h general.h /usr/include/stdlib.h
spread.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
spread.o: /usr/include/sys/types.h /usr/include/sys/signal.h
spread.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
spread.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
spread.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
spread.o: /usr/include/string.h wxmisc.h ./wx_fake/wx_timer.h
spread.o: ./wx_fake/wx_form.h spread.h glist.h gambitio.h gmisc.h gblock.h
spread.o: /usr/include/assert.h garray.h grblock.h grarray.h gstring.h
spreadim.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
spreadim.o: ./wx_fake/wx_form.h grblock.h grarray.h gambitio.h gmisc.h
spreadim.o: garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
spreadim.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
spreadim.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
spreadim.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
spreadim.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
spreadim.o: /usr/include/machine/frame.h /usr/include/assert.h spread.h
spreadim.o: glist.h gblock.h gstring.h /usr/include/string.h rational.h
spreadim.o: integer.h /usr/include/math.h glist.imp grarray.imp grblock.imp
spreadim.o: garray.imp
gambit.o: /usr/include/assert.h /usr/include/sys/stdsyms.h
gambit.o: /usr/include/string.h /usr/include/ctype.h ./wx_fake/wx.h
gambit.o: ./wx_fake/wx_tbar.h wxio.h /usr/include/stdio.h gambitio.h gmisc.h
gambit.o: gambit.h wxmisc.h ./wx_fake/wx_timer.h ./wx_fake/wx_form.h
gambit.o: normgui.h gstring.h extgui.h /usr/include/signal.h
gambit.o: /usr/include/sys/signal.h /usr/include/sys/types.h
gambit.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gambit.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gambit.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gambit.o: /usr/include/math.h
accels.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
accels.o: ./wx_fake/wx_form.h keynames.h gmisc.h glist.imp glist.h gambitio.h
accels.o: /usr/include/assert.h /usr/include/sys/stdsyms.h accels.h garray.h
accels.o: /usr/include/stdlib.h /usr/include/sys/wait.h
accels.o: /usr/include/sys/types.h /usr/include/sys/signal.h
accels.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
accels.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
accels.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h gstring.h
accels.o: /usr/include/string.h garray.imp
wxstatus.o: ./wx_fake/wx.h wxstatus.h gstatus.h gsignal.h gmisc.h gprogres.h
wxstatus.o: gambitio.h /usr/include/stdio.h gstring.h /usr/include/string.h
efgnfgi.o: efgnfgi.h rational.h integer.h /usr/include/math.h
efgnfgi.o: /usr/include/sys/stdsyms.h
general.o: general.h /usr/include/stdio.h /usr/include/stdlib.h
general.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
general.o: /usr/include/sys/types.h /usr/include/sys/signal.h
general.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
general.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
general.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
general.o: /usr/include/string.h
