# $Id$
# Top level of the distributable makefile.  This expects to be called
# with valid GUI and HOST parameters and does no error checking.  Calls
# the lower level makefile to allow for multiple platforms from the
# same source dir.

OBJDIR=obj_$(GUI)_$(HOST)
gcl: $(OBJDIR)
	$(MAKE) -f make.go depend_gcl HOST=$(HOST) GUI=$(GUI) DEPEND=True
	cd $(OBJDIR); \
	$(MAKE) -f ../make.go gcl_$(HOST) host=$(HOST) GUI=$(GUI)
gui: $(OBJDIR)
	$(MAKE) -f make.go depend_gui HOST=$(HOST) GUI=$(GUI) DEPEND=True
	cd $(OBJDIR); \
	$(MAKE) -f ../make.go gui HOST=$(HOST) GUI=$(GUI)
depend_gui:
	$(MAKE) -f make.go depend_gui HOST=$(HOST) GUI=$(GUI) DEPEND=True
depend_gcl:
	$(MAKE) -f make.go depend_gcl HOST=$(HOST) GUI=$(GUI) DEPEND=True
$(OBJDIR):
	mkdir $(OBJDIR)


