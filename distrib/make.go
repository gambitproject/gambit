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
CFLAGS = -DMINI_POLY -Wall  -fno-implicit-templates -I../

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
		grarray.cc gmatrix.cc gclsig.cc bfs.cc gwatch.cc \
		gfunc.cc gclstats.cc glist.cc subsolve.cc gfuncmin.cc \
		gpoly.cc gpolyctr.cc prepoly.cc monomial.cc gnumber.cc \
		gsmatrix.cc

GLIB_OBJECTS = gmisc.o gambitio.o garray.o gblock.o gstring.o integer.o \
		rational.o gnulib.o gvector.o gpvector.o gdpvect.o \
		grarray.o gmatrix.o bfs.o gwatch.o \
		gfunc.o gclstats.o glist.o subsolve.o gfuncmin.o \
		gpoly.o gpolyctr.o prepoly.o monomial.o gnumber.o \
		gsmatrix.o 

GAME_SOURCES = readnfg.cc readefg.cc nfg.cc nfgdbl.cc nfgrat.cc nfgutils.cc \
		efg.cc efgdbl.cc efgrat.cc nfstrat.cc efgnfgr.cc efgutils.cc \
		efstrat.cc efgnfgd.cc efdom.cc efgnfg.cc

GAME_OBJECTS = readnfg.o readefg.o nfg.o nfgdbl.o nfgrat.o nfgutils.o \
		efg.o efgdbl.o efgrat.o nfstrat.o efgnfgr.o efgutils.o \
		efstrat.o efgnfgd.o efdom.o efgnfg.o

EALG_SOURCES =  egobit.cc eliap.cc seqform.cc efgcsum.cc \
		psnesub.cc efgpure.cc lemkesub.cc liapsub.cc simpsub.cc \
		enumsub.cc csumsub.cc behavsol.cc 

EALG_OBJECTS =  egobit.o eliap.o seqform.o efgcsum.o \
		psnesub.o efgpure.o lemkesub.o liapsub.o simpsub.o \
		enumsub.o csumsub.o behavsol.o 

NALG_SOURCES = lemke.cc nliap.cc ngobit.cc enum.cc simpdiv.cc tableau.cc \
		ludecomp.cc nfgpure.cc lhtab.cc lemketab.cc grid.cc nfgcsum.cc\
		lpsolve.cc nfdom.cc vertenum.cc mixedsol.cc nfdommix.cc 


NALG_OBJECTS = lemke.o nliap.o ngobit.o enum.o simpdiv.o tableau.o ludecomp.o \
		nfgpure.o lhtab.o lemketab.o grid.o nfgcsum.o lpsolve.o nfdom.o\
		vertenum.o mixedsol.o nfdommix.o 

AGCL_SOURCES = gsmutils.cc gsm.cc gsmfunc.cc gsmoper.cc gsmhash.cc \
               gsminstr.cc portion.cc nfgfunc.cc efgfunc.cc listfunc.cc \
               algfunc.cc gcompile.cc gcl.cc gclsig.cc solfunc.cc gsmincl.cc\
	       system.cc gcmdline.cc

AGCL_OBJECTS = gsmutils.o gsm.o gsmfunc.o gsmoper.o gsmhash.o \
               gsminstr.o portion.o nfgfunc.o efgfunc.o listfunc.o \
               algfunc.o gcompile.o gcl.o gclsig.o solfunc.o gsmincl.o \
	       system.o gcmdline.o

# Gui stuff for the extensive form
EGUI_SOURCES = efgshow.cc efgshow1.cc treewin.cc treewin1.cc treedraw.cc \
               twflash.cc efgsoln.cc efgoutcd.cc efgsolng.cc bsolnsf.cc
		
EGUI_OBJECTS = efgshow.o efgshow1.o treewin.o treewin1.o treedraw.o \
               twflash.o efgsoln.o efgoutcd.o efgsolng.o bsolnsf.o 

# Gui stuff for the normal form
NGUI_SOURCES = nfgshow.cc nfgshow1.cc nfgsoln.cc msolnsf.cc nfgoutcd.cc
NGUI_OBJECTS = nfgshow.o nfgshow1.o nfgsoln.o msolnsf.o nfgoutcd.o

# Gui stuff used in both the normal and extensive forms
BGUI_SOURCES = wxmisc.cc gambdraw.cc wxio.cc spread.cc spreadim.cc \
               gambit.cc accels.cc algdlgs.cc wxstatus.cc paramsd.cc 
BGUI_OBJECTS = wxmisc.o gambdraw.o wxio.o spread.o spreadim.o \
               gambit.o accels.o algdlgs.o wxstatus.o paramsd.o

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
	makedepend -f make.go -DMINI_POLY -I./wx_fake $(GCL_SOURCES) 
depend_gui:
	makedepend -f make.go -DMINI_POLY -I./wx_fake $(GUI_SOURCES) 
clean:
	rm -f *.o core *~ *.bak gambgui gcl *.zip

$(OBJDIR):
	mkdir $(OBJDIR)
# DO NOT DELETE THIS LINE -- make depend depends on it.

