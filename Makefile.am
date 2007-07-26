## Process this file with automake to create Makefile.in

SUBDIRS = doc contrib src

EXTRA_DIST = gambit.spec gambit.desktop

install-osx:
	make install
	cp -r contrib/mac/Gambit.app /Applications
	cp /usr/local/bin/gambit /Applications/Gambit.app/Contents/MacOS/Gambit
