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
CFLAGS = -Wall  -fno-implicit-templates -I../

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
		efstrat.cc efgnfgd.cc efdom.cc efgnfg.cc

GAME_OBJECTS = readnfg.o readefg.o nfg.o nfgdbl.o nfgrat.o nfgutils.o \
		efg.o efgdbl.o efgrat.o nfstrat.o efgnfgr.o efgutils.o \
		efstrat.o efgnfgd.o efdom.o efgnfg.o

EALG_SOURCES =  egobit.cc eliap.cc seqform.cc efgcsum.cc \
		psnesub.cc efgpure.cc lemkesub.cc liapsub.cc simpsub.cc \
		enumsub.cc csumsub.cc behavsol.cc efgconv.cc

EALG_OBJECTS =  egobit.o eliap.o seqform.o efgcsum.o \
		psnesub.o efgpure.o lemkesub.o liapsub.o simpsub.o \
		enumsub.o csumsub.o behavsol.o efgconv.o

NALG_SOURCES = lemke.cc nliap.cc ngobit.cc enum.cc simpdiv.cc tableau.cc \
		ludecomp.cc nfgpure.cc lhtab.cc lemketab.cc grid.cc nfgcsum.cc\
		lpsolve.cc nfdom.cc vertenum.cc mixedsol.cc nfdommix.cc \
		nfgconv.cc

NALG_OBJECTS = lemke.o nliap.o ngobit.o enum.o simpdiv.o tableau.o ludecomp.o \
		nfgpure.o lhtab.o lemketab.o grid.o nfgcsum.o lpsolve.o nfdom.o\
		vertenum.o mixedsol.o nfdommix.o nfgconv.o

AGCL_SOURCES = gsmutils.cc gsm.cc gsmfunc.cc gsmoper.cc gsmhash.cc \
               gsminstr.cc portion.cc nfgfunc.cc efgfunc.cc listfunc.cc \
               algfunc.cc gcompile.cc gcl.cc gclsig.cc solfunc.cc gsmincl.cc\
	       system.cc gcmdline.cc

AGCL_OBJECTS = gsmutils.o gsm.o gsmfunc.o gsmoper.o gsmhash.o \
               gsminstr.o portion.o nfgfunc.o efgfunc.o listfunc.o \
               algfunc.o gcompile.o gcl.o gclsig.o solfunc.o gsmincl.o \
	       system.o gcmdline.o

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
BGUI_SOURCES = wxmisc.cc gambdraw.cc wxio.cc spread.cc spreadim.cc \
              gambit.cc accels.cc algdlgs.cc wxstatus.cc efgnfgi.cc 
BGUI_OBJECTS = wxmisc.o gambdraw.o wxio.o spread.o spreadim.o \
              gambit.o accels.o algdlgs.o wxstatus.o efgnfgi.o 

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
	makedepend -f make.go -I./wx_fake $(GCL_SOURCES) 
depend_gui:
	makedepend -f make.go -I./wx_fake $(GUI_SOURCES) 
clean:
	rm -f *.o core *~ *.bak gambgui gcl *.zip

$(OBJDIR):
	mkdir $(OBJDIR)
# DO NOT DELETE THIS LINE -- make depend depends on it.

