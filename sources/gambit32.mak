
.AUTODEPEND

WXDIR = D:\WXWIN
BCCDIR = C:\BC5
SRCSUFF = .cc
EXTRALIBS = # bin32\gambit
EXTRACPPFLAGS = 
EXTRALINKFLAGS = -Tpe -aa -V4.0 -c

TARGET = gambit
OBJECTS =    spreadconfig.obj\
   spreadcanvas.obj\
   spread3d.obj\
   dlnfg.obj\
   dlefg.obj\
   treedrag.obj\
   treerender.obj\
   dlsupport.obj\
   nfgsolvd.obj\
   efgsolvd.obj\
   spread.obj\
   nfgshow.obj\
   spreadim.obj\
   treedraw.obj\
   nfgsoln.obj\
   wxsignal.obj\
   wxstatus.obj\
   accels.obj\
   algdlgs.obj\
   gnullsts.obj\
   msolnsf.obj\
   bsolnsf.obj\
   efgshow.obj\
   efgsolng.obj\
   efgsoln.obj\
   nfgsolng.obj\
   twflash.obj\
   treewin.obj\
   wxio.obj\
   wxmisc.obj\
   gambit.obj\
   gambdraw.obj\
   guimisc.obj\
   system.obj\
   gpool.obj\
   gstream.obj\
   gtext.obj\
   gsmatrix.obj\
   gnumber.obj\
   gmisc.obj\
   garray.obj\
   gblock.obj\
   glist.obj\
   grarray.obj\
   grblock.obj\
   gmatrix.obj\
   gvector.obj\
   gpvector.obj\
   gdpvect.obj\
   gwatch.obj\
   gfunc.obj\
   gnulib.obj\
   integer.obj\
   rational.obj\
   sfstrat.obj\
   sfg.obj\
   efbasis.obj\
   nfginst.obj\
   efginst.obj\
   readnfg.obj\
   readefg.obj\
   nfgutils.obj\
   nfg.obj\
   nfstrat.obj\
   nfgiter.obj\
   efg.obj\
   efgnfg.obj\
   efstrat.obj\
   efgiter.obj\
   behavsol.obj\
   mixedsol.obj\
   efgutils.obj\
   gpolylst.obj\
   gpartltr.obj\
   linrcomb.obj\
   ideal.obj\
   gtree.obj\
   complex.obj\
   quiksolv.obj\
   odometer.obj\
   poly.obj\
   rectangl.obj\
   interval.obj\
   double.obj\
   prepoly.obj\
   monomial.obj\
   gpoly.obj\
   algutils.obj\
   ineqsolv.obj\
   efgalleq.obj\
   nfgensup.obj\
   efgensup.obj\
   nfgalleq.obj\
   btableau.obj\
   lptab.obj\
   polensub.obj\
   seqeq.obj\
   epolenum.obj\
   polenum.obj\
   clique.obj\
   basis.obj\
   nfgpure.obj\
   nfgcsum.obj\
   subsolve.obj\
   nfdommix.obj\
   simpdiv.obj\
   efgcsum.obj\
   efgpure.obj\
   lemkesub.obj\
   liapsub.obj\
   simpsub.obj\
   enumsub.obj\
   psnesub.obj\
   csumsub.obj\
   enum.obj\
   grid.obj\
   lemketab.obj\
   lemke.obj\
   efdom.obj\
   gfuncmin.obj\
   vertenum.obj\
   seqform.obj\
   tableau.obj\
   bfs.obj\
   egobit.obj\
   eliap.obj\
   lhtab.obj\
   lpsolve.obj\
   ludecomp.obj\
   nfdom.obj\
   ngobit.obj\
   nliap.obj

CFG = gambit32.cfg
WXLIBDIR = $(WXDIR)\lib
WXLIB = wx32
WXINC = -I$(WXDIR)\include\base -I$(WXDIR)\include\msw
WIN95FLAG = -D__WIN95__ -D__WINDOWS__

OPT = -Od

SRCSUFF = cc
OBJSUFF = obj

.$(SRCSUFF).obj:
	bcc32 $(CPPFLAGS) -P -c {$< }

.c.obj:
	bcc32 $(CPPFLAGS) -P- -c {$< }

LIBS=$(WXLIB) $(EXTRALIBS) cw32mti import32 ole2w32

LINKFLAGS= /Tpe /L$(WXLIBDIR);$(BCCDIR)\lib $(EXTRALINKFLAGS)
OPT = -Od
DEBUG_FLAGS= -v

CPPFLAGS= $(WXINC) $(EXTRACPPFLAGS) $(OPT) @$(CFG)

all: $(TARGET).exe $(EXTRATARGETS)

$(TARGET).exe:	$(OBJECTS) gambit.res
  ilink32 $(LINKFLAGS) @&&!
c0w32.obj $(OBJECTS)
$(TARGET)
nul
$(LIBS)

$(TARGET).res
!

gambit.res :      ..\winsrc\res\gambit.rc 
    brc32 -r -fo.\gambit.res /i$(BCCDIR)\include /i$(WXDIR)\include\msw /i$(WXDIR)\contrib\fafa ..\winsrc\res\gambit

clean:
        -erase *.obj
        -erase *.exe
        -erase *.res
        -erase *.map
        -erase *.rws

