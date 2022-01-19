### General information

Gambit uses the standard autotools mechanism for configuring and building.
This should be familiar to most users of Un\*ces and MacOS X.  In general,
you just need to unpack the sources, change directory to the top level
of the sources (typically of the form gambit-X.Y.Z), and do the
usual

    ./configure
    make
    sudo make install

Command-line options are available to modify the configuration process;
do `./configure --help` for information.  

By default Gambit will be installed in /usr/local.  You can change this
by replacing configure step with one of the form

    ./configure --prefix=/your/path/here

NOTE: The graphical interface relies on external calls to other
programs built in this process, especially for the computation of
equilibria.  It is strongly recommended that you install the Gambit
executables to a directory in your path!


### Building from git repository

If you want to live on the bleeding edge, you can get the latest
version of the Gambit sources from the Gambit repository on
github.com, via:

    git clone https://github.com/gambitproject/gambit.git
    cd gambit

After this, you will need to set up the build scripts by executing:

    aclocal
    automake --add-missing
    autoconf

For this, you will need to have automake, autoconf, and libtool2
installed on your system.

At this point, you can then continue with the configuration and build
stages as in the previous section.

### gambit-enumpoly

The program gambit-enumpoly does not compile on some compilers and systems.
If you encounter problems building this, you can disable compilation using
the switch `--disable-enumpoly` at the configuration step, e.g.

    ./configure --disable-enumpoly [other options here]

There is a related issue in the issue tracker (#288); if you encounter problems
building `gambit-enumpoly` (or want to contribute towards fixing it!) please
use that issue to post information.


### For Windows users

For Windows users wanting to compile Gambit on their own, you'll need
to use either the Cygwin or MinGW environments.  We do compilation and
testing of Gambit on Windows using MinGW, which can be gotten from

http://www.mingw.org


### For OS X users

OS X users should being by following the Un\*x/Linux instructions above.
This will create the command-line tools, and the graphical interface
binary called `gambit`.  This graphical interface binary requires an
X server to run correctly.

For a more native OS X experience, after completing the Un*x/Linux
instructions, additionally issue the command

    make osx-bundle

This will create a directory Gambit.app with the graphical interface
in an application bundle.  This bundle can then be copied (e.g., to
`/Applications`) and used like any other OS X application.

wxWidgets is available, for example, via Homebrew on Mac OS X.
If you build wxWidgets yourself (see below),
be sure to tell the `./configure` step where to find the version you built
by using the `--with-wx-prefix parameter`.  For example, if you install
wxWidgets into `/usr/local` (the default when you build it), configure
Gambit with

    ./configure --with-wx-prefix=/usr/local


### The graphical interface and wxWidgets

Gambit requires wxWidgets version 3.1.5 or higher for the
graphical interface.  See their website at

http://www.wxwidgets.org

to download this if you need it.  Packages of this should be available
for most Un\*x users through their package managers (apt or rpm).  Note
that you'll need the appropriate -dev package for wxWidgets to get the
header files needed to build Gambit.

Un\*x users, please note that Gambit only supports the GTK port of wxWidgets.

If wxWidgets it isn't installed in a standard place (e.g., `/usr` or
`/usr/local`), you'll need to tell configure where to find it with the
`--with-wx-prefix=PREFIX` option, for example:

    ./configure --with-wx-prefix=/home/mylogin/wx

Finally, if you don't want to build the graphical interface, you
can either (a) simply not install wxWidgets, or (b) pass the argument
`--disable-gui` to the configure step, for example,

    ./configure --disable-gui

This will just build the command-line tools, and will not require
a wxWidgets installation.


### pygambit Python extension

Gambit is available as a Python extension module, called `pygambit`.
Python versions 3.7 through 3.10 inclusive are supported.
`pygambit` is available via PyPi and `pip`.

If you wish to compile the extension module on your own, then you can

    cd src
    python setup.py build
    python setup.py install

As in general with Python, it is strongly recommended to install `pygambit`
as part of a virtual environment rather than in the system's Python.

