For developers: Building Gambit from source
===========================================

This section covers instructions for building Gambit from source.
This is for those who are interested in developing Gambit, or who
want to play around with the latest features before they make it
into a pre-compiled binary version.  

This section requires at least some familiarity with programming.
Most users will want to stick with binary distributions; see
:ref:`section-downloading` for how to get the current version for
your operating system.

General information
-------------------

Gambit uses the standard autotools mechanism for configuring and building.
This should be familiar to most users of Un*ces and MacOS X.  

If you are building from a source tarball, 
you just need to unpack the sources, change directory to the top level
of the sources (typically of the form gambit-0.yyyy.mm.dd), and do the
usual ::

  ./configure
  make
  sudo make install

Command-line options are available to modify the configuration process;
do `./configure --help` for information.  Of these, the option which
may be most useful is to disable the build of the graphical interface

By default Gambit will be installed in /usr/local.  You can change this
by replacing configure step with one of the form ::

  ./configure --prefix=/your/path/here

.. note::
  The graphical interface relies on external calls to other
  programs built in this process, especially for the computation of
  equilibria.  It is strongly recommended that you install the Gambit
  executables to a directory in your path!


Building from git repository
----------------------------

If you want to live on the bleeding edge, you can get the latest
version of the Gambit sources from the Gambit repository on
github.com, via ::

  git clone git://github.com/gambitproject/gambit.git
  cd gambit

After this, you will need to set up the build scripts by executing ::

  aclocal
  libtoolize
  automake --add-missing
  autoconf

For this, you will need to have automake, autoconf, and libtool2
installed on your system.

At this point, you can then continue with the configuration and build
stages as in the previous section.

In the git repository, the branch ``master`` always points to the
latest development version.  New development should in general always
be based off this branch.  Branches labeled ``maintVV``, where ``VV``
is the version number, point to the latest commit on a stable
version; so, for example, ``maint13`` refers to the latest commit for
Gambit version 13.x.x.  Bug fixes should typically be based off of
this branch.


Supported compilers
-------------------

Currently, gcc is the only compiler supported.  The version of gcc needs
to be new enough to handle templates correctly.  The oldest versions
of gcc known to compile Gambit are 3.4.6 (Linux, Ubuntu) and 3.4.2 (MinGW for Windows, Debian stable).

If you wish to use another compiler, the most likely stumbling block is
that Gambit uses templated member functions for classes, so the compiler
must support these.  (Version of gcc prior to 3.4 do not, for example.)



For Windows users
-----------------

For Windows users wanting to compile Gambit on their own, you'll need
to use either the Cygwin or MinGW environments.  We do compilation and
testing of Gambit on Windows using MinGW, which can be gotten from
`<http://www.mingw.org>`_.
We prefer MinGW over Cygwin because MinGW will create native Windows
applications, whereas Cygwin requires an extra compatibility layer.


For OS X users
--------------

For building the command-line tools only, one should follow the
instructions for Un*x/Linux platforms above.  ``make install`` will
install the command-line tools into ``/usr/local/bin`` (or the path
specified in the ``configure`` step).

To build the graphical interface, wxWidgets 2.8.12 is required.
(The interface will build with wxWidgets 2.9.4, but there is a bug
in wxWidgets involving drag-and-drop which renders the graphical interface
essentially unusable.)

Snow Leopard (OS X 10.8) users will have to take some extra steps to
build wxWidgets.  wxWidgets 2.8.12 requires the 10.6 SDK to build the
using Cocoa; this has been removed by Apple in recent editions of
XCode.  Download and unpack the 10.6 SDK from an earlier XCode version
into
``/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.6.sdk``.
With that in place, unpack the wxWidgets sources, and from the root
directory of the wxWidgets sources, do::

  mkdir build-debug
  cd build-debug
  arch_flags="-arch i386" CFLAGS="$arch_flags" CXXFLAGS="$arch_flags" \
     CPPFLAGS="$arch_flags" LDFLAGS="$arch_flags" OBJCFLAGS="$arch_flags" \ 
     OBJCXXFLAGS="$arch_flags" \
     ../configure  \
     --with-macosx-version-min=10.6 \
     --with-macosx-sdk=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.6.sdk \
     --prefix="$(pwd)" --disable-shared --enable-debug --enable-unicode
  make

Then, when configuring Gambit, use::

  arch_flags="-arch i386" CFLAGS="$arch_flags" CXXFLAGS="$arch_flags" \
     CPPFLAGS="$arch_flags" LDFLAGS="$arch_flags" OBJCFLAGS="$arch_flags" \ 
     OBJCXXFLAGS="$arch_flags" \
     ./configure --with-wxdir=WXPATH/build-debug
  make osx-bundle

where ``WXPATH`` is the path at which you have the wxWidgets sources
unpacked.

This produces an application ``Gambit.app`` in the current directory,
which can be run from its current location, or copied elsewhere in the
disk (such as ``/Applications``).  The application bundle includes the
command-line executables.


The graphical interface and wxWidgets
-------------------------------------

Gambit requires wxWidgets version 2.8.0 or higher for the
graphical interface.  See the wxWidgets website at
`<http://www.wxwidgets.org>`_
to download this if you need it.  Packages of this should be available
for most Un*x users through their package managers (apt or rpm).  Note
that you'll need the appropriate -dev package for wxWidgets to get the
header files needed to build Gambit.

The current recommended versions of wxWidgets are 2.9.4 for Windows
and Linux, and 2.8.12 for Mac OS X.  There is a bug in the 2.9.x
series of wxWidgets which affects drag-and-drop operations, and
therefore makes the graphical interface essentially useless.

Un*x users, please note that Gambit at this time only supports the
GTK port of wxWidgets, and not the Motif/Lesstif or the Universal ports.
Neither of the latter ports support drag-and-drop features, which are
heavily used in the graphical interface.

If wxWidgets it isn't installed in a standard place (e.g., /usr or
/usr/local), you'll need to tell configure where to find it with the
--with-wx-prefix=PREFIX option, for example::

  ./configure --with-wx-prefix=/home/mylogin/wx

Finally, if you don't want to build the graphical interface, you
can either (a) simply not install wxWidgets, or (b) pass the argument
--disable-gui to the configure step, for example, ::

  ./configure --disable-gui

This will just build the command-line tools, and will not require
a wxWidgets installation.


.. _build-python:

Building the Python extension
-----------------------------

The :ref:`Python extension for Gambit <python-api>` is in src/python
in the Gambit source tree.  Prerequisite packages include setuptools,
Cython, IPython, and scipy.

Building the extension follows the standard approach::

  cd src/python
  python setup.py build
  sudo python setup.py install

There is a set of test cases in src/python/gambit/tests.  These can
be exercised via nosetests (requires Python package nose)::

  cd src/python/gambit/tests
  nosetests

Once installed, simply ``import gambit`` in your Python shell or
script to get started.