gmisc.o: gmisc.h rational.h integer.h /usr/include/math.h
gmisc.o: /usr/include/features.h /usr/include/sys/cdefs.h
gmisc.o: /usr/include/huge_val.h /usr/include/endian.h /usr/include/bytesex.h
gmisc.o: /usr/include/nan.h /usr/include/values.h /usr/include/ieee754.h
gmisc.o: /usr/include/ieee854.h /usr/include/stdio.h /usr/include/libio.h
gmisc.o: /usr/include/_G_config.h /usr/include/stdlib.h /usr/include/errno.h
gmisc.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
gmisc.o: /usr/include/alloca.h /usr/include/limits.h
gmisc.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
gmisc.o: /usr/include/posix2_lim.h /usr/include/ctype.h gambitio.h gstring.h
gmisc.o: /usr/include/string.h
gambitio.o: /usr/include/assert.h /usr/include/features.h
gambitio.o: /usr/include/sys/cdefs.h gambitio.h /usr/include/stdio.h
gambitio.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h
garray.o: garray.imp /usr/include/stdlib.h /usr/include/features.h
garray.o: /usr/include/sys/cdefs.h /usr/include/errno.h
garray.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
garray.o: /usr/include/alloca.h /usr/include/assert.h garray.h gambitio.h
garray.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
garray.o: gmisc.h gstring.h /usr/include/string.h rational.h integer.h
garray.o: /usr/include/math.h /usr/include/huge_val.h /usr/include/endian.h
garray.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
garray.o: /usr/include/ieee754.h /usr/include/ieee854.h
gblock.o: gmisc.h gblock.imp /usr/include/stdlib.h /usr/include/features.h
gblock.o: /usr/include/sys/cdefs.h /usr/include/errno.h
gblock.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
gblock.o: /usr/include/alloca.h /usr/include/assert.h gblock.h garray.h
gblock.o: gambitio.h /usr/include/stdio.h /usr/include/libio.h
gblock.o: /usr/include/_G_config.h gstring.h /usr/include/string.h rational.h
gblock.o: integer.h /usr/include/math.h /usr/include/huge_val.h
gblock.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
gblock.o: /usr/include/values.h /usr/include/ieee754.h /usr/include/ieee854.h
gblock.o: garray.imp
gstring.o: /usr/include/stdlib.h /usr/include/features.h
gstring.o: /usr/include/sys/cdefs.h /usr/include/errno.h
gstring.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
gstring.o: /usr/include/alloca.h /usr/include/assert.h /usr/include/ctype.h
gstring.o: /usr/include/endian.h /usr/include/bytesex.h gambitio.h
gstring.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
gstring.o: gmisc.h gstring.h /usr/include/string.h
integer.o: gambitio.h /usr/include/stdio.h /usr/include/libio.h
integer.o: /usr/include/_G_config.h gmisc.h integer.h gnulib.h
integer.o: /usr/include/stdlib.h /usr/include/features.h
integer.o: /usr/include/sys/cdefs.h /usr/include/errno.h
integer.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
integer.o: /usr/include/alloca.h /usr/include/string.h /usr/include/memory.h
integer.o: /usr/include/fcntl.h /usr/include/sys/types.h
integer.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
integer.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
integer.o: /usr/include/sys/bitypes.h /usr/include/gnu/types.h
integer.o: /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h
integer.o: /usr/include/math.h /usr/include/huge_val.h /usr/include/endian.h
integer.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
integer.o: /usr/include/ieee754.h /usr/include/ieee854.h /usr/include/ctype.h
integer.o: /usr/include/limits.h /usr/include/posix1_lim.h
integer.o: /usr/include/linux/limits.h /usr/include/posix2_lim.h
integer.o: /usr/include/assert.h
rational.o: gambitio.h /usr/include/stdio.h /usr/include/libio.h
rational.o: /usr/include/_G_config.h gmisc.h rational.h integer.h
rational.o: /usr/include/math.h /usr/include/features.h
rational.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h
rational.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
rational.o: /usr/include/values.h /usr/include/ieee754.h
rational.o: /usr/include/ieee854.h gnulib.h /usr/include/stdlib.h
rational.o: /usr/include/errno.h /usr/include/linux/errno.h
rational.o: /usr/include/asm/errno.h /usr/include/alloca.h
rational.o: /usr/include/string.h /usr/include/memory.h /usr/include/fcntl.h
rational.o: /usr/include/sys/types.h /usr/include/linux/types.h
rational.o: /usr/include/linux/posix_types.h /usr/include/asm/posix_types.h
rational.o: /usr/include/asm/types.h /usr/include/sys/bitypes.h
rational.o: /usr/include/gnu/types.h /usr/include/linux/fcntl.h
rational.o: /usr/include/asm/fcntl.h /usr/include/assert.h
rational.o: /usr/include/ctype.h
gnulib.o: /usr/include/assert.h /usr/include/features.h
gnulib.o: /usr/include/sys/cdefs.h /usr/include/values.h gnulib.h
gnulib.o: /usr/include/stdlib.h /usr/include/errno.h
gnulib.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
gnulib.o: /usr/include/alloca.h /usr/include/string.h /usr/include/memory.h
gnulib.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
gnulib.o: /usr/include/fcntl.h /usr/include/sys/types.h
gnulib.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
gnulib.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
gnulib.o: /usr/include/sys/bitypes.h /usr/include/gnu/types.h
gnulib.o: /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h
gnulib.o: /usr/include/math.h /usr/include/huge_val.h /usr/include/endian.h
gnulib.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/ieee754.h
gnulib.o: /usr/include/ieee854.h
gvector.o: gvector.imp gvector.h gmisc.h gambitio.h /usr/include/stdio.h
gvector.o: /usr/include/libio.h /usr/include/_G_config.h garray.h
gvector.o: /usr/include/stdlib.h /usr/include/features.h
gvector.o: /usr/include/sys/cdefs.h /usr/include/errno.h
gvector.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
gvector.o: /usr/include/alloca.h /usr/include/assert.h rational.h integer.h
gvector.o: /usr/include/math.h /usr/include/huge_val.h /usr/include/endian.h
gvector.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
gvector.o: /usr/include/ieee754.h /usr/include/ieee854.h
gpvector.o: gpvector.imp gpvector.h gvector.h gmisc.h gambitio.h
gpvector.o: /usr/include/stdio.h /usr/include/libio.h
gpvector.o: /usr/include/_G_config.h garray.h /usr/include/stdlib.h
gpvector.o: /usr/include/features.h /usr/include/sys/cdefs.h
gpvector.o: /usr/include/errno.h /usr/include/linux/errno.h
gpvector.o: /usr/include/asm/errno.h /usr/include/alloca.h
gpvector.o: /usr/include/assert.h rational.h integer.h /usr/include/math.h
gpvector.o: /usr/include/huge_val.h /usr/include/endian.h
gpvector.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
gpvector.o: /usr/include/ieee754.h /usr/include/ieee854.h
gdpvect.o: gdpvect.imp gdpvect.h gpvector.h gvector.h gmisc.h gambitio.h
gdpvect.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
gdpvect.o: garray.h /usr/include/stdlib.h /usr/include/features.h
gdpvect.o: /usr/include/sys/cdefs.h /usr/include/errno.h
gdpvect.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
gdpvect.o: /usr/include/alloca.h /usr/include/assert.h rational.h integer.h
gdpvect.o: /usr/include/math.h /usr/include/huge_val.h /usr/include/endian.h
gdpvect.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
gdpvect.o: /usr/include/ieee754.h /usr/include/ieee854.h
grarray.o: grarray.imp grarray.h gambitio.h /usr/include/stdio.h
grarray.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h
grarray.o: /usr/include/assert.h /usr/include/features.h
grarray.o: /usr/include/sys/cdefs.h garray.h /usr/include/stdlib.h
grarray.o: /usr/include/errno.h /usr/include/linux/errno.h
grarray.o: /usr/include/asm/errno.h /usr/include/alloca.h rational.h
grarray.o: integer.h /usr/include/math.h /usr/include/huge_val.h
grarray.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
grarray.o: /usr/include/values.h /usr/include/ieee754.h
grarray.o: /usr/include/ieee854.h
gmatrix.o: gmatrix.imp gmatrix.h gambitio.h /usr/include/stdio.h
gmatrix.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h grarray.h
gmatrix.o: gblock.h /usr/include/stdlib.h /usr/include/features.h
gmatrix.o: /usr/include/sys/cdefs.h /usr/include/errno.h
gmatrix.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
gmatrix.o: /usr/include/alloca.h /usr/include/assert.h garray.h gvector.h
gmatrix.o: gsmatrix.h rational.h integer.h /usr/include/math.h
gmatrix.o: /usr/include/huge_val.h /usr/include/endian.h
gmatrix.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
gmatrix.o: /usr/include/ieee754.h /usr/include/ieee854.h
gclsig.o: gsignal.h gmisc.h /usr/include/signal.h /usr/include/features.h
gclsig.o: /usr/include/sys/cdefs.h /usr/include/sys/types.h
gclsig.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
gclsig.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
gclsig.o: /usr/include/sys/bitypes.h /usr/include/linux/signal.h
gclsig.o: /usr/include/asm/signal.h
glpsolve.o: glpsolve.imp glpsolve.h gtableau.h /usr/include/assert.h
glpsolve.o: /usr/include/features.h /usr/include/sys/cdefs.h gmisc.h
glpsolve.o: gambitio.h /usr/include/stdio.h /usr/include/libio.h
glpsolve.o: /usr/include/_G_config.h gmatrix.h grarray.h gblock.h
glpsolve.o: /usr/include/stdlib.h /usr/include/errno.h
glpsolve.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
glpsolve.o: /usr/include/alloca.h garray.h gvector.h gsmatrix.h gmap.h bfs.h
glpsolve.o: rational.h integer.h /usr/include/math.h /usr/include/huge_val.h
glpsolve.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
glpsolve.o: /usr/include/values.h /usr/include/ieee754.h
glpsolve.o: /usr/include/ieee854.h gtableau.imp
bfs.o: rational.h integer.h /usr/include/math.h /usr/include/features.h
bfs.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h /usr/include/endian.h
bfs.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
bfs.o: /usr/include/ieee754.h /usr/include/ieee854.h garray.imp
bfs.o: /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h
bfs.o: /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/assert.h
bfs.o: garray.h gambitio.h /usr/include/stdio.h /usr/include/libio.h
bfs.o: /usr/include/_G_config.h gmisc.h gblock.imp gblock.h glist.imp glist.h
bfs.o: gmap.imp gmap.h /usr/include/string.h bfs.h
gwatch.o: gwatch.h /usr/include/stdio.h /usr/include/libio.h
gwatch.o: /usr/include/_G_config.h /usr/include/math.h
gwatch.o: /usr/include/features.h /usr/include/sys/cdefs.h
gwatch.o: /usr/include/huge_val.h /usr/include/endian.h
gwatch.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
gwatch.o: /usr/include/ieee754.h /usr/include/ieee854.h
gfunc.o: gfunc.h gmisc.h
gclstats.o: gstatus.h gsignal.h gmisc.h gprogres.h gambitio.h
gclstats.o: /usr/include/stdio.h /usr/include/libio.h
gclstats.o: /usr/include/_G_config.h gstring.h /usr/include/string.h
gclstats.o: /usr/include/features.h /usr/include/sys/cdefs.h
gclstats.o: /usr/include/signal.h /usr/include/sys/types.h
gclstats.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
gclstats.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
gclstats.o: /usr/include/sys/bitypes.h /usr/include/linux/signal.h
gclstats.o: /usr/include/asm/signal.h
glist.o: gmisc.h glist.imp glist.h gambitio.h /usr/include/stdio.h
glist.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/assert.h
glist.o: /usr/include/features.h /usr/include/sys/cdefs.h gstring.h
glist.o: /usr/include/string.h rational.h integer.h /usr/include/math.h
glist.o: /usr/include/huge_val.h /usr/include/endian.h /usr/include/bytesex.h
glist.o: /usr/include/nan.h /usr/include/values.h /usr/include/ieee754.h
glist.o: /usr/include/ieee854.h garray.h /usr/include/stdlib.h
glist.o: /usr/include/errno.h /usr/include/linux/errno.h
glist.o: /usr/include/asm/errno.h /usr/include/alloca.h gblock.h
subsolve.o: efg.h gstring.h /usr/include/string.h /usr/include/features.h
subsolve.o: /usr/include/sys/cdefs.h gblock.h /usr/include/stdlib.h
subsolve.o: /usr/include/errno.h /usr/include/linux/errno.h
subsolve.o: /usr/include/asm/errno.h /usr/include/alloca.h
subsolve.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
subsolve.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h glist.h
subsolve.o: gpvector.h gvector.h behav.h efstrat.h gdpvect.h efplayer.h
subsolve.o: infoset.h rational.h integer.h /usr/include/math.h
subsolve.o: /usr/include/huge_val.h /usr/include/endian.h
subsolve.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
subsolve.o: /usr/include/ieee754.h /usr/include/ieee854.h node.h outcome.h
subsolve.o: efgutils.h nfg.h nfstrat.h gwatch.h subsolve.imp subsolve.h
subsolve.o: behavsol.h garray.imp glist.imp
gfuncmin.o: /usr/include/math.h /usr/include/features.h
gfuncmin.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h
gfuncmin.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
gfuncmin.o: /usr/include/values.h /usr/include/ieee754.h
gfuncmin.o: /usr/include/ieee854.h gfunc.h gmisc.h gstatus.h gsignal.h
gfuncmin.o: gprogres.h gambitio.h /usr/include/stdio.h /usr/include/libio.h
gfuncmin.o: /usr/include/_G_config.h gstring.h /usr/include/string.h
gfuncmin.o: gvector.h garray.h /usr/include/stdlib.h /usr/include/errno.h
gfuncmin.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
gfuncmin.o: /usr/include/alloca.h /usr/include/assert.h gpvector.h gmatrix.h
gfuncmin.o: grarray.h gblock.h gsmatrix.h
readnfg.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
readnfg.o: /usr/include/ctype.h /usr/include/features.h
readnfg.o: /usr/include/sys/cdefs.h /usr/include/endian.h
readnfg.o: /usr/include/bytesex.h gmisc.h gambitio.h glist.h rational.h
readnfg.o: integer.h /usr/include/math.h /usr/include/huge_val.h
readnfg.o: /usr/include/nan.h /usr/include/values.h /usr/include/ieee754.h
readnfg.o: /usr/include/ieee854.h nfg.h garray.h /usr/include/stdlib.h
readnfg.o: /usr/include/errno.h /usr/include/linux/errno.h
readnfg.o: /usr/include/asm/errno.h /usr/include/alloca.h
readnfg.o: /usr/include/assert.h gstring.h /usr/include/string.h gpvector.h
readnfg.o: gvector.h
readefg.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
readefg.o: /usr/include/stdlib.h /usr/include/features.h
readefg.o: /usr/include/sys/cdefs.h /usr/include/errno.h
readefg.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
readefg.o: /usr/include/alloca.h /usr/include/ctype.h /usr/include/endian.h
readefg.o: /usr/include/bytesex.h gambitio.h gmisc.h gstring.h
readefg.o: /usr/include/string.h rational.h integer.h /usr/include/math.h
readefg.o: /usr/include/huge_val.h /usr/include/nan.h /usr/include/values.h
readefg.o: /usr/include/ieee754.h /usr/include/ieee854.h gstack.h glist.h
readefg.o: efg.h gblock.h /usr/include/assert.h garray.h gpvector.h gvector.h
readefg.o: behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h outcome.h
readefg.o: gstack.imp
nfg.o: /usr/include/assert.h /usr/include/features.h /usr/include/sys/cdefs.h
nfg.o: nfg.h garray.h /usr/include/stdlib.h /usr/include/errno.h
nfg.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
nfg.o: /usr/include/alloca.h gambitio.h /usr/include/stdio.h
nfg.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h gstring.h
nfg.o: /usr/include/string.h gpvector.h gvector.h nfstrat.h gblock.h
nfg.o: nfplayer.h efg.h glist.h behav.h efstrat.h gdpvect.h efplayer.h
nfg.o: infoset.h rational.h integer.h /usr/include/math.h
nfg.o: /usr/include/huge_val.h /usr/include/endian.h /usr/include/bytesex.h
nfg.o: /usr/include/nan.h /usr/include/values.h /usr/include/ieee754.h
nfg.o: /usr/include/ieee854.h node.h outcome.h lexicon.h mixed.h garray.imp
nfg.o: gblock.imp
nfgdbl.o: rational.h integer.h /usr/include/math.h /usr/include/features.h
nfgdbl.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h
nfgdbl.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
nfgdbl.o: /usr/include/values.h /usr/include/ieee754.h /usr/include/ieee854.h
nfgdbl.o: nfg.h garray.h /usr/include/stdlib.h /usr/include/errno.h
nfgdbl.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
nfgdbl.o: /usr/include/alloca.h /usr/include/assert.h gambitio.h
nfgdbl.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
nfgdbl.o: gmisc.h gstring.h /usr/include/string.h gpvector.h gvector.h
nfgdbl.o: nfg.imp nfplayer.h nfstrat.h gblock.h nfgiter.h mixed.h nfgiter.imp
nfgdbl.o: nfgciter.imp nfgciter.h readnfg.imp glist.h readnfg.h mixedsol.imp
nfgdbl.o: mixedsol.h garray.imp glist.imp
nfgrat.o: rational.h integer.h /usr/include/math.h /usr/include/features.h
nfgrat.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h
nfgrat.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
nfgrat.o: /usr/include/values.h /usr/include/ieee754.h /usr/include/ieee854.h
nfgrat.o: nfg.h garray.h /usr/include/stdlib.h /usr/include/errno.h
nfgrat.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
nfgrat.o: /usr/include/alloca.h /usr/include/assert.h gambitio.h
nfgrat.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
nfgrat.o: gmisc.h gstring.h /usr/include/string.h gpvector.h gvector.h
nfgrat.o: nfg.imp nfplayer.h nfstrat.h gblock.h nfgiter.h mixed.h nfgiter.imp
nfgrat.o: nfgciter.imp nfgciter.h readnfg.imp glist.h readnfg.h mixedsol.imp
nfgrat.o: mixedsol.h garray.imp glist.imp
nfgutils.o: gmisc.h nfg.h garray.h /usr/include/stdlib.h
nfgutils.o: /usr/include/features.h /usr/include/sys/cdefs.h
nfgutils.o: /usr/include/errno.h /usr/include/linux/errno.h
nfgutils.o: /usr/include/asm/errno.h /usr/include/alloca.h
nfgutils.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h
nfgutils.o: /usr/include/libio.h /usr/include/_G_config.h gstring.h
nfgutils.o: /usr/include/string.h gpvector.h gvector.h nfplayer.h nfstrat.h
nfgutils.o: gblock.h nfgciter.h rational.h integer.h /usr/include/math.h
nfgutils.o: /usr/include/huge_val.h /usr/include/endian.h
nfgutils.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
nfgutils.o: /usr/include/ieee754.h /usr/include/ieee854.h
efg.o: garray.h /usr/include/stdlib.h /usr/include/features.h
efg.o: /usr/include/sys/cdefs.h /usr/include/errno.h
efg.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
efg.o: /usr/include/alloca.h /usr/include/assert.h gambitio.h
efg.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
efg.o: gmisc.h rational.h integer.h /usr/include/math.h
efg.o: /usr/include/huge_val.h /usr/include/endian.h /usr/include/bytesex.h
efg.o: /usr/include/nan.h /usr/include/values.h /usr/include/ieee754.h
efg.o: /usr/include/ieee854.h garray.imp gblock.imp gblock.h glist.imp
efg.o: glist.h efg.h gstring.h /usr/include/string.h gpvector.h gvector.h
efg.o: behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h outcome.h
efg.o: efgutils.h
efgdbl.o: rational.h integer.h /usr/include/math.h /usr/include/features.h
efgdbl.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h
efgdbl.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
efgdbl.o: /usr/include/values.h /usr/include/ieee754.h /usr/include/ieee854.h
efgdbl.o: efg.imp efg.h gstring.h /usr/include/string.h gblock.h
efgdbl.o: /usr/include/stdlib.h /usr/include/errno.h
efgdbl.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
efgdbl.o: /usr/include/alloca.h /usr/include/assert.h garray.h gambitio.h
efgdbl.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
efgdbl.o: gmisc.h glist.h gpvector.h gvector.h behav.h efstrat.h gdpvect.h
efgdbl.o: efplayer.h infoset.h node.h outcome.h efgutils.h tnode.h
efgdbl.o: behavsol.imp behavsol.h efgiter.imp efgciter.h efgiter.h
efgdbl.o: efgciter.imp readefg.imp gstack.h readefg.h glist.imp
efgrat.o: rational.h integer.h /usr/include/math.h /usr/include/features.h
efgrat.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h
efgrat.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
efgrat.o: /usr/include/values.h /usr/include/ieee754.h /usr/include/ieee854.h
efgrat.o: efg.imp efg.h gstring.h /usr/include/string.h gblock.h
efgrat.o: /usr/include/stdlib.h /usr/include/errno.h
efgrat.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
efgrat.o: /usr/include/alloca.h /usr/include/assert.h garray.h gambitio.h
efgrat.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
efgrat.o: gmisc.h glist.h gpvector.h gvector.h behav.h efstrat.h gdpvect.h
efgrat.o: efplayer.h infoset.h node.h outcome.h efgutils.h tnode.h
efgrat.o: behavsol.imp behavsol.h efgiter.imp efgciter.h efgiter.h
efgrat.o: efgciter.imp readefg.imp gstack.h readefg.h glist.imp
nfstrat.o: nfstrat.h gstring.h /usr/include/string.h /usr/include/features.h
nfstrat.o: /usr/include/sys/cdefs.h gblock.h /usr/include/stdlib.h
nfstrat.o: /usr/include/errno.h /usr/include/linux/errno.h
nfstrat.o: /usr/include/asm/errno.h /usr/include/alloca.h
nfstrat.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
nfstrat.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h nfplayer.h
nfstrat.o: nfg.h gpvector.h gvector.h
efgnfgr.o: efgnfg.imp tnode.h efg.h gstring.h /usr/include/string.h
efgnfgr.o: /usr/include/features.h /usr/include/sys/cdefs.h gblock.h
efgnfgr.o: /usr/include/stdlib.h /usr/include/errno.h
efgnfgr.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
efgnfgr.o: /usr/include/alloca.h /usr/include/assert.h garray.h gambitio.h
efgnfgr.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
efgnfgr.o: gmisc.h glist.h gpvector.h gvector.h behav.h efstrat.h gdpvect.h
efgnfgr.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h
efgnfgr.o: /usr/include/huge_val.h /usr/include/endian.h
efgnfgr.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
efgnfgr.o: /usr/include/ieee754.h /usr/include/ieee854.h node.h outcome.h
efgnfgr.o: nfg.h nfplayer.h nfstrat.h nfgiter.h nfgciter.h mixed.h lexicon.h
efgutils.o: efgutils.h efg.h gstring.h /usr/include/string.h
efgutils.o: /usr/include/features.h /usr/include/sys/cdefs.h gblock.h
efgutils.o: /usr/include/stdlib.h /usr/include/errno.h
efgutils.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
efgutils.o: /usr/include/alloca.h /usr/include/assert.h garray.h gambitio.h
efgutils.o: /usr/include/stdio.h /usr/include/libio.h
efgutils.o: /usr/include/_G_config.h gmisc.h glist.h gpvector.h gvector.h
efgutils.o: behav.h efstrat.h gdpvect.h efplayer.h infoset.h rational.h
efgutils.o: integer.h /usr/include/math.h /usr/include/huge_val.h
efgutils.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
efgutils.o: /usr/include/values.h /usr/include/ieee754.h
efgutils.o: /usr/include/ieee854.h node.h outcome.h
efstrat.o: efg.h gstring.h /usr/include/string.h /usr/include/features.h
efstrat.o: /usr/include/sys/cdefs.h gblock.h /usr/include/stdlib.h
efstrat.o: /usr/include/errno.h /usr/include/linux/errno.h
efstrat.o: /usr/include/asm/errno.h /usr/include/alloca.h
efstrat.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
efstrat.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h glist.h
efstrat.o: gpvector.h gvector.h behav.h efstrat.h gdpvect.h efplayer.h
efstrat.o: infoset.h rational.h integer.h /usr/include/math.h
efstrat.o: /usr/include/huge_val.h /usr/include/endian.h
efstrat.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
efstrat.o: /usr/include/ieee754.h /usr/include/ieee854.h node.h outcome.h
efgnfgd.o: efgnfg.imp tnode.h efg.h gstring.h /usr/include/string.h
efgnfgd.o: /usr/include/features.h /usr/include/sys/cdefs.h gblock.h
efgnfgd.o: /usr/include/stdlib.h /usr/include/errno.h
efgnfgd.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
efgnfgd.o: /usr/include/alloca.h /usr/include/assert.h garray.h gambitio.h
efgnfgd.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
efgnfgd.o: gmisc.h glist.h gpvector.h gvector.h behav.h efstrat.h gdpvect.h
efgnfgd.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h
efgnfgd.o: /usr/include/huge_val.h /usr/include/endian.h
efgnfgd.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
efgnfgd.o: /usr/include/ieee754.h /usr/include/ieee854.h node.h outcome.h
efgnfgd.o: nfg.h nfplayer.h nfstrat.h nfgiter.h nfgciter.h mixed.h lexicon.h
efdom.o: efg.h gstring.h /usr/include/string.h /usr/include/features.h
efdom.o: /usr/include/sys/cdefs.h gblock.h /usr/include/stdlib.h
efdom.o: /usr/include/errno.h /usr/include/linux/errno.h
efdom.o: /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/assert.h
efdom.o: garray.h gambitio.h /usr/include/stdio.h /usr/include/libio.h
efdom.o: /usr/include/_G_config.h gmisc.h glist.h gpvector.h gvector.h
efdom.o: behav.h efstrat.h gdpvect.h efplayer.h infoset.h rational.h
efdom.o: integer.h /usr/include/math.h /usr/include/huge_val.h
efdom.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
efdom.o: /usr/include/values.h /usr/include/ieee754.h /usr/include/ieee854.h
efdom.o: node.h outcome.h efgciter.h gstatus.h gsignal.h gprogres.h
efgnfg.o: efg.h gstring.h /usr/include/string.h /usr/include/features.h
efgnfg.o: /usr/include/sys/cdefs.h gblock.h /usr/include/stdlib.h
efgnfg.o: /usr/include/errno.h /usr/include/linux/errno.h
efgnfg.o: /usr/include/asm/errno.h /usr/include/alloca.h
efgnfg.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
efgnfg.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h glist.h
efgnfg.o: gpvector.h gvector.h behav.h efstrat.h gdpvect.h efplayer.h
efgnfg.o: infoset.h rational.h integer.h /usr/include/math.h
efgnfg.o: /usr/include/huge_val.h /usr/include/endian.h
efgnfg.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
efgnfg.o: /usr/include/ieee754.h /usr/include/ieee854.h node.h outcome.h
efgnfg.o: nfg.h nfplayer.h nfstrat.h mixed.h nfgiter.h nfgciter.h glist.imp
efgnfg.o: garray.imp lexicon.h
egobit.o: /usr/include/math.h /usr/include/features.h
egobit.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h
egobit.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
egobit.o: /usr/include/values.h /usr/include/ieee754.h /usr/include/ieee854.h
egobit.o: egobit.h gambitio.h /usr/include/stdio.h /usr/include/libio.h
egobit.o: /usr/include/_G_config.h gmisc.h gstatus.h gsignal.h gprogres.h
egobit.o: gstring.h /usr/include/string.h glist.h efg.h gblock.h
egobit.o: /usr/include/stdlib.h /usr/include/errno.h
egobit.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
egobit.o: /usr/include/alloca.h /usr/include/assert.h garray.h gpvector.h
egobit.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h
egobit.o: rational.h integer.h node.h outcome.h behavsol.h gfunc.h gmatrix.h
egobit.o: grarray.h gsmatrix.h
eliap.o: eliap.h gambitio.h /usr/include/stdio.h /usr/include/libio.h
eliap.o: /usr/include/_G_config.h gmisc.h gstatus.h gsignal.h gprogres.h
eliap.o: gstring.h /usr/include/string.h /usr/include/features.h
eliap.o: /usr/include/sys/cdefs.h glist.h efg.h gblock.h
eliap.o: /usr/include/stdlib.h /usr/include/errno.h
eliap.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
eliap.o: /usr/include/alloca.h /usr/include/assert.h garray.h gpvector.h
eliap.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h
eliap.o: rational.h integer.h /usr/include/math.h /usr/include/huge_val.h
eliap.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
eliap.o: /usr/include/values.h /usr/include/ieee754.h /usr/include/ieee854.h
eliap.o: node.h outcome.h behavsol.h subsolve.h nfg.h gfunc.h gmatrix.h
eliap.o: grarray.h gsmatrix.h
seqform.o: seqform.imp seqform.h efg.h gstring.h /usr/include/string.h
seqform.o: /usr/include/features.h /usr/include/sys/cdefs.h gblock.h
seqform.o: /usr/include/stdlib.h /usr/include/errno.h
seqform.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
seqform.o: /usr/include/alloca.h /usr/include/assert.h garray.h gambitio.h
seqform.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
seqform.o: gmisc.h glist.h gpvector.h gvector.h behav.h efstrat.h gdpvect.h
seqform.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h
seqform.o: /usr/include/huge_val.h /usr/include/endian.h
seqform.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
seqform.o: /usr/include/ieee754.h /usr/include/ieee854.h node.h outcome.h
seqform.o: gmatrix.h grarray.h gsmatrix.h lemketab.h tableau.h ludecomp.h
seqform.o: bfs.h gmap.h gstatus.h gsignal.h gprogres.h behavsol.h subsolve.h
seqform.o: nfg.h gwatch.h
efgcsum.o: efgcsum.imp gwatch.h gpvector.h gvector.h gmisc.h gambitio.h
efgcsum.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
efgcsum.o: garray.h /usr/include/stdlib.h /usr/include/features.h
efgcsum.o: /usr/include/sys/cdefs.h /usr/include/errno.h
efgcsum.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
efgcsum.o: /usr/include/alloca.h /usr/include/assert.h efgutils.h efg.h
efgcsum.o: gstring.h /usr/include/string.h gblock.h glist.h behav.h efstrat.h
efgcsum.o: gdpvect.h efplayer.h infoset.h rational.h integer.h
efgcsum.o: /usr/include/math.h /usr/include/huge_val.h /usr/include/endian.h
efgcsum.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
efgcsum.o: /usr/include/ieee754.h /usr/include/ieee854.h node.h outcome.h
efgcsum.o: efgcsum.h gstatus.h gsignal.h gprogres.h tableau.h ludecomp.h
efgcsum.o: gmatrix.h grarray.h gsmatrix.h bfs.h gmap.h lpsolve.h behavsol.h
efgcsum.o: subsolve.h nfg.h
psnesub.o: rational.h integer.h /usr/include/math.h /usr/include/features.h
psnesub.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h
psnesub.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
psnesub.o: /usr/include/values.h /usr/include/ieee754.h
psnesub.o: /usr/include/ieee854.h psnesub.imp psnesub.h subsolve.h efg.h
psnesub.o: gstring.h /usr/include/string.h gblock.h /usr/include/stdlib.h
psnesub.o: /usr/include/errno.h /usr/include/linux/errno.h
psnesub.o: /usr/include/asm/errno.h /usr/include/alloca.h
psnesub.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
psnesub.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h glist.h
psnesub.o: gpvector.h gvector.h behav.h efstrat.h gdpvect.h efplayer.h
psnesub.o: infoset.h node.h outcome.h nfg.h behavsol.h nfgpure.h mixed.h
psnesub.o: nfstrat.h mixedsol.h
efgpure.o: efgpure.imp glist.h gambitio.h /usr/include/stdio.h
efgpure.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h efg.h
efgpure.o: gstring.h /usr/include/string.h /usr/include/features.h
efgpure.o: /usr/include/sys/cdefs.h gblock.h /usr/include/stdlib.h
efgpure.o: /usr/include/errno.h /usr/include/linux/errno.h
efgpure.o: /usr/include/asm/errno.h /usr/include/alloca.h
efgpure.o: /usr/include/assert.h garray.h gpvector.h gvector.h behav.h
efgpure.o: efstrat.h gdpvect.h efplayer.h infoset.h rational.h integer.h
efgpure.o: /usr/include/math.h /usr/include/huge_val.h /usr/include/endian.h
efgpure.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
efgpure.o: /usr/include/ieee754.h /usr/include/ieee854.h node.h outcome.h
efgpure.o: efgiter.h efgciter.h behavsol.h efgpure.h subsolve.h nfg.h
lemkesub.o: rational.h integer.h /usr/include/math.h /usr/include/features.h
lemkesub.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h
lemkesub.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
lemkesub.o: /usr/include/values.h /usr/include/ieee754.h
lemkesub.o: /usr/include/ieee854.h lemkesub.imp lemkesub.h lemke.h nfg.h
lemkesub.o: garray.h /usr/include/stdlib.h /usr/include/errno.h
lemkesub.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
lemkesub.o: /usr/include/alloca.h /usr/include/assert.h gambitio.h
lemkesub.o: /usr/include/stdio.h /usr/include/libio.h
lemkesub.o: /usr/include/_G_config.h gmisc.h gstring.h /usr/include/string.h
lemkesub.o: gpvector.h gvector.h mixedsol.h mixed.h nfstrat.h gblock.h
lemkesub.o: glist.h lhtab.h lemketab.h tableau.h ludecomp.h gmatrix.h
lemkesub.o: grarray.h gsmatrix.h bfs.h gmap.h gstatus.h gsignal.h gprogres.h
lemkesub.o: subsolve.h efg.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h
lemkesub.o: node.h outcome.h behavsol.h
liapsub.o: liapsub.h nliap.h gambitio.h /usr/include/stdio.h
liapsub.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h gstatus.h
liapsub.o: gsignal.h gprogres.h gstring.h /usr/include/string.h
liapsub.o: /usr/include/features.h /usr/include/sys/cdefs.h glist.h nfg.h
liapsub.o: garray.h /usr/include/stdlib.h /usr/include/errno.h
liapsub.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
liapsub.o: /usr/include/alloca.h /usr/include/assert.h gpvector.h gvector.h
liapsub.o: mixed.h nfstrat.h gblock.h mixedsol.h subsolve.h efg.h behav.h
liapsub.o: efstrat.h gdpvect.h efplayer.h infoset.h rational.h integer.h
liapsub.o: /usr/include/math.h /usr/include/huge_val.h /usr/include/endian.h
liapsub.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
liapsub.o: /usr/include/ieee754.h /usr/include/ieee854.h node.h outcome.h
liapsub.o: behavsol.h
simpsub.o: rational.h integer.h /usr/include/math.h /usr/include/features.h
simpsub.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h
simpsub.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
simpsub.o: /usr/include/values.h /usr/include/ieee754.h
simpsub.o: /usr/include/ieee854.h simpsub.imp simpsub.h simpdiv.h nfg.h
simpsub.o: garray.h /usr/include/stdlib.h /usr/include/errno.h
simpsub.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
simpsub.o: /usr/include/alloca.h /usr/include/assert.h gambitio.h
simpsub.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
simpsub.o: gmisc.h gstring.h /usr/include/string.h gpvector.h gvector.h
simpsub.o: glist.h grarray.h gstatus.h gsignal.h gprogres.h mixed.h nfstrat.h
simpsub.o: gblock.h mixedsol.h subsolve.h efg.h behav.h efstrat.h gdpvect.h
simpsub.o: efplayer.h infoset.h node.h outcome.h behavsol.h
enumsub.o: rational.h integer.h /usr/include/math.h /usr/include/features.h
enumsub.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h
enumsub.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
enumsub.o: /usr/include/values.h /usr/include/ieee754.h
enumsub.o: /usr/include/ieee854.h enumsub.imp enumsub.h subsolve.h efg.h
enumsub.o: gstring.h /usr/include/string.h gblock.h /usr/include/stdlib.h
enumsub.o: /usr/include/errno.h /usr/include/linux/errno.h
enumsub.o: /usr/include/asm/errno.h /usr/include/alloca.h
enumsub.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
enumsub.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h glist.h
enumsub.o: gpvector.h gvector.h behav.h efstrat.h gdpvect.h efplayer.h
enumsub.o: infoset.h node.h outcome.h nfg.h behavsol.h enum.h gstatus.h
enumsub.o: gsignal.h gprogres.h lhtab.h lemketab.h tableau.h ludecomp.h
enumsub.o: gmatrix.h grarray.h gsmatrix.h bfs.h gmap.h mixed.h nfstrat.h
enumsub.o: mixedsol.h vertenum.h
csumsub.o: rational.h integer.h /usr/include/math.h /usr/include/features.h
csumsub.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h
csumsub.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
csumsub.o: /usr/include/values.h /usr/include/ieee754.h
csumsub.o: /usr/include/ieee854.h csumsub.imp csumsub.h subsolve.h efg.h
csumsub.o: gstring.h /usr/include/string.h gblock.h /usr/include/stdlib.h
csumsub.o: /usr/include/errno.h /usr/include/linux/errno.h
csumsub.o: /usr/include/asm/errno.h /usr/include/alloca.h
csumsub.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
csumsub.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h glist.h
csumsub.o: gpvector.h gvector.h behav.h efstrat.h gdpvect.h efplayer.h
csumsub.o: infoset.h node.h outcome.h nfg.h behavsol.h nfgcsum.h gstatus.h
csumsub.o: gsignal.h gprogres.h tableau.h ludecomp.h gmatrix.h grarray.h
csumsub.o: gsmatrix.h bfs.h gmap.h lpsolve.h mixed.h nfstrat.h mixedsol.h
behavsol.o: behavsol.h gmisc.h behav.h gstring.h /usr/include/string.h
behavsol.o: /usr/include/features.h /usr/include/sys/cdefs.h efstrat.h
behavsol.o: gblock.h /usr/include/stdlib.h /usr/include/errno.h
behavsol.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
behavsol.o: /usr/include/alloca.h /usr/include/assert.h garray.h gambitio.h
behavsol.o: /usr/include/stdio.h /usr/include/libio.h
behavsol.o: /usr/include/_G_config.h efg.h glist.h gpvector.h gvector.h
behavsol.o: efplayer.h infoset.h rational.h integer.h /usr/include/math.h
behavsol.o: /usr/include/huge_val.h /usr/include/endian.h
behavsol.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
behavsol.o: /usr/include/ieee754.h /usr/include/ieee854.h node.h outcome.h
behavsol.o: gdpvect.h
efgconv.o: efg.h gstring.h /usr/include/string.h /usr/include/features.h
efgconv.o: /usr/include/sys/cdefs.h gblock.h /usr/include/stdlib.h
efgconv.o: /usr/include/errno.h /usr/include/linux/errno.h
efgconv.o: /usr/include/asm/errno.h /usr/include/alloca.h
efgconv.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
efgconv.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h glist.h
efgconv.o: gpvector.h gvector.h behav.h efstrat.h gdpvect.h efplayer.h
efgconv.o: infoset.h rational.h integer.h /usr/include/math.h
efgconv.o: /usr/include/huge_val.h /usr/include/endian.h
efgconv.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
efgconv.o: /usr/include/ieee754.h /usr/include/ieee854.h node.h outcome.h
lemke.o: lemke.imp gwatch.h gpvector.h gvector.h gmisc.h gambitio.h
lemke.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
lemke.o: garray.h /usr/include/stdlib.h /usr/include/features.h
lemke.o: /usr/include/sys/cdefs.h /usr/include/errno.h
lemke.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
lemke.o: /usr/include/alloca.h /usr/include/assert.h nfg.h gstring.h
lemke.o: /usr/include/string.h lemke.h mixedsol.h mixed.h nfstrat.h gblock.h
lemke.o: glist.h lhtab.h lemketab.h tableau.h rational.h integer.h
lemke.o: /usr/include/math.h /usr/include/huge_val.h /usr/include/endian.h
lemke.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
lemke.o: /usr/include/ieee754.h /usr/include/ieee854.h ludecomp.h gmatrix.h
lemke.o: grarray.h gsmatrix.h bfs.h gmap.h gstatus.h gsignal.h gprogres.h
nliap.o: nliap.h gambitio.h /usr/include/stdio.h /usr/include/libio.h
nliap.o: /usr/include/_G_config.h gmisc.h gstatus.h gsignal.h gprogres.h
nliap.o: gstring.h /usr/include/string.h /usr/include/features.h
nliap.o: /usr/include/sys/cdefs.h glist.h nfg.h garray.h
nliap.o: /usr/include/stdlib.h /usr/include/errno.h
nliap.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
nliap.o: /usr/include/alloca.h /usr/include/assert.h gpvector.h gvector.h
nliap.o: mixed.h nfstrat.h gblock.h mixedsol.h gfunc.h
ngobit.o: /usr/include/math.h /usr/include/features.h
ngobit.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h
ngobit.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
ngobit.o: /usr/include/values.h /usr/include/ieee754.h /usr/include/ieee854.h
ngobit.o: ngobit.h gambitio.h /usr/include/stdio.h /usr/include/libio.h
ngobit.o: /usr/include/_G_config.h gmisc.h gstatus.h gsignal.h gprogres.h
ngobit.o: gstring.h /usr/include/string.h glist.h nfg.h garray.h
ngobit.o: /usr/include/stdlib.h /usr/include/errno.h
ngobit.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
ngobit.o: /usr/include/alloca.h /usr/include/assert.h gpvector.h gvector.h
ngobit.o: mixed.h nfstrat.h gblock.h mixedsol.h gfunc.h
enum.o: enum.imp gwatch.h nfg.h garray.h /usr/include/stdlib.h
enum.o: /usr/include/features.h /usr/include/sys/cdefs.h /usr/include/errno.h
enum.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
enum.o: /usr/include/alloca.h /usr/include/assert.h gambitio.h
enum.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
enum.o: gmisc.h gstring.h /usr/include/string.h gpvector.h gvector.h
enum.o: nfgiter.h nfstrat.h gblock.h enum.h glist.h gstatus.h gsignal.h
enum.o: gprogres.h lhtab.h lemketab.h tableau.h rational.h integer.h
enum.o: /usr/include/math.h /usr/include/huge_val.h /usr/include/endian.h
enum.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
enum.o: /usr/include/ieee754.h /usr/include/ieee854.h ludecomp.h gmatrix.h
enum.o: grarray.h gsmatrix.h bfs.h gmap.h mixed.h mixedsol.h vertenum.h
simpdiv.o: simpdiv.imp gambitio.h /usr/include/stdio.h /usr/include/libio.h
simpdiv.o: /usr/include/_G_config.h gmisc.h nfg.h garray.h
simpdiv.o: /usr/include/stdlib.h /usr/include/features.h
simpdiv.o: /usr/include/sys/cdefs.h /usr/include/errno.h
simpdiv.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
simpdiv.o: /usr/include/alloca.h /usr/include/assert.h gstring.h
simpdiv.o: /usr/include/string.h gpvector.h gvector.h grarray.h gwatch.h
simpdiv.o: simpdiv.h glist.h gstatus.h gsignal.h gprogres.h mixed.h nfstrat.h
simpdiv.o: gblock.h mixedsol.h rational.h integer.h /usr/include/math.h
simpdiv.o: /usr/include/huge_val.h /usr/include/endian.h
simpdiv.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
simpdiv.o: /usr/include/ieee754.h /usr/include/ieee854.h
tableau.o: rational.h integer.h /usr/include/math.h /usr/include/features.h
tableau.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h
tableau.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
tableau.o: /usr/include/values.h /usr/include/ieee754.h
tableau.o: /usr/include/ieee854.h tableau.imp tableau.h ludecomp.h gmatrix.h
tableau.o: gambitio.h /usr/include/stdio.h /usr/include/libio.h
tableau.o: /usr/include/_G_config.h gmisc.h grarray.h gblock.h
tableau.o: /usr/include/stdlib.h /usr/include/errno.h
tableau.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
tableau.o: /usr/include/alloca.h /usr/include/assert.h garray.h gvector.h
tableau.o: gsmatrix.h glist.h bfs.h gmap.h
ludecomp.o: ludecomp.imp ludecomp.h gmatrix.h gambitio.h /usr/include/stdio.h
ludecomp.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h grarray.h
ludecomp.o: gblock.h /usr/include/stdlib.h /usr/include/features.h
ludecomp.o: /usr/include/sys/cdefs.h /usr/include/errno.h
ludecomp.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
ludecomp.o: /usr/include/alloca.h /usr/include/assert.h garray.h gvector.h
ludecomp.o: gsmatrix.h glist.h glist.imp rational.h integer.h
ludecomp.o: /usr/include/math.h /usr/include/huge_val.h /usr/include/endian.h
ludecomp.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
ludecomp.o: /usr/include/ieee754.h /usr/include/ieee854.h
nfgpure.o: nfgpure.imp nfgpure.h nfg.h garray.h /usr/include/stdlib.h
nfgpure.o: /usr/include/features.h /usr/include/sys/cdefs.h
nfgpure.o: /usr/include/errno.h /usr/include/linux/errno.h
nfgpure.o: /usr/include/asm/errno.h /usr/include/alloca.h
nfgpure.o: /usr/include/assert.h gambitio.h /usr/include/stdio.h
nfgpure.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h gstring.h
nfgpure.o: /usr/include/string.h gpvector.h gvector.h mixed.h nfstrat.h
nfgpure.o: gblock.h mixedsol.h glist.h nfgiter.h nfgciter.h rational.h
nfgpure.o: integer.h /usr/include/math.h /usr/include/huge_val.h
nfgpure.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
nfgpure.o: /usr/include/values.h /usr/include/ieee754.h
nfgpure.o: /usr/include/ieee854.h
lhtab.o: lhtab.h lemketab.h tableau.h rational.h integer.h
lhtab.o: /usr/include/math.h /usr/include/features.h /usr/include/sys/cdefs.h
lhtab.o: /usr/include/huge_val.h /usr/include/endian.h /usr/include/bytesex.h
lhtab.o: /usr/include/nan.h /usr/include/values.h /usr/include/ieee754.h
lhtab.o: /usr/include/ieee854.h ludecomp.h gmatrix.h gambitio.h
lhtab.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
lhtab.o: gmisc.h grarray.h gblock.h /usr/include/stdlib.h
lhtab.o: /usr/include/errno.h /usr/include/linux/errno.h
lhtab.o: /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/assert.h
lhtab.o: garray.h gvector.h gsmatrix.h glist.h bfs.h gmap.h nfg.h gstring.h
lhtab.o: /usr/include/string.h gpvector.h nfgiter.h nfstrat.h
lemketab.o: lemketab.h tableau.h rational.h integer.h /usr/include/math.h
lemketab.o: /usr/include/features.h /usr/include/sys/cdefs.h
lemketab.o: /usr/include/huge_val.h /usr/include/endian.h
lemketab.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
lemketab.o: /usr/include/ieee754.h /usr/include/ieee854.h ludecomp.h
lemketab.o: gmatrix.h gambitio.h /usr/include/stdio.h /usr/include/libio.h
lemketab.o: /usr/include/_G_config.h gmisc.h grarray.h gblock.h
lemketab.o: /usr/include/stdlib.h /usr/include/errno.h
lemketab.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
lemketab.o: /usr/include/alloca.h /usr/include/assert.h garray.h gvector.h
lemketab.o: gsmatrix.h glist.h bfs.h gmap.h
grid.o: /usr/include/math.h /usr/include/features.h /usr/include/sys/cdefs.h
grid.o: /usr/include/huge_val.h /usr/include/endian.h /usr/include/bytesex.h
grid.o: /usr/include/nan.h /usr/include/values.h /usr/include/ieee754.h
grid.o: /usr/include/ieee854.h mixed.h gmisc.h nfstrat.h gstring.h
grid.o: /usr/include/string.h gblock.h /usr/include/stdlib.h
grid.o: /usr/include/errno.h /usr/include/linux/errno.h
grid.o: /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/assert.h
grid.o: garray.h gambitio.h /usr/include/stdio.h /usr/include/libio.h
grid.o: /usr/include/_G_config.h gpvector.h gvector.h grid.h gstatus.h
grid.o: gsignal.h gprogres.h nfg.h gwatch.h
nfgcsum.o: nfgcsum.imp gwatch.h gpvector.h gvector.h gmisc.h gambitio.h
nfgcsum.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
nfgcsum.o: garray.h /usr/include/stdlib.h /usr/include/features.h
nfgcsum.o: /usr/include/sys/cdefs.h /usr/include/errno.h
nfgcsum.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
nfgcsum.o: /usr/include/alloca.h /usr/include/assert.h nfg.h gstring.h
nfgcsum.o: /usr/include/string.h nfgiter.h nfstrat.h gblock.h nfgcsum.h
nfgcsum.o: rational.h integer.h /usr/include/math.h /usr/include/huge_val.h
nfgcsum.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
nfgcsum.o: /usr/include/values.h /usr/include/ieee754.h
nfgcsum.o: /usr/include/ieee854.h glist.h gstatus.h gsignal.h gprogres.h
nfgcsum.o: tableau.h ludecomp.h gmatrix.h grarray.h gsmatrix.h bfs.h gmap.h
nfgcsum.o: lpsolve.h mixed.h mixedsol.h
lpsolve.o: lpsolve.imp lpsolve.h tableau.h rational.h integer.h
lpsolve.o: /usr/include/math.h /usr/include/features.h
lpsolve.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h
lpsolve.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
lpsolve.o: /usr/include/values.h /usr/include/ieee754.h
lpsolve.o: /usr/include/ieee854.h ludecomp.h gmatrix.h gambitio.h
lpsolve.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
lpsolve.o: gmisc.h grarray.h gblock.h /usr/include/stdlib.h
lpsolve.o: /usr/include/errno.h /usr/include/linux/errno.h
lpsolve.o: /usr/include/asm/errno.h /usr/include/alloca.h
lpsolve.o: /usr/include/assert.h garray.h gvector.h gsmatrix.h glist.h bfs.h
lpsolve.o: gmap.h gstatus.h gsignal.h gprogres.h gstring.h
lpsolve.o: /usr/include/string.h
nfdom.o: nfg.h garray.h /usr/include/stdlib.h /usr/include/features.h
nfdom.o: /usr/include/sys/cdefs.h /usr/include/errno.h
nfdom.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
nfdom.o: /usr/include/alloca.h /usr/include/assert.h gambitio.h
nfdom.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
nfdom.o: gmisc.h gstring.h /usr/include/string.h gpvector.h gvector.h
nfdom.o: nfstrat.h gblock.h nfgciter.h rational.h integer.h
nfdom.o: /usr/include/math.h /usr/include/huge_val.h /usr/include/endian.h
nfdom.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
nfdom.o: /usr/include/ieee754.h /usr/include/ieee854.h gstatus.h gsignal.h
nfdom.o: gprogres.h
vertenum.o: vertenum.imp vertenum.h tableau.h rational.h integer.h
vertenum.o: /usr/include/math.h /usr/include/features.h
vertenum.o: /usr/include/sys/cdefs.h /usr/include/huge_val.h
vertenum.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
vertenum.o: /usr/include/values.h /usr/include/ieee754.h
vertenum.o: /usr/include/ieee854.h ludecomp.h gmatrix.h gambitio.h
vertenum.o: /usr/include/stdio.h /usr/include/libio.h
vertenum.o: /usr/include/_G_config.h gmisc.h grarray.h gblock.h
vertenum.o: /usr/include/stdlib.h /usr/include/errno.h
vertenum.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
vertenum.o: /usr/include/alloca.h /usr/include/assert.h garray.h gvector.h
vertenum.o: gsmatrix.h glist.h bfs.h gmap.h gstatus.h gsignal.h gprogres.h
vertenum.o: gstring.h /usr/include/string.h
mixedsol.o: mixedsol.h gmisc.h mixed.h nfstrat.h gstring.h
mixedsol.o: /usr/include/string.h /usr/include/features.h
mixedsol.o: /usr/include/sys/cdefs.h gblock.h /usr/include/stdlib.h
mixedsol.o: /usr/include/errno.h /usr/include/linux/errno.h
mixedsol.o: /usr/include/asm/errno.h /usr/include/alloca.h
mixedsol.o: /usr/include/assert.h garray.h gambitio.h /usr/include/stdio.h
mixedsol.o: /usr/include/libio.h /usr/include/_G_config.h gpvector.h
mixedsol.o: gvector.h
nfdommix.o: gambitio.h /usr/include/stdio.h /usr/include/libio.h
nfdommix.o: /usr/include/_G_config.h gmisc.h nfg.h garray.h
nfdommix.o: /usr/include/stdlib.h /usr/include/features.h
nfdommix.o: /usr/include/sys/cdefs.h /usr/include/errno.h
nfdommix.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
nfdommix.o: /usr/include/alloca.h /usr/include/assert.h gstring.h
nfdommix.o: /usr/include/string.h gpvector.h gvector.h nfgiter.h nfstrat.h
nfdommix.o: gblock.h nfgciter.h lpsolve.h tableau.h rational.h integer.h
nfdommix.o: /usr/include/math.h /usr/include/huge_val.h /usr/include/endian.h
nfdommix.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
nfdommix.o: /usr/include/ieee754.h /usr/include/ieee854.h ludecomp.h
nfdommix.o: gmatrix.h grarray.h gsmatrix.h glist.h bfs.h gmap.h gstatus.h
nfdommix.o: gsignal.h gprogres.h
nfgconv.o: nfg.h garray.h /usr/include/stdlib.h /usr/include/features.h
nfgconv.o: /usr/include/sys/cdefs.h /usr/include/errno.h
nfgconv.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
nfgconv.o: /usr/include/alloca.h /usr/include/assert.h gambitio.h
nfgconv.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
nfgconv.o: gmisc.h gstring.h /usr/include/string.h gpvector.h gvector.h
nfgconv.o: nfplayer.h nfstrat.h gblock.h nfgciter.h rational.h integer.h
nfgconv.o: /usr/include/math.h /usr/include/huge_val.h /usr/include/endian.h
nfgconv.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
nfgconv.o: /usr/include/ieee754.h /usr/include/ieee854.h
gsmutils.o: portion.h gsmincl.h gstring.h /usr/include/string.h
gsmutils.o: /usr/include/features.h /usr/include/sys/cdefs.h gmisc.h glist.h
gsmutils.o: gambitio.h /usr/include/stdio.h /usr/include/libio.h
gsmutils.o: /usr/include/_G_config.h rational.h integer.h /usr/include/math.h
gsmutils.o: /usr/include/huge_val.h /usr/include/endian.h
gsmutils.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
gsmutils.o: /usr/include/ieee754.h /usr/include/ieee854.h gvector.h garray.h
gsmutils.o: /usr/include/stdlib.h /usr/include/errno.h
gsmutils.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
gsmutils.o: /usr/include/alloca.h /usr/include/assert.h gdpvect.h gpvector.h
gsmutils.o: gmatrix.h grarray.h gblock.h gsmatrix.h
gsm.o: gstack.imp gstack.h /usr/include/assert.h /usr/include/features.h
gsm.o: /usr/include/sys/cdefs.h garray.imp /usr/include/stdlib.h
gsm.o: /usr/include/errno.h /usr/include/linux/errno.h
gsm.o: /usr/include/asm/errno.h /usr/include/alloca.h garray.h gambitio.h
gsm.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
gsm.o: gmisc.h gslist.imp gslist.h glist.h gstring.h /usr/include/string.h
gsm.o: gsm.h gsmincl.h portion.h rational.h integer.h /usr/include/math.h
gsm.o: /usr/include/huge_val.h /usr/include/endian.h /usr/include/bytesex.h
gsm.o: /usr/include/nan.h /usr/include/values.h /usr/include/ieee754.h
gsm.o: /usr/include/ieee854.h gsmhash.h hash.h gsmfunc.h gsminstr.h gblock.h
gsm.o: nfg.h gpvector.h gvector.h mixedsol.h mixed.h nfstrat.h behavsol.h
gsm.o: behav.h efstrat.h efg.h efplayer.h infoset.h node.h outcome.h
gsm.o: gdpvect.h
gsmfunc.o: /usr/include/assert.h /usr/include/features.h
gsmfunc.o: /usr/include/sys/cdefs.h gsmfunc.h gsmincl.h gstring.h
gsmfunc.o: /usr/include/string.h gmisc.h glist.h gambitio.h
gsmfunc.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
gsmfunc.o: gstack.h gsm.h portion.h rational.h integer.h /usr/include/math.h
gsmfunc.o: /usr/include/huge_val.h /usr/include/endian.h
gsmfunc.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
gsmfunc.o: /usr/include/ieee754.h /usr/include/ieee854.h gsmhash.h hash.h
gsmfunc.o: gsminstr.h nfg.h garray.h /usr/include/stdlib.h
gsmfunc.o: /usr/include/errno.h /usr/include/linux/errno.h
gsmfunc.o: /usr/include/asm/errno.h /usr/include/alloca.h gpvector.h
gsmfunc.o: gvector.h efg.h gblock.h behav.h efstrat.h gdpvect.h efplayer.h
gsmfunc.o: infoset.h node.h outcome.h nfstrat.h
gsmoper.o: /usr/include/stdlib.h /usr/include/features.h
gsmoper.o: /usr/include/sys/cdefs.h /usr/include/errno.h
gsmoper.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
gsmoper.o: /usr/include/alloca.h /usr/include/ctype.h /usr/include/endian.h
gsmoper.o: /usr/include/bytesex.h gmisc.h gsm.h gambitio.h
gsmoper.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
gsmoper.o: gstring.h /usr/include/string.h gsmincl.h portion.h glist.h
gsmoper.o: rational.h integer.h /usr/include/math.h /usr/include/huge_val.h
gsmoper.o: /usr/include/nan.h /usr/include/values.h /usr/include/ieee754.h
gsmoper.o: /usr/include/ieee854.h gsmfunc.h gblock.h /usr/include/assert.h
gsmoper.o: garray.h mixedsol.h mixed.h nfstrat.h gpvector.h gvector.h
gsmoper.o: behavsol.h behav.h efstrat.h efg.h efplayer.h infoset.h node.h
gsmoper.o: outcome.h gdpvect.h nfg.h system.h gstack.h
gsmoper.o: /usr/include/sys/types.h /usr/include/linux/types.h
gsmoper.o: /usr/include/linux/posix_types.h /usr/include/asm/posix_types.h
gsmoper.o: /usr/include/asm/types.h /usr/include/sys/bitypes.h
gsmoper.o: /usr/include/sys/time.h /usr/include/linux/time.h
gsmoper.o: /usr/include/sys/time.h
gsmhash.o: gstring.h /usr/include/string.h /usr/include/features.h
gsmhash.o: /usr/include/sys/cdefs.h portion.h gsmincl.h gmisc.h glist.h
gsmhash.o: gambitio.h /usr/include/stdio.h /usr/include/libio.h
gsmhash.o: /usr/include/_G_config.h rational.h integer.h /usr/include/math.h
gsmhash.o: /usr/include/huge_val.h /usr/include/endian.h
gsmhash.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
gsmhash.o: /usr/include/ieee754.h /usr/include/ieee854.h gsmfunc.h glist.imp
gsmhash.o: /usr/include/assert.h hash.imp hash.h gsmhash.h
gsminstr.o: gsminstr.h gstring.h /usr/include/string.h
gsminstr.o: /usr/include/features.h /usr/include/sys/cdefs.h gmisc.h
gsminstr.o: gambitio.h /usr/include/stdio.h /usr/include/libio.h
gsminstr.o: /usr/include/_G_config.h glist.imp glist.h /usr/include/assert.h
portion.o: /usr/include/assert.h /usr/include/features.h
portion.o: /usr/include/sys/cdefs.h /usr/include/string.h garray.imp
portion.o: /usr/include/stdlib.h /usr/include/errno.h
portion.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
portion.o: /usr/include/alloca.h garray.h gambitio.h /usr/include/stdio.h
portion.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h gblock.imp
portion.o: gblock.h portion.h gsmincl.h gstring.h glist.h rational.h
portion.o: integer.h /usr/include/math.h /usr/include/huge_val.h
portion.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
portion.o: /usr/include/values.h /usr/include/ieee754.h
portion.o: /usr/include/ieee854.h gsmhash.h hash.h nfg.h gpvector.h gvector.h
portion.o: efg.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h
portion.o: outcome.h nfplayer.h nfstrat.h mixedsol.h mixed.h behavsol.h gsm.h
nfgfunc.o: gsm.h gambitio.h /usr/include/stdio.h /usr/include/libio.h
nfgfunc.o: /usr/include/_G_config.h gmisc.h gstring.h /usr/include/string.h
nfgfunc.o: /usr/include/features.h /usr/include/sys/cdefs.h gsmincl.h
nfgfunc.o: portion.h glist.h rational.h integer.h /usr/include/math.h
nfgfunc.o: /usr/include/huge_val.h /usr/include/endian.h
nfgfunc.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
nfgfunc.o: /usr/include/ieee754.h /usr/include/ieee854.h gsmfunc.h nfg.h
nfgfunc.o: garray.h /usr/include/stdlib.h /usr/include/errno.h
nfgfunc.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
nfgfunc.o: /usr/include/alloca.h /usr/include/assert.h gpvector.h gvector.h
nfgfunc.o: nfplayer.h mixed.h nfstrat.h gblock.h gwatch.h gstatus.h gsignal.h
nfgfunc.o: gprogres.h
efgfunc.o: gsm.h gambitio.h /usr/include/stdio.h /usr/include/libio.h
efgfunc.o: /usr/include/_G_config.h gmisc.h gstring.h /usr/include/string.h
efgfunc.o: /usr/include/features.h /usr/include/sys/cdefs.h gsmincl.h
efgfunc.o: portion.h glist.h rational.h integer.h /usr/include/math.h
efgfunc.o: /usr/include/huge_val.h /usr/include/endian.h
efgfunc.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
efgfunc.o: /usr/include/ieee754.h /usr/include/ieee854.h gsmfunc.h gwatch.h
efgfunc.o: gstatus.h gsignal.h gprogres.h efg.h gblock.h
efgfunc.o: /usr/include/stdlib.h /usr/include/errno.h
efgfunc.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
efgfunc.o: /usr/include/alloca.h /usr/include/assert.h garray.h gpvector.h
efgfunc.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h
efgfunc.o: outcome.h efgutils.h
listfunc.o: /usr/include/assert.h /usr/include/features.h
listfunc.o: /usr/include/sys/cdefs.h gsm.h gambitio.h /usr/include/stdio.h
listfunc.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h gstring.h
listfunc.o: /usr/include/string.h gsmincl.h portion.h glist.h rational.h
listfunc.o: integer.h /usr/include/math.h /usr/include/huge_val.h
listfunc.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
listfunc.o: /usr/include/values.h /usr/include/ieee754.h
listfunc.o: /usr/include/ieee854.h gsmfunc.h nfg.h garray.h
listfunc.o: /usr/include/stdlib.h /usr/include/errno.h
listfunc.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
listfunc.o: /usr/include/alloca.h gpvector.h gvector.h efg.h gblock.h behav.h
listfunc.o: efstrat.h gdpvect.h efplayer.h infoset.h node.h outcome.h
listfunc.o: gwatch.h
algfunc.o: gsm.h gambitio.h /usr/include/stdio.h /usr/include/libio.h
algfunc.o: /usr/include/_G_config.h gmisc.h gstring.h /usr/include/string.h
algfunc.o: /usr/include/features.h /usr/include/sys/cdefs.h gsmincl.h
algfunc.o: portion.h glist.h rational.h integer.h /usr/include/math.h
algfunc.o: /usr/include/huge_val.h /usr/include/endian.h
algfunc.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
algfunc.o: /usr/include/ieee754.h /usr/include/ieee854.h gsmfunc.h gwatch.h
algfunc.o: mixedsol.h mixed.h nfstrat.h gblock.h /usr/include/stdlib.h
algfunc.o: /usr/include/errno.h /usr/include/linux/errno.h
algfunc.o: /usr/include/asm/errno.h /usr/include/alloca.h
algfunc.o: /usr/include/assert.h garray.h gpvector.h gvector.h behavsol.h
algfunc.o: behav.h efstrat.h efg.h efplayer.h infoset.h node.h outcome.h
algfunc.o: gdpvect.h nfg.h nfplayer.h enum.h gstatus.h gsignal.h gprogres.h
algfunc.o: lhtab.h lemketab.h tableau.h ludecomp.h gmatrix.h grarray.h
algfunc.o: gsmatrix.h bfs.h gmap.h vertenum.h enumsub.h subsolve.h nfgpure.h
algfunc.o: efgpure.h psnesub.h grid.h ngobit.h egobit.h lemke.h seqform.h
algfunc.o: lemkesub.h liapsub.h nliap.h eliap.h nfgcsum.h lpsolve.h csumsub.h
algfunc.o: efgcsum.h simpdiv.h simpsub.h
gcompile.o: /usr/include/stdio.h /usr/include/libio.h
gcompile.o: /usr/include/_G_config.h /usr/include/stdlib.h
gcompile.o: /usr/include/features.h /usr/include/sys/cdefs.h
gcompile.o: /usr/include/errno.h /usr/include/linux/errno.h
gcompile.o: /usr/include/asm/errno.h /usr/include/alloca.h
gcompile.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
gcompile.o: gmisc.h gambitio.h gstring.h /usr/include/string.h rational.h
gcompile.o: integer.h /usr/include/math.h /usr/include/huge_val.h
gcompile.o: /usr/include/nan.h /usr/include/values.h /usr/include/ieee754.h
gcompile.o: /usr/include/ieee854.h glist.h gstack.h gsm.h gsmincl.h
gcompile.o: gsminstr.h gsmfunc.h portion.h system.h gstack.imp
gcompile.o: /usr/include/assert.h glist.imp
gcl.o: /usr/include/signal.h /usr/include/features.h /usr/include/sys/cdefs.h
gcl.o: /usr/include/sys/types.h /usr/include/linux/types.h
gcl.o: /usr/include/linux/posix_types.h /usr/include/asm/posix_types.h
gcl.o: /usr/include/asm/types.h /usr/include/sys/bitypes.h
gcl.o: /usr/include/linux/signal.h /usr/include/asm/signal.h
gcl.o: /usr/include/values.h /usr/include/math.h /usr/include/huge_val.h
gcl.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
gcl.o: /usr/include/ieee754.h /usr/include/ieee854.h /usr/include/assert.h
gcl.o: rational.h integer.h gstring.h /usr/include/string.h glist.h
gcl.o: gambitio.h /usr/include/stdio.h /usr/include/libio.h
gcl.o: /usr/include/_G_config.h gmisc.h gsm.h gsmincl.h gstack.h gcompile.h
gclsig.o: gsignal.h gmisc.h /usr/include/signal.h /usr/include/features.h
gclsig.o: /usr/include/sys/cdefs.h /usr/include/sys/types.h
gclsig.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
gclsig.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
gclsig.o: /usr/include/sys/bitypes.h /usr/include/linux/signal.h
gclsig.o: /usr/include/asm/signal.h
solfunc.o: gsm.h gambitio.h /usr/include/stdio.h /usr/include/libio.h
solfunc.o: /usr/include/_G_config.h gmisc.h gstring.h /usr/include/string.h
solfunc.o: /usr/include/features.h /usr/include/sys/cdefs.h gsmincl.h
solfunc.o: portion.h glist.h rational.h integer.h /usr/include/math.h
solfunc.o: /usr/include/huge_val.h /usr/include/endian.h
solfunc.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
solfunc.o: /usr/include/ieee754.h /usr/include/ieee854.h gsmfunc.h efg.h
solfunc.o: gblock.h /usr/include/stdlib.h /usr/include/errno.h
solfunc.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
solfunc.o: /usr/include/alloca.h /usr/include/assert.h garray.h gpvector.h
solfunc.o: gvector.h behav.h efstrat.h gdpvect.h efplayer.h infoset.h node.h
solfunc.o: outcome.h nfg.h behavsol.h mixedsol.h mixed.h nfstrat.h nfplayer.h
solfunc.o: efgutils.h
gsmincl.o: gsmincl.h gstring.h /usr/include/string.h /usr/include/features.h
gsmincl.o: /usr/include/sys/cdefs.h gmisc.h gambitio.h /usr/include/stdio.h
gsmincl.o: /usr/include/libio.h /usr/include/_G_config.h
system.o: /usr/include/stdlib.h /usr/include/features.h
system.o: /usr/include/sys/cdefs.h /usr/include/errno.h
system.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
system.o: /usr/include/alloca.h /usr/include/assert.h /usr/include/string.h
system.o: gambitio.h /usr/include/stdio.h /usr/include/libio.h
system.o: /usr/include/_G_config.h gmisc.h system.h
gcmdline.o: /usr/include/assert.h /usr/include/features.h
gcmdline.o: /usr/include/sys/cdefs.h /usr/include/ctype.h
gcmdline.o: /usr/include/endian.h /usr/include/bytesex.h gcmdline.h
gcmdline.o: /usr/include/stdlib.h /usr/include/errno.h
gcmdline.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
gcmdline.o: /usr/include/alloca.h gambitio.h /usr/include/stdio.h
gcmdline.o: /usr/include/libio.h /usr/include/_G_config.h gmisc.h gstring.h
gcmdline.o: /usr/include/string.h glist.h gstack.h gstack.imp