gmisc.o: gmisc.h rational.h integer.h /usr/include/math.h
gmisc.o: /usr/include/sys/stdsyms.h gnumber.h /usr/include/stdio.h
gmisc.o: /usr/include/stdlib.h /usr/include/sys/wait.h
gmisc.o: /usr/include/sys/types.h /usr/include/sys/signal.h
gmisc.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gmisc.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gmisc.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gmisc.o: /usr/include/limits.h /usr/include/sys/param.h
gmisc.o: /usr/include/machine/param.h /usr/include/sys/sysmacros.h
gmisc.o: /usr/include/machine/param_shm.h /usr/include/sys/time.h
gmisc.o: /usr/include/machine/spl.h /usr/include/ctype.h gambitio.h gstring.h
gmisc.o: /usr/include/string.h gpoly.h monomial.h prepoly.h garray.h
gmisc.o: /usr/include/assert.h glist.h gblock.h gvector.h
gambitio.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
garray.o: garray.imp /usr/include/stdlib.h /usr/include/sys/wait.h
garray.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
garray.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
garray.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
garray.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
garray.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
garray.o: gambitio.h /usr/include/stdio.h gmisc.h gstring.h
garray.o: /usr/include/string.h rational.h integer.h /usr/include/math.h
garray.o: gnumber.h
gblock.o: gmisc.h gblock.imp /usr/include/stdlib.h /usr/include/sys/wait.h
gblock.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
gblock.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
gblock.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
gblock.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
gblock.o: /usr/include/machine/frame.h /usr/include/assert.h gblock.h
gblock.o: garray.h gambitio.h /usr/include/stdio.h gstring.h
gblock.o: /usr/include/string.h rational.h integer.h /usr/include/math.h
gblock.o: gnumber.h garray.imp
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
gnulib.o: /usr/include/assert.h /usr/include/values.h gnulib.h
gnulib.o: /usr/include/stddef.h /usr/include/sys/stdsyms.h
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
gvector.o: /usr/include/machine/frame.h /usr/include/assert.h gnumber.h
gvector.o: rational.h integer.h /usr/include/math.h
gpvector.o: gpvector.imp gpvector.h gvector.h gmisc.h gambitio.h
gpvector.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
gpvector.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
gpvector.o: /usr/include/sys/types.h /usr/include/sys/signal.h
gpvector.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gpvector.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gpvector.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gpvector.o: /usr/include/assert.h rational.h integer.h /usr/include/math.h
gpvector.o: gnumber.h
gdpvect.o: gdpvect.imp gdpvect.h gpvector.h gvector.h gmisc.h gambitio.h
gdpvect.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
gdpvect.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
gdpvect.o: /usr/include/sys/types.h /usr/include/sys/signal.h
gdpvect.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gdpvect.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gdpvect.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gdpvect.o: /usr/include/assert.h rational.h integer.h /usr/include/math.h
gdpvect.o: gnumber.h
grarray.o: grarray.imp grarray.h gambitio.h /usr/include/stdio.h gmisc.h
grarray.o: /usr/include/assert.h garray.h /usr/include/stdlib.h
grarray.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
grarray.o: /usr/include/sys/types.h /usr/include/sys/signal.h
grarray.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
grarray.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
grarray.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h gnumber.h
grarray.o: rational.h integer.h /usr/include/math.h gstring.h
grarray.o: /usr/include/string.h
gmatrix.o: gmatrix.imp gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h
gmatrix.o: grarray.h gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
gmatrix.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
gmatrix.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
gmatrix.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
gmatrix.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
gmatrix.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
gmatrix.o: gvector.h gsmatrix.h rational.h integer.h /usr/include/math.h
gclsig.o: gsignal.h gmisc.h /usr/include/signal.h /usr/include/sys/signal.h
gclsig.o: /usr/include/sys/types.h /usr/include/sys/stdsyms.h
gclsig.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gclsig.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gclsig.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
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
glist.o: /usr/include/assert.h gstring.h /usr/include/string.h rational.h
glist.o: integer.h /usr/include/math.h /usr/include/sys/stdsyms.h gnumber.h
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
subsolve.o: gvector.h grblock.h grarray.h gnumber.h rational.h integer.h
subsolve.o: /usr/include/math.h gpoly.h monomial.h prepoly.h gpolyctr.h
subsolve.o: behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h outcome.h
subsolve.o: efgutils.h nfg.h nfstrat.h gwatch.h subsolve.imp subsolve.h
subsolve.o: behavsol.h garray.imp glist.imp
gfuncmin.o: /usr/include/math.h /usr/include/sys/stdsyms.h gfunc.h gmisc.h
gfuncmin.o: gstatus.h gsignal.h gprogres.h gambitio.h /usr/include/stdio.h
gfuncmin.o: gstring.h /usr/include/string.h gvector.h garray.h
gfuncmin.o: /usr/include/stdlib.h /usr/include/sys/wait.h
gfuncmin.o: /usr/include/sys/types.h /usr/include/sys/signal.h
gfuncmin.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gfuncmin.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gfuncmin.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gfuncmin.o: /usr/include/assert.h gpvector.h gmatrix.h grarray.h gblock.h
gfuncmin.o: gsmatrix.h
gpoly.o: gpoly.imp gpoly.h monomial.h prepoly.h /usr/include/math.h
gpoly.o: /usr/include/sys/stdsyms.h gambitio.h /usr/include/stdio.h gmisc.h
gpoly.o: gstring.h /usr/include/string.h garray.h /usr/include/stdlib.h
gpoly.o: /usr/include/sys/wait.h /usr/include/sys/types.h
gpoly.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
gpoly.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
gpoly.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
gpoly.o: /usr/include/machine/frame.h /usr/include/assert.h glist.h gblock.h
gpoly.o: rational.h integer.h gvector.h gnumber.h glist.imp garray.imp
gpoly.o: gblock.imp
gpolyctr.o: gpolyctr.h gpoly.h monomial.h prepoly.h /usr/include/math.h
gpolyctr.o: /usr/include/sys/stdsyms.h gambitio.h /usr/include/stdio.h
gpolyctr.o: gmisc.h gstring.h /usr/include/string.h garray.h
gpolyctr.o: /usr/include/stdlib.h /usr/include/sys/wait.h
gpolyctr.o: /usr/include/sys/types.h /usr/include/sys/signal.h
gpolyctr.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gpolyctr.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gpolyctr.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gpolyctr.o: /usr/include/assert.h glist.h gblock.h rational.h integer.h
gpolyctr.o: gvector.h gnumber.h
prepoly.o: garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
prepoly.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
prepoly.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
prepoly.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
prepoly.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
prepoly.o: /usr/include/machine/frame.h /usr/include/assert.h gambitio.h
prepoly.o: /usr/include/stdio.h gmisc.h prepoly.h /usr/include/math.h
prepoly.o: gstring.h /usr/include/string.h glist.h gblock.h rational.h
prepoly.o: integer.h gvector.h glist.imp
monomial.o: monomial.imp monomial.h prepoly.h /usr/include/math.h
monomial.o: /usr/include/sys/stdsyms.h gambitio.h /usr/include/stdio.h
monomial.o: gmisc.h gstring.h /usr/include/string.h garray.h
monomial.o: /usr/include/stdlib.h /usr/include/sys/wait.h
monomial.o: /usr/include/sys/types.h /usr/include/sys/signal.h
monomial.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
monomial.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
monomial.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
monomial.o: /usr/include/assert.h glist.h gblock.h rational.h integer.h
monomial.o: gvector.h glist.imp double.h garray.imp gblock.imp gnumber.h
gnumber.o: gambitio.h /usr/include/stdio.h gmisc.h gnumber.h rational.h
gnumber.o: integer.h /usr/include/math.h /usr/include/sys/stdsyms.h gnulib.h
gnumber.o: /usr/include/stddef.h /usr/include/stdlib.h
gnumber.o: /usr/include/sys/wait.h /usr/include/sys/types.h
gnumber.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
gnumber.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
gnumber.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
gnumber.o: /usr/include/machine/frame.h /usr/include/string.h
gnumber.o: /usr/include/memory.h /usr/include/errno.h
gnumber.o: /usr/include/sys/errno.h /usr/include/fcntl.h
gnumber.o: /usr/include/sys/fcntl.h /usr/include/values.h ./wx_fake/float.h
gnumber.o: /usr/include/assert.h /usr/include/ctype.h
gsmatrix.o: gsmatrix.imp gsmatrix.h gmatrix.h gambitio.h /usr/include/stdio.h
gsmatrix.o: gmisc.h grarray.h gblock.h /usr/include/stdlib.h
gsmatrix.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
gsmatrix.o: /usr/include/sys/types.h /usr/include/sys/signal.h
gsmatrix.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gsmatrix.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gsmatrix.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gsmatrix.o: /usr/include/assert.h garray.h gvector.h rational.h integer.h
gsmatrix.o: /usr/include/math.h
readnfg.o: /usr/include/stdio.h /usr/include/ctype.h gmisc.h gambitio.h
readnfg.o: glist.h rational.h integer.h /usr/include/math.h
readnfg.o: /usr/include/sys/stdsyms.h nfg.h garray.h /usr/include/stdlib.h
readnfg.o: /usr/include/sys/wait.h /usr/include/sys/types.h
readnfg.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
readnfg.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
readnfg.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
readnfg.o: /usr/include/machine/frame.h /usr/include/assert.h gstring.h
readnfg.o: /usr/include/string.h grblock.h grarray.h gnumber.h gpoly.h
readnfg.o: monomial.h prepoly.h gblock.h gvector.h gpolyctr.h nfplayer.h
readnfg.o: nfstrat.h
readefg.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/sys/wait.h
readefg.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
readefg.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
readefg.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
readefg.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
readefg.o: /usr/include/machine/frame.h /usr/include/ctype.h gambitio.h
readefg.o: gmisc.h gstring.h /usr/include/string.h rational.h integer.h
readefg.o: /usr/include/math.h gnumber.h gstack.h glist.h efg.h gblock.h
readefg.o: /usr/include/assert.h garray.h gpvector.h gvector.h grblock.h
readefg.o: grarray.h gpoly.h monomial.h prepoly.h gpolyctr.h behav.h
readefg.o: efstrat.h gdpvect.h efplayer.h infoset.h node.h outcome.h
readefg.o: gstack.imp
nfg.o: rational.h integer.h /usr/include/math.h /usr/include/sys/stdsyms.h
nfg.o: /usr/include/assert.h nfg.h garray.h /usr/include/stdlib.h
nfg.o: /usr/include/sys/wait.h /usr/include/sys/types.h
nfg.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
nfg.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
nfg.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
nfg.o: /usr/include/machine/frame.h gambitio.h /usr/include/stdio.h gmisc.h
nfg.o: gstring.h /usr/include/string.h grblock.h grarray.h gnumber.h gpoly.h
nfg.o: monomial.h prepoly.h glist.h gblock.h gvector.h gpolyctr.h nfstrat.h
nfg.o: nfplayer.h efg.h gpvector.h behav.h efstrat.h gdpvect.h efplayer.h
nfg.o: infoset.h node.h outcome.h lexicon.h nfgiter.h nfgiter.imp nfgciter.h
nfg.o: nfgciter.imp garray.imp gblock.imp glist.imp nfg.imp mixed.h
nfg.o: mixedsol.imp mixedsol.h
nfgdbl.o: nfg.imp nfg.h garray.h /usr/include/stdlib.h
nfgdbl.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
nfgdbl.o: /usr/include/sys/types.h /usr/include/sys/signal.h
nfgdbl.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
nfgdbl.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
nfgdbl.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
nfgdbl.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
nfgdbl.o: gstring.h /usr/include/string.h grblock.h grarray.h gnumber.h
nfgdbl.o: rational.h integer.h /usr/include/math.h gpoly.h monomial.h
nfgdbl.o: prepoly.h glist.h gblock.h gvector.h gpolyctr.h nfplayer.h
nfgdbl.o: nfstrat.h gpvector.h mixed.h glist.imp grblock.imp
nfgrat.o: rational.h integer.h /usr/include/math.h /usr/include/sys/stdsyms.h
nfgrat.o: nfg.imp nfg.h garray.h /usr/include/stdlib.h
nfgrat.o: /usr/include/sys/wait.h /usr/include/sys/types.h
nfgrat.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
nfgrat.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
nfgrat.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
nfgrat.o: /usr/include/machine/frame.h /usr/include/assert.h gambitio.h
nfgrat.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
nfgrat.o: grblock.h grarray.h gnumber.h gpoly.h monomial.h prepoly.h glist.h
nfgrat.o: gblock.h gvector.h gpolyctr.h nfplayer.h nfstrat.h gpvector.h
nfgrat.o: mixed.h glist.imp grblock.imp
nfgutils.o: gmisc.h nfg.h garray.h /usr/include/stdlib.h
nfgutils.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
nfgutils.o: /usr/include/sys/types.h /usr/include/sys/signal.h
nfgutils.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
nfgutils.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
nfgutils.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
nfgutils.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gstring.h
nfgutils.o: /usr/include/string.h grblock.h grarray.h gnumber.h rational.h
nfgutils.o: integer.h /usr/include/math.h gpoly.h monomial.h prepoly.h
nfgutils.o: glist.h gblock.h gvector.h gpolyctr.h nfplayer.h nfstrat.h
nfgutils.o: nfgciter.h
efg.o: garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
efg.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efg.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efg.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efg.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efg.o: /usr/include/machine/frame.h /usr/include/assert.h gambitio.h
efg.o: /usr/include/stdio.h gmisc.h rational.h integer.h /usr/include/math.h
efg.o: garray.imp gblock.imp gblock.h glist.imp glist.h efg.h gstring.h
efg.o: /usr/include/string.h gpvector.h gvector.h grblock.h grarray.h
efg.o: gnumber.h gpoly.h monomial.h prepoly.h gpolyctr.h behav.h efstrat.h
efg.o: gdpvect.h efplayer.h infoset.h node.h outcome.h efgutils.h lexicon.h
efg.o: efg.imp behavsol.imp behavsol.h
efgdbl.o: rational.h integer.h /usr/include/math.h /usr/include/sys/stdsyms.h
efgdbl.o: efg.imp efg.h gstring.h /usr/include/string.h gblock.h
efgdbl.o: /usr/include/stdlib.h /usr/include/sys/wait.h
efgdbl.o: /usr/include/sys/types.h /usr/include/sys/signal.h
efgdbl.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
efgdbl.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
efgdbl.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
efgdbl.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
efgdbl.o: gmisc.h glist.h gpvector.h gvector.h grblock.h grarray.h gnumber.h
efgdbl.o: gpoly.h monomial.h prepoly.h gpolyctr.h behav.h efstrat.h gdpvect.h
efgdbl.o: efplayer.h infoset.h node.h outcome.h efgutils.h efgiter.imp
efgdbl.o: efgciter.h efgiter.h efgciter.imp garray.imp glist.imp
efgrat.o: rational.h integer.h /usr/include/math.h /usr/include/sys/stdsyms.h
efgrat.o: efg.imp efg.h gstring.h /usr/include/string.h gblock.h
efgrat.o: /usr/include/stdlib.h /usr/include/sys/wait.h
efgrat.o: /usr/include/sys/types.h /usr/include/sys/signal.h
efgrat.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
efgrat.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
efgrat.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
efgrat.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
efgrat.o: gmisc.h glist.h gpvector.h gvector.h grblock.h grarray.h gnumber.h
efgrat.o: gpoly.h monomial.h prepoly.h gpolyctr.h behav.h efstrat.h gdpvect.h
efgrat.o: efplayer.h infoset.h node.h outcome.h efgutils.h efgiter.imp
efgrat.o: efgciter.h efgiter.h efgciter.imp garray.imp glist.imp
nfstrat.o: nfstrat.h gstring.h /usr/include/string.h gblock.h
nfstrat.o: /usr/include/stdlib.h /usr/include/sys/wait.h
nfstrat.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
nfstrat.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
nfstrat.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
nfstrat.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
nfstrat.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
nfstrat.o: gambitio.h /usr/include/stdio.h gmisc.h nfplayer.h nfg.h grblock.h
nfstrat.o: grarray.h gnumber.h rational.h integer.h /usr/include/math.h
nfstrat.o: gpoly.h monomial.h prepoly.h glist.h gvector.h gpolyctr.h
efgnfgr.o: efgnfg.imp efg.h gstring.h /usr/include/string.h gblock.h
efgnfgr.o: /usr/include/stdlib.h /usr/include/sys/wait.h
efgnfgr.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efgnfgr.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efgnfgr.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efgnfgr.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efgnfgr.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efgnfgr.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h
efgnfgr.o: gvector.h grblock.h grarray.h gnumber.h rational.h integer.h
efgnfgr.o: /usr/include/math.h gpoly.h monomial.h prepoly.h gpolyctr.h
efgnfgr.o: behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h outcome.h
efgnfgr.o: nfg.h nfplayer.h nfstrat.h nfgiter.h nfgciter.h mixed.h lexicon.h
efgnfgr.o: efg.imp efgutils.h
efgutils.o: efgutils.h efg.h gstring.h /usr/include/string.h gblock.h
efgutils.o: /usr/include/stdlib.h /usr/include/sys/wait.h
efgutils.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efgutils.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efgutils.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efgutils.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efgutils.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efgutils.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h
efgutils.o: gvector.h grblock.h grarray.h gnumber.h rational.h integer.h
efgutils.o: /usr/include/math.h gpoly.h monomial.h prepoly.h gpolyctr.h
efgutils.o: behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h outcome.h
efstrat.o: efg.h gstring.h /usr/include/string.h gblock.h
efstrat.o: /usr/include/stdlib.h /usr/include/sys/wait.h
efstrat.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efstrat.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efstrat.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efstrat.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efstrat.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efstrat.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h
efstrat.o: gvector.h grblock.h grarray.h gnumber.h rational.h integer.h
efstrat.o: /usr/include/math.h gpoly.h monomial.h prepoly.h gpolyctr.h
efstrat.o: behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h outcome.h
efgnfgd.o: efgnfg.imp efg.h gstring.h /usr/include/string.h gblock.h
efgnfgd.o: /usr/include/stdlib.h /usr/include/sys/wait.h
efgnfgd.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efgnfgd.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efgnfgd.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efgnfgd.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efgnfgd.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efgnfgd.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h
efgnfgd.o: gvector.h grblock.h grarray.h gnumber.h rational.h integer.h
efgnfgd.o: /usr/include/math.h gpoly.h monomial.h prepoly.h gpolyctr.h
efgnfgd.o: behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h outcome.h
efgnfgd.o: nfg.h nfplayer.h nfstrat.h nfgiter.h nfgciter.h mixed.h lexicon.h
efgnfgd.o: efg.imp efgutils.h
efdom.o: efg.h gstring.h /usr/include/string.h gblock.h /usr/include/stdlib.h
efdom.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
efdom.o: /usr/include/sys/types.h /usr/include/sys/signal.h
efdom.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
efdom.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
efdom.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
efdom.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
efdom.o: gmisc.h glist.h gpvector.h gvector.h grblock.h grarray.h gnumber.h
efdom.o: rational.h integer.h /usr/include/math.h gpoly.h monomial.h
efdom.o: prepoly.h gpolyctr.h behav.h efstrat.h gdpvect.h efplayer.h
efdom.o: infoset.h node.h outcome.h efgciter.h gstatus.h gsignal.h gprogres.h
efgnfg.o: efg.h gstring.h /usr/include/string.h gblock.h
efgnfg.o: /usr/include/stdlib.h /usr/include/sys/wait.h
efgnfg.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efgnfg.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efgnfg.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efgnfg.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efgnfg.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efgnfg.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h
efgnfg.o: gvector.h grblock.h grarray.h gnumber.h rational.h integer.h
efgnfg.o: /usr/include/math.h gpoly.h monomial.h prepoly.h gpolyctr.h behav.h
efgnfg.o: efstrat.h gdpvect.h efplayer.h infoset.h node.h outcome.h nfg.h
efgnfg.o: nfplayer.h nfstrat.h mixed.h nfgiter.h nfgciter.h lexicon.h
efgnfg.o: glist.imp garray.imp
egobit.o: /usr/include/math.h /usr/include/sys/stdsyms.h egobit.h gambitio.h
egobit.o: /usr/include/stdio.h gmisc.h gstatus.h gsignal.h gprogres.h
egobit.o: gstring.h /usr/include/string.h glist.h efg.h gblock.h
egobit.o: /usr/include/stdlib.h /usr/include/sys/wait.h
egobit.o: /usr/include/sys/types.h /usr/include/sys/signal.h
egobit.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
egobit.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
egobit.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
egobit.o: /usr/include/assert.h garray.h gpvector.h gvector.h grblock.h
egobit.o: grarray.h gnumber.h rational.h integer.h gpoly.h monomial.h
egobit.o: prepoly.h gpolyctr.h behav.h efstrat.h gdpvect.h efplayer.h
egobit.o: infoset.h node.h outcome.h behavsol.h gfunc.h gmatrix.h gsmatrix.h
eliap.o: eliap.h gambitio.h /usr/include/stdio.h gmisc.h gstatus.h gsignal.h
eliap.o: gprogres.h gstring.h /usr/include/string.h glist.h efg.h gblock.h
eliap.o: /usr/include/stdlib.h /usr/include/sys/wait.h
eliap.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
eliap.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
eliap.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
eliap.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
eliap.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
eliap.o: gpvector.h gvector.h grblock.h grarray.h gnumber.h rational.h
eliap.o: integer.h /usr/include/math.h gpoly.h monomial.h prepoly.h
eliap.o: gpolyctr.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h
eliap.o: outcome.h behavsol.h subsolve.h nfg.h gfunc.h gmatrix.h gsmatrix.h
seqform.o: seqform.imp seqform.h glist.h gambitio.h /usr/include/stdio.h
seqform.o: gmisc.h gstatus.h gsignal.h gprogres.h gstring.h
seqform.o: /usr/include/string.h behavsol.h behav.h efstrat.h gblock.h
seqform.o: /usr/include/stdlib.h /usr/include/sys/wait.h
seqform.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
seqform.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
seqform.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
seqform.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
seqform.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h efg.h
seqform.o: gpvector.h gvector.h grblock.h grarray.h gnumber.h rational.h
seqform.o: integer.h /usr/include/math.h gpoly.h monomial.h prepoly.h
seqform.o: gpolyctr.h efplayer.h infoset.h node.h outcome.h gdpvect.h
seqform.o: gwatch.h lhtab.h lemketab.h tableau.h ludecomp.h gmatrix.h
seqform.o: gsmatrix.h bfs.h gmap.h
efgcsum.o: efgcsum.imp gwatch.h gpvector.h gvector.h gmisc.h gambitio.h
efgcsum.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
efgcsum.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
efgcsum.o: /usr/include/sys/types.h /usr/include/sys/signal.h
efgcsum.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
efgcsum.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
efgcsum.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
efgcsum.o: /usr/include/assert.h efgutils.h efg.h gstring.h
efgcsum.o: /usr/include/string.h gblock.h glist.h grblock.h grarray.h
efgcsum.o: gnumber.h rational.h integer.h /usr/include/math.h gpoly.h
efgcsum.o: monomial.h prepoly.h gpolyctr.h behav.h efstrat.h gdpvect.h
efgcsum.o: efplayer.h infoset.h node.h outcome.h efgcsum.h gstatus.h
efgcsum.o: gsignal.h gprogres.h tableau.h ludecomp.h gmatrix.h gsmatrix.h
efgcsum.o: bfs.h gmap.h lpsolve.h vertenum.h behavsol.h subsolve.h nfg.h
psnesub.o: rational.h integer.h /usr/include/math.h
psnesub.o: /usr/include/sys/stdsyms.h psnesub.imp psnesub.h nfgpure.h nfg.h
psnesub.o: garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
psnesub.o: /usr/include/sys/types.h /usr/include/sys/signal.h
psnesub.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
psnesub.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
psnesub.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
psnesub.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
psnesub.o: gstring.h /usr/include/string.h grblock.h grarray.h gnumber.h
psnesub.o: gpoly.h monomial.h prepoly.h glist.h gblock.h gvector.h gpolyctr.h
psnesub.o: mixed.h nfstrat.h gpvector.h mixedsol.h behavsol.h behav.h
psnesub.o: efstrat.h efg.h efplayer.h infoset.h node.h outcome.h gdpvect.h
psnesub.o: subsolve.h
efgpure.o: efgpure.imp glist.h gambitio.h /usr/include/stdio.h gmisc.h efg.h
efgpure.o: gstring.h /usr/include/string.h gblock.h /usr/include/stdlib.h
efgpure.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
efgpure.o: /usr/include/sys/types.h /usr/include/sys/signal.h
efgpure.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
efgpure.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
efgpure.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
efgpure.o: /usr/include/assert.h garray.h gpvector.h gvector.h grblock.h
efgpure.o: grarray.h gnumber.h rational.h integer.h /usr/include/math.h
efgpure.o: gpoly.h monomial.h prepoly.h gpolyctr.h behav.h efstrat.h
efgpure.o: gdpvect.h efplayer.h infoset.h node.h outcome.h efgiter.h
efgpure.o: efgciter.h behavsol.h efgpure.h subsolve.h nfg.h
lemkesub.o: rational.h integer.h /usr/include/math.h
lemkesub.o: /usr/include/sys/stdsyms.h lemkesub.imp lemkesub.h lemke.h nfg.h
lemkesub.o: garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
lemkesub.o: /usr/include/sys/types.h /usr/include/sys/signal.h
lemkesub.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
lemkesub.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
lemkesub.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
lemkesub.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
lemkesub.o: gstring.h /usr/include/string.h grblock.h grarray.h gnumber.h
lemkesub.o: gpoly.h monomial.h prepoly.h glist.h gblock.h gvector.h
lemkesub.o: gpolyctr.h mixedsol.h mixed.h nfstrat.h gpvector.h gstatus.h
lemkesub.o: gsignal.h gprogres.h behavsol.h behav.h efstrat.h efg.h
lemkesub.o: efplayer.h infoset.h node.h outcome.h gdpvect.h lemke.imp
lemkesub.o: gwatch.h lhtab.h lemketab.h tableau.h ludecomp.h gmatrix.h
lemkesub.o: gsmatrix.h bfs.h gmap.h subsolve.h
liapsub.o: liapsub.h nliap.h gambitio.h /usr/include/stdio.h gmisc.h
liapsub.o: gstatus.h gsignal.h gprogres.h gstring.h /usr/include/string.h
liapsub.o: glist.h nfg.h garray.h /usr/include/stdlib.h
liapsub.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
liapsub.o: /usr/include/sys/types.h /usr/include/sys/signal.h
liapsub.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
liapsub.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
liapsub.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
liapsub.o: /usr/include/assert.h grblock.h grarray.h gnumber.h rational.h
liapsub.o: integer.h /usr/include/math.h gpoly.h monomial.h prepoly.h
liapsub.o: gblock.h gvector.h gpolyctr.h mixed.h nfstrat.h gpvector.h
liapsub.o: mixedsol.h subsolve.h efg.h behav.h efstrat.h gdpvect.h efplayer.h
liapsub.o: infoset.h node.h outcome.h behavsol.h
simpsub.o: rational.h integer.h /usr/include/math.h
simpsub.o: /usr/include/sys/stdsyms.h simpsub.imp simpsub.h simpdiv.h nfg.h
simpsub.o: garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
simpsub.o: /usr/include/sys/types.h /usr/include/sys/signal.h
simpsub.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
simpsub.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
simpsub.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
simpsub.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
simpsub.o: gstring.h /usr/include/string.h grblock.h grarray.h gnumber.h
simpsub.o: gpoly.h monomial.h prepoly.h glist.h gblock.h gvector.h gpolyctr.h
simpsub.o: gstatus.h gsignal.h gprogres.h mixedsol.h mixed.h nfstrat.h
simpsub.o: gpvector.h subsolve.h efg.h behav.h efstrat.h gdpvect.h efplayer.h
simpsub.o: infoset.h node.h outcome.h behavsol.h
enumsub.o: rational.h integer.h /usr/include/math.h
enumsub.o: /usr/include/sys/stdsyms.h enumsub.imp enumsub.h enum.h nfg.h
enumsub.o: garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
enumsub.o: /usr/include/sys/types.h /usr/include/sys/signal.h
enumsub.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
enumsub.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
enumsub.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
enumsub.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
enumsub.o: gstring.h /usr/include/string.h grblock.h grarray.h gnumber.h
enumsub.o: gpoly.h monomial.h prepoly.h glist.h gblock.h gvector.h gpolyctr.h
enumsub.o: gstatus.h gsignal.h gprogres.h mixed.h nfstrat.h gpvector.h
enumsub.o: mixedsol.h vertenum.h tableau.h ludecomp.h gmatrix.h gsmatrix.h
enumsub.o: bfs.h gmap.h behavsol.h behav.h efstrat.h efg.h efplayer.h
enumsub.o: infoset.h node.h outcome.h gdpvect.h subsolve.h
csumsub.o: rational.h integer.h /usr/include/math.h
csumsub.o: /usr/include/sys/stdsyms.h csumsub.imp csumsub.h subsolve.h efg.h
csumsub.o: gstring.h /usr/include/string.h gblock.h /usr/include/stdlib.h
csumsub.o: /usr/include/sys/wait.h /usr/include/sys/types.h
csumsub.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
csumsub.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
csumsub.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
csumsub.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
csumsub.o: gambitio.h /usr/include/stdio.h gmisc.h glist.h gpvector.h
csumsub.o: gvector.h grblock.h grarray.h gnumber.h gpoly.h monomial.h
csumsub.o: prepoly.h gpolyctr.h behav.h efstrat.h gdpvect.h efplayer.h
csumsub.o: infoset.h node.h outcome.h nfg.h behavsol.h nfgcsum.h gstatus.h
csumsub.o: gsignal.h gprogres.h mixedsol.h mixed.h nfstrat.h
behavsol.o: behavsol.h gmisc.h behav.h gstring.h /usr/include/string.h
behavsol.o: efstrat.h gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
behavsol.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
behavsol.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
behavsol.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
behavsol.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
behavsol.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
behavsol.o: gambitio.h /usr/include/stdio.h efg.h glist.h gpvector.h
behavsol.o: gvector.h grblock.h grarray.h gnumber.h rational.h integer.h
behavsol.o: /usr/include/math.h gpoly.h monomial.h prepoly.h gpolyctr.h
behavsol.o: efplayer.h infoset.h node.h outcome.h gdpvect.h
lemke.o: lemke.imp gwatch.h gpvector.h gvector.h gmisc.h gambitio.h
lemke.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
lemke.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
lemke.o: /usr/include/sys/types.h /usr/include/sys/signal.h
lemke.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
lemke.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
lemke.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
lemke.o: /usr/include/assert.h nfg.h gstring.h /usr/include/string.h
lemke.o: grblock.h grarray.h gnumber.h rational.h integer.h
lemke.o: /usr/include/math.h gpoly.h monomial.h prepoly.h glist.h gblock.h
lemke.o: gpolyctr.h lemke.h mixedsol.h mixed.h nfstrat.h gstatus.h gsignal.h
lemke.o: gprogres.h lhtab.h lemketab.h tableau.h ludecomp.h gmatrix.h
lemke.o: gsmatrix.h bfs.h gmap.h
nliap.o: nliap.h gambitio.h /usr/include/stdio.h gmisc.h gstatus.h gsignal.h
nliap.o: gprogres.h gstring.h /usr/include/string.h glist.h nfg.h garray.h
nliap.o: /usr/include/stdlib.h /usr/include/sys/wait.h
nliap.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
nliap.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
nliap.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
nliap.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
nliap.o: /usr/include/machine/frame.h /usr/include/assert.h grblock.h
nliap.o: grarray.h gnumber.h rational.h integer.h /usr/include/math.h gpoly.h
nliap.o: monomial.h prepoly.h gblock.h gvector.h gpolyctr.h mixed.h nfstrat.h
nliap.o: gpvector.h mixedsol.h gfunc.h
ngobit.o: /usr/include/math.h /usr/include/sys/stdsyms.h ngobit.h gambitio.h
ngobit.o: /usr/include/stdio.h gmisc.h gstatus.h gsignal.h gprogres.h
ngobit.o: gstring.h /usr/include/string.h glist.h nfg.h garray.h
ngobit.o: /usr/include/stdlib.h /usr/include/sys/wait.h
ngobit.o: /usr/include/sys/types.h /usr/include/sys/signal.h
ngobit.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
ngobit.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
ngobit.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
ngobit.o: /usr/include/assert.h grblock.h grarray.h gnumber.h rational.h
ngobit.o: integer.h gpoly.h monomial.h prepoly.h gblock.h gvector.h
ngobit.o: gpolyctr.h mixed.h nfstrat.h gpvector.h mixedsol.h gfunc.h
enum.o: enum.imp gwatch.h nfg.h garray.h /usr/include/stdlib.h
enum.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
enum.o: /usr/include/sys/types.h /usr/include/sys/signal.h
enum.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
enum.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
enum.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
enum.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
enum.o: gstring.h /usr/include/string.h grblock.h grarray.h gnumber.h
enum.o: rational.h integer.h /usr/include/math.h gpoly.h monomial.h prepoly.h
enum.o: glist.h gblock.h gvector.h gpolyctr.h nfgiter.h nfstrat.h enum.h
enum.o: gstatus.h gsignal.h gprogres.h mixed.h gpvector.h mixedsol.h
enum.o: vertenum.h tableau.h ludecomp.h gmatrix.h gsmatrix.h bfs.h gmap.h
simpdiv.o: simpdiv.imp gambitio.h /usr/include/stdio.h gmisc.h nfg.h garray.h
simpdiv.o: /usr/include/stdlib.h /usr/include/sys/wait.h
simpdiv.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
simpdiv.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
simpdiv.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
simpdiv.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
simpdiv.o: /usr/include/machine/frame.h /usr/include/assert.h gstring.h
simpdiv.o: /usr/include/string.h grblock.h grarray.h gnumber.h rational.h
simpdiv.o: integer.h /usr/include/math.h gpoly.h monomial.h prepoly.h glist.h
simpdiv.o: gblock.h gvector.h gpolyctr.h gwatch.h simpdiv.h gstatus.h
simpdiv.o: gsignal.h gprogres.h mixedsol.h mixed.h nfstrat.h gpvector.h
tableau.o: rational.h integer.h /usr/include/math.h
tableau.o: /usr/include/sys/stdsyms.h tableau.imp tableau.h ludecomp.h
tableau.o: gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h grarray.h
tableau.o: gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
tableau.o: /usr/include/sys/types.h /usr/include/sys/signal.h
tableau.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
tableau.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
tableau.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
tableau.o: /usr/include/assert.h garray.h gvector.h gsmatrix.h glist.h bfs.h
tableau.o: gmap.h
ludecomp.o: ludecomp.imp ludecomp.h gmatrix.h gambitio.h /usr/include/stdio.h
ludecomp.o: gmisc.h grarray.h gblock.h /usr/include/stdlib.h
ludecomp.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
ludecomp.o: /usr/include/sys/types.h /usr/include/sys/signal.h
ludecomp.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
ludecomp.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
ludecomp.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
ludecomp.o: /usr/include/assert.h garray.h gvector.h gsmatrix.h glist.h
ludecomp.o: glist.imp rational.h integer.h /usr/include/math.h
nfgpure.o: nfgpure.imp nfgpure.h nfg.h garray.h /usr/include/stdlib.h
nfgpure.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
nfgpure.o: /usr/include/sys/types.h /usr/include/sys/signal.h
nfgpure.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
nfgpure.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
nfgpure.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
nfgpure.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h gmisc.h
nfgpure.o: gstring.h /usr/include/string.h grblock.h grarray.h gnumber.h
nfgpure.o: rational.h integer.h /usr/include/math.h gpoly.h monomial.h
nfgpure.o: prepoly.h glist.h gblock.h gvector.h gpolyctr.h mixed.h nfstrat.h
nfgpure.o: gpvector.h mixedsol.h nfgiter.h nfgciter.h
lhtab.o: lhtab.h lemketab.h tableau.h rational.h integer.h
lhtab.o: /usr/include/math.h /usr/include/sys/stdsyms.h ludecomp.h gmatrix.h
lhtab.o: gambitio.h /usr/include/stdio.h gmisc.h grarray.h gblock.h
lhtab.o: /usr/include/stdlib.h /usr/include/sys/wait.h
lhtab.o: /usr/include/sys/types.h /usr/include/sys/signal.h
lhtab.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
lhtab.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
lhtab.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
lhtab.o: /usr/include/assert.h garray.h gvector.h gsmatrix.h glist.h bfs.h
lhtab.o: gmap.h gstatus.h gsignal.h gprogres.h gstring.h
lhtab.o: /usr/include/string.h nfg.h grblock.h gnumber.h gpoly.h monomial.h
lhtab.o: prepoly.h gpolyctr.h nfgiter.h nfstrat.h
lemketab.o: lemketab.h tableau.h rational.h integer.h /usr/include/math.h
lemketab.o: /usr/include/sys/stdsyms.h ludecomp.h gmatrix.h gambitio.h
lemketab.o: /usr/include/stdio.h gmisc.h grarray.h gblock.h
lemketab.o: /usr/include/stdlib.h /usr/include/sys/wait.h
lemketab.o: /usr/include/sys/types.h /usr/include/sys/signal.h
lemketab.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
lemketab.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
lemketab.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
lemketab.o: /usr/include/assert.h garray.h gvector.h gsmatrix.h glist.h bfs.h
lemketab.o: gmap.h gstatus.h gsignal.h gprogres.h gstring.h
lemketab.o: /usr/include/string.h
grid.o: /usr/include/math.h /usr/include/sys/stdsyms.h mixed.h gmisc.h
grid.o: nfstrat.h gstring.h /usr/include/string.h gblock.h
grid.o: /usr/include/stdlib.h /usr/include/sys/wait.h
grid.o: /usr/include/sys/types.h /usr/include/sys/signal.h
grid.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
grid.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
grid.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
grid.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
grid.o: gpvector.h gvector.h grid.h mixedsol.h gnumber.h rational.h integer.h
grid.o: glist.h gstatus.h gsignal.h gprogres.h nfg.h grblock.h grarray.h
grid.o: gpoly.h monomial.h prepoly.h gpolyctr.h gwatch.h
nfgcsum.o: nfgcsum.imp gwatch.h gpvector.h gvector.h gmisc.h gambitio.h
nfgcsum.o: /usr/include/stdio.h garray.h /usr/include/stdlib.h
nfgcsum.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
nfgcsum.o: /usr/include/sys/types.h /usr/include/sys/signal.h
nfgcsum.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
nfgcsum.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
nfgcsum.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
nfgcsum.o: /usr/include/assert.h nfg.h gstring.h /usr/include/string.h
nfgcsum.o: grblock.h grarray.h gnumber.h rational.h integer.h
nfgcsum.o: /usr/include/math.h gpoly.h monomial.h prepoly.h glist.h gblock.h
nfgcsum.o: gpolyctr.h nfgiter.h nfstrat.h nfgcsum.h gstatus.h gsignal.h
nfgcsum.o: gprogres.h mixedsol.h mixed.h lpsolve.h tableau.h ludecomp.h
nfgcsum.o: gmatrix.h gsmatrix.h bfs.h gmap.h vertenum.h
lpsolve.o: lpsolve.imp lpsolve.h tableau.h rational.h integer.h
lpsolve.o: /usr/include/math.h /usr/include/sys/stdsyms.h ludecomp.h
lpsolve.o: gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h grarray.h
lpsolve.o: gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
lpsolve.o: /usr/include/sys/types.h /usr/include/sys/signal.h
lpsolve.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
lpsolve.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
lpsolve.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
lpsolve.o: /usr/include/assert.h garray.h gvector.h gsmatrix.h glist.h bfs.h
lpsolve.o: gmap.h gstatus.h gsignal.h gprogres.h gstring.h
lpsolve.o: /usr/include/string.h vertenum.h
nfdom.o: nfg.h garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
nfdom.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
nfdom.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
nfdom.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
nfdom.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
nfdom.o: /usr/include/machine/frame.h /usr/include/assert.h gambitio.h
nfdom.o: /usr/include/stdio.h gmisc.h gstring.h /usr/include/string.h
nfdom.o: grblock.h grarray.h gnumber.h rational.h integer.h
nfdom.o: /usr/include/math.h gpoly.h monomial.h prepoly.h glist.h gblock.h
nfdom.o: gvector.h gpolyctr.h nfstrat.h nfgciter.h gstatus.h gsignal.h
nfdom.o: gprogres.h
vertenum.o: vertenum.imp vertenum.h tableau.h rational.h integer.h
vertenum.o: /usr/include/math.h /usr/include/sys/stdsyms.h ludecomp.h
vertenum.o: gmatrix.h gambitio.h /usr/include/stdio.h gmisc.h grarray.h
vertenum.o: gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
vertenum.o: /usr/include/sys/types.h /usr/include/sys/signal.h
vertenum.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
vertenum.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
vertenum.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
vertenum.o: /usr/include/assert.h garray.h gvector.h gsmatrix.h glist.h bfs.h
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
mixedsol.o: gpvector.h gvector.h gnumber.h rational.h integer.h
mixedsol.o: /usr/include/math.h
nfdommix.o: gambitio.h /usr/include/stdio.h gmisc.h nfg.h garray.h
nfdommix.o: /usr/include/stdlib.h /usr/include/sys/wait.h
nfdommix.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
nfdommix.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
nfdommix.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
nfdommix.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
nfdommix.o: /usr/include/machine/frame.h /usr/include/assert.h gstring.h
nfdommix.o: /usr/include/string.h grblock.h grarray.h gnumber.h rational.h
nfdommix.o: integer.h /usr/include/math.h gpoly.h monomial.h prepoly.h
nfdommix.o: glist.h gblock.h gvector.h gpolyctr.h nfgiter.h nfstrat.h
nfdommix.o: nfgciter.h lpsolve.h tableau.h ludecomp.h gmatrix.h gsmatrix.h
nfdommix.o: bfs.h gmap.h gstatus.h gsignal.h gprogres.h vertenum.h
efgshow.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
efgshow.o: ./wx_fake/wx_form.h efg.h gstring.h /usr/include/string.h gblock.h
efgshow.o: /usr/include/stdlib.h /usr/include/sys/wait.h
efgshow.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efgshow.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efgshow.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efgshow.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efgshow.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efgshow.o: gambitio.h gmisc.h glist.h gpvector.h gvector.h grblock.h
efgshow.o: grarray.h gnumber.h rational.h integer.h /usr/include/math.h
efgshow.o: gpoly.h monomial.h prepoly.h gpolyctr.h behav.h efstrat.h
efgshow.o: gdpvect.h efplayer.h infoset.h node.h outcome.h efgconst.h
efgshow.o: treewin.h treedraw.h treecons.h gambdraw.h efgshow.h efgnfgi.h
efgshow.o: gambit.h accels.h efgsolng.h behavsol.h bsolnsf.h gslist.h
efgshow.o: paramsd.h efgsoln.h spread.h nfggui.h ./wx_fake/wx_tbar.h
efgshow.o: elimdomd.h efgoutcd.h
efgshow1.o: ./wx_fake/wx.h ./wx_fake/wx_form.h wxmisc.h /usr/include/stdio.h
efgshow1.o: ./wx_fake/wx_timer.h efg.h gstring.h /usr/include/string.h
efgshow1.o: gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
efgshow1.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efgshow1.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efgshow1.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efgshow1.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efgshow1.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efgshow1.o: gambitio.h gmisc.h glist.h gpvector.h gvector.h grblock.h
efgshow1.o: grarray.h gnumber.h rational.h integer.h /usr/include/math.h
efgshow1.o: gpoly.h monomial.h prepoly.h gpolyctr.h behav.h efstrat.h
efgshow1.o: gdpvect.h efplayer.h infoset.h node.h outcome.h efgconst.h
efgshow1.o: gambit.h treewin.h treedraw.h treecons.h gambdraw.h efgshow.h
efgshow1.o: efgnfgi.h accels.h efgsolng.h behavsol.h bsolnsf.h gslist.h
efgshow1.o: paramsd.h efgsoln.h spread.h efsuptd.h elimdomd.h
efgshow1.o: ./wx_fake/wx_tbar.h efgaccl.h
treewin.o: bitmaps/copy.xpm bitmaps/move.xpm playersd.h bitmaps/scissor.xbm
treewin1.o: ./wx_fake/wx.h ./wx_fake/wx_mf.h ./wx_fake/wx_clipb.h wxmisc.h
treewin1.o: /usr/include/stdio.h ./wx_fake/wx_timer.h ./wx_fake/wx_form.h
treewin1.o: efg.h gstring.h /usr/include/string.h gblock.h
treewin1.o: /usr/include/stdlib.h /usr/include/sys/wait.h
treewin1.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
treewin1.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
treewin1.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
treewin1.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
treewin1.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
treewin1.o: gambitio.h gmisc.h glist.h gpvector.h gvector.h grblock.h
treewin1.o: grarray.h gnumber.h rational.h integer.h /usr/include/math.h
treewin1.o: gpoly.h monomial.h prepoly.h gpolyctr.h behav.h efstrat.h
treewin1.o: gdpvect.h efplayer.h infoset.h node.h outcome.h treewin.h
treewin1.o: treedraw.h treecons.h gambdraw.h efgconst.h
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
efgsoln.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
efgsoln.o: ./wx_fake/wx_form.h spread.h glist.h gambitio.h gmisc.h gblock.h
efgsoln.o: /usr/include/stdlib.h /usr/include/sys/wait.h
efgsoln.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efgsoln.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efgsoln.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efgsoln.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efgsoln.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efgsoln.o: grblock.h grarray.h gstring.h /usr/include/string.h rational.h
efgsoln.o: integer.h /usr/include/math.h efgsoln.h efg.h gpvector.h gvector.h
efgsoln.o: gnumber.h gpoly.h monomial.h prepoly.h gpolyctr.h behav.h
efgsoln.o: efstrat.h gdpvect.h efplayer.h infoset.h node.h outcome.h
efgsoln.o: efgconst.h efgshow.h efgnfgi.h gambit.h accels.h efgsolng.h
efgsoln.o: behavsol.h bsolnsf.h gslist.h paramsd.h treedraw.h treecons.h
efgsoln.o: gambdraw.h treewin.h
efgoutcd.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
efgoutcd.o: ./wx_fake/wx_form.h spread.h glist.h gambitio.h gmisc.h gblock.h
efgoutcd.o: /usr/include/stdlib.h /usr/include/sys/wait.h
efgoutcd.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efgoutcd.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efgoutcd.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efgoutcd.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efgoutcd.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efgoutcd.o: grblock.h grarray.h gstring.h /usr/include/string.h efg.h
efgoutcd.o: gpvector.h gvector.h gnumber.h rational.h integer.h
efgoutcd.o: /usr/include/math.h gpoly.h monomial.h prepoly.h gpolyctr.h
efgoutcd.o: behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h outcome.h
efgoutcd.o: treewin.h treedraw.h treecons.h gambdraw.h efgconst.h efgoutcd.h
efgoutcd.o: paramsd.h efgshow.h efgnfgi.h gambit.h accels.h efgsolng.h
efgoutcd.o: behavsol.h bsolnsf.h gslist.h
efgsolng.o: ./wx_fake/wx.h ./wx_fake/wx_form.h wxmisc.h /usr/include/stdio.h
efgsolng.o: ./wx_fake/wx_timer.h wxstatus.h gstatus.h gsignal.h gmisc.h
efgsolng.o: gprogres.h gambitio.h gstring.h /usr/include/string.h subsolve.h
efgsolng.o: efg.h gblock.h /usr/include/stdlib.h /usr/include/sys/wait.h
efgsolng.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
efgsolng.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
efgsolng.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
efgsolng.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
efgsolng.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
efgsolng.o: glist.h gpvector.h gvector.h grblock.h grarray.h gnumber.h
efgsolng.o: rational.h integer.h /usr/include/math.h gpoly.h monomial.h
efgsolng.o: prepoly.h gpolyctr.h behav.h efstrat.h gdpvect.h efplayer.h
efgsolng.o: infoset.h node.h outcome.h nfg.h behavsol.h gfunc.h efgsolng.h
efgsolng.o: nfgconst.h elimdomd.h nfstrat.h nliap.h mixed.h mixedsol.h
efgsolng.o: eliap.h liapsub.h liapprm.h algdlgs.h
bsolnsf.o: bsolnsf.h garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
bsolnsf.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
bsolnsf.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
bsolnsf.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
bsolnsf.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
bsolnsf.o: /usr/include/machine/frame.h /usr/include/assert.h gambitio.h
bsolnsf.o: /usr/include/stdio.h gmisc.h gslist.h glist.h behavsol.h behav.h
bsolnsf.o: gstring.h /usr/include/string.h efstrat.h gblock.h efg.h
bsolnsf.o: gpvector.h gvector.h grblock.h grarray.h gnumber.h rational.h
bsolnsf.o: integer.h /usr/include/math.h gpoly.h monomial.h prepoly.h
bsolnsf.o: gpolyctr.h efplayer.h infoset.h node.h outcome.h gdpvect.h
bsolnsf.o: gslist.imp
nfgshow.o: ./wx_fake/wx.h nfgshow.h wxmisc.h /usr/include/stdio.h
nfgshow.o: ./wx_fake/wx_timer.h ./wx_fake/wx_form.h spread.h glist.h
nfgshow.o: gambitio.h gmisc.h gblock.h /usr/include/stdlib.h
nfgshow.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
nfgshow.o: /usr/include/sys/types.h /usr/include/sys/signal.h
nfgshow.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
nfgshow.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
nfgshow.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
nfgshow.o: /usr/include/assert.h garray.h grblock.h grarray.h gstring.h
nfgshow.o: /usr/include/string.h nfgdraw.h gambdraw.h accels.h nfg.h
nfgshow.o: gnumber.h rational.h integer.h /usr/include/math.h gpoly.h
nfgshow.o: monomial.h prepoly.h gvector.h gpolyctr.h nfgiter.h nfstrat.h
nfgshow.o: mixedsol.h mixed.h gpvector.h nfgsolvd.h msolnsf.h gslist.h
nfgshow.o: efgnfgi.h paramsd.h nfgsoln.h nfgconst.h nfplayer.h nfgoutcd.h
nfgshow1.o: nfgshow.h ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h
nfgshow1.o: ./wx_fake/wx_timer.h ./wx_fake/wx_form.h spread.h glist.h
nfgshow1.o: gambitio.h gmisc.h gblock.h /usr/include/stdlib.h
nfgshow1.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
nfgshow1.o: /usr/include/sys/types.h /usr/include/sys/signal.h
nfgshow1.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
nfgshow1.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
nfgshow1.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
nfgshow1.o: /usr/include/assert.h garray.h grblock.h grarray.h gstring.h
nfgshow1.o: /usr/include/string.h nfgdraw.h gambdraw.h accels.h nfg.h
nfgshow1.o: gnumber.h rational.h integer.h /usr/include/math.h gpoly.h
nfgshow1.o: monomial.h prepoly.h gvector.h gpolyctr.h nfgiter.h nfstrat.h
nfgshow1.o: mixedsol.h mixed.h gpvector.h nfgsolvd.h msolnsf.h gslist.h
nfgshow1.o: efgnfgi.h paramsd.h nfplayer.h nfgoutcd.h nfgaccl.h nfgconst.h
nfgshow1.o: sprdaccl.h sprconst.h
nfgsoln.o: ./wx_fake/wx.h nfgconst.h nfgshow.h wxmisc.h /usr/include/stdio.h
nfgsoln.o: ./wx_fake/wx_timer.h ./wx_fake/wx_form.h spread.h glist.h
nfgsoln.o: gambitio.h gmisc.h gblock.h /usr/include/stdlib.h
nfgsoln.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
nfgsoln.o: /usr/include/sys/types.h /usr/include/sys/signal.h
nfgsoln.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
nfgsoln.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
nfgsoln.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
nfgsoln.o: /usr/include/assert.h garray.h grblock.h grarray.h gstring.h
nfgsoln.o: /usr/include/string.h nfgdraw.h gambdraw.h accels.h nfg.h
nfgsoln.o: gnumber.h rational.h integer.h /usr/include/math.h gpoly.h
nfgsoln.o: monomial.h prepoly.h gvector.h gpolyctr.h nfgiter.h nfstrat.h
nfgsoln.o: mixedsol.h mixed.h gpvector.h nfgsolvd.h msolnsf.h gslist.h
nfgsoln.o: efgnfgi.h paramsd.h nfgsoln.h nfplayer.h
msolnsf.o: msolnsf.h garray.h /usr/include/stdlib.h /usr/include/sys/wait.h
msolnsf.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
msolnsf.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
msolnsf.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
msolnsf.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
msolnsf.o: /usr/include/machine/frame.h /usr/include/assert.h gambitio.h
msolnsf.o: /usr/include/stdio.h gmisc.h gslist.h glist.h mixedsol.h mixed.h
msolnsf.o: nfstrat.h gstring.h /usr/include/string.h gblock.h gpvector.h
msolnsf.o: gvector.h gnumber.h rational.h integer.h /usr/include/math.h
msolnsf.o: gslist.imp
nfgoutcd.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
nfgoutcd.o: ./wx_fake/wx_form.h spread.h glist.h gambitio.h gmisc.h gblock.h
nfgoutcd.o: /usr/include/stdlib.h /usr/include/sys/wait.h
nfgoutcd.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
nfgoutcd.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
nfgoutcd.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
nfgoutcd.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
nfgoutcd.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
nfgoutcd.o: grblock.h grarray.h gstring.h /usr/include/string.h nfg.h
nfgoutcd.o: gnumber.h rational.h integer.h /usr/include/math.h gpoly.h
nfgoutcd.o: monomial.h prepoly.h gvector.h gpolyctr.h nfgshow.h nfgdraw.h
nfgoutcd.o: gambdraw.h accels.h nfgiter.h nfstrat.h mixedsol.h mixed.h
nfgoutcd.o: gpvector.h nfgsolvd.h msolnsf.h gslist.h efgnfgi.h paramsd.h
nfgoutcd.o: nfgoutcd.h nfplayer.h
wxmisc.o: ./wx_fake/wx.h ./wx_fake/wx_form.h ./wx_fake/wx_help.h wxmisc.h
wxmisc.o: /usr/include/stdio.h ./wx_fake/wx_timer.h
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
wxio.o: gambitio.h gmisc.h
spread.o: /usr/include/stdio.h ./wx_fake/wx.h ./wx_fake/wx_mf.h
spread.o: ./wx_fake/wx_tbar.h wxmisc.h ./wx_fake/wx_timer.h
spread.o: ./wx_fake/wx_form.h spread.h glist.h gambitio.h gmisc.h gblock.h
spread.o: /usr/include/stdlib.h /usr/include/sys/wait.h
spread.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
spread.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
spread.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
spread.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
spread.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
spread.o: grblock.h grarray.h gstring.h /usr/include/string.h
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
gambit.o: /usr/include/assert.h /usr/include/string.h /usr/include/ctype.h
gambit.o: ./wx_fake/wx.h ./wx_fake/wx_tbar.h wxio.h /usr/include/stdio.h
gambit.o: gambitio.h gmisc.h gambit.h wxmisc.h ./wx_fake/wx_timer.h
gambit.o: ./wx_fake/wx_form.h nfggui.h gstring.h efggui.h
gambit.o: /usr/include/signal.h /usr/include/sys/signal.h
gambit.o: /usr/include/sys/types.h /usr/include/sys/stdsyms.h
gambit.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
gambit.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
gambit.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h
gambit.o: /usr/include/math.h
accels.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
accels.o: ./wx_fake/wx_form.h keynames.h gmisc.h glist.imp glist.h gambitio.h
accels.o: /usr/include/assert.h accels.h garray.h /usr/include/stdlib.h
accels.o: /usr/include/sys/wait.h /usr/include/sys/stdsyms.h
accels.o: /usr/include/sys/types.h /usr/include/sys/signal.h
accels.o: /usr/include/sys/sigevent.h /usr/include/sys/siginfo.h
accels.o: /usr/include/sys/newsig.h /usr/include/machine/save_state.h
accels.o: /usr/include/sys/syscall.h /usr/include/machine/frame.h gstring.h
accels.o: /usr/include/string.h garray.imp
wxstatus.o: ./wx_fake/wx.h wxstatus.h gstatus.h gsignal.h gmisc.h gprogres.h
wxstatus.o: gambitio.h /usr/include/stdio.h gstring.h /usr/include/string.h
paramsd.o: ./wx_fake/wx.h wxmisc.h /usr/include/stdio.h ./wx_fake/wx_timer.h
paramsd.o: ./wx_fake/wx_form.h spread.h glist.h gambitio.h gmisc.h gblock.h
paramsd.o: /usr/include/stdlib.h /usr/include/sys/wait.h
paramsd.o: /usr/include/sys/stdsyms.h /usr/include/sys/types.h
paramsd.o: /usr/include/sys/signal.h /usr/include/sys/sigevent.h
paramsd.o: /usr/include/sys/siginfo.h /usr/include/sys/newsig.h
paramsd.o: /usr/include/machine/save_state.h /usr/include/sys/syscall.h
paramsd.o: /usr/include/machine/frame.h /usr/include/assert.h garray.h
paramsd.o: grblock.h grarray.h gstring.h /usr/include/string.h prepoly.h
paramsd.o: /usr/include/math.h rational.h integer.h gvector.h gnumber.h
paramsd.o: paramsd.h glist.imp
