## Process this file with automake to create Makefile.in

EXTRA_DIST = \
	about.xpm \
	adddecimal.xpm \
	calc.xpm \
	close.xpm \
	color.xpm \
	datasrc.xpm \
	deldecimal.xpm \
	dice.xpm \
	efg.ico \
	exit.xpm \
	font.xpm \
	gambitbig.xpm \
	gambit.bmp \
	gambit.ico \
	gambit.xpm \
	label.xpm \
	layout.xpm \
	move.xpm \
	newcol.xpm \
	newplayer.xpm \
	newrow.xpm \
	newtable.xpm \
	newtree.xpm \
	new.xpm \
	next.xpm \
	nfg.ico \
	open.xpm \
	person.xpm \
	preview.xpm \
	prev.xpm \
	print.xpm \
	profiles.xpm \
	redo.xpm \
	saveas.xpm \
	savedata.xpm \
	save.xpm \
	stop.xpm \
	table.xpm \
	tobegin.xpm \
	toend.xpm \
	tree.xpm \
	undo.xpm \
	zoomfit.xpm \
	zoomin.xpm \
	zoomout.xpm \
	zoom1.xpm \
        gambitrc.rc

SUFFIXES = .rc

%.o: %.rc
	$(target_alias)-windres @REZFLAGS@ $< $@

noinst_DATA = @RC_OBJECT@


