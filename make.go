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
		gfunc.cc gclstats.cc glist.cc subsolve.cc gfuncmin.cc

GLIB_OBJECTS = gmisc.o gambitio.o garray.o gblock.o gstring.o integer.o \
		rational.o gnulib.o gvector.o gpvector.o gdpvect.o \
		grarray.o gmatrix.o bfs.o gwatch.o \
		gfunc.o gclstats.o glist.o subsolve.o gfuncmin.o

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
bfs.o:
