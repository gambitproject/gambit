
CC = g++
CFLAGS = -Wall $(DEFINES) -fguiding-decls $(INCDIR) 
OBJECT_SUFFIX = .o

SOURCES = garray.cc gblock.cc glist.cc gmisc.cc gpool.cc \
          grarray.cc gstream.cc gtext.cc gwatch.cc

OBJECTS = $(SOURCES:.cc=.o)

INCDIR = -I.. -I../../include

# rule for creating .o (object) files from .cc files

%.o: %.cc
	$(CC) $(CFLAGS) -c $< -o $@

# rule for creating .d (dependency) files from .cc files

%.d: %.cc
	@echo updating dependencies: $@
	@$(SHELL) -ec '$(CC) -MM $(CPPFLAGS) $(INCDIR) $< \
		| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
		[ -s $@ ] || rm -f $@'

libgambit_base.a:  $(OBJECTS)
	ar ruv $@ $(OBJECTS)
	ranlib $@

include $(SOURCES:.cc=.d)

