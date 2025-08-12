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
of the sources (typically of the form gambit-xx.y.z), and do the
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

  git clone https://github.com/gambitproject/gambit.git
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
version.


For Windows users
-----------------

For Windows users wanting to compile Gambit on their own, you'll need
to use either the Cygwin or MinGW environments.  We do compilation and
testing of Gambit on Windows using MinGW.


For OS X users
--------------

For building the command-line tools only, one should follow the
instructions for Un*x/Linux platforms above.


The graphical interface and wxWidgets
-------------------------------------

Gambit requires wxWidgets version 3.1.x or higher.
See the wxWidgets website at
`<http://www.wxwidgets.org>`_
to download this if you need it.  Packages of this should be available
for most Un*x users through their package managers (apt or rpm).  Note
that you'll need the appropriate -dev package for wxWidgets to get the
header files needed to build Gambit.

Un*x users, please note that Gambit at this time only supports the
GTK port of wxWidgets.

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

For OS X users, after the usual ``make`` step, run

  make osx-bundle

This produces an application ``Gambit.app`` in the current directory,
which can be run from its current location, or copied elsewhere in the
disk (such as ``/Applications``).  The application bundle includes the
command-line executables.



.. _build-python:

Building the Python extension
-----------------------------

The :ref:`pygambit Python package <pygambit>` is in ``src/pygambit``
in the Gambit source tree.
Building the extension follows the standard approach.
From the **root directory of the source tree** execute

    python -m pip install .

There is a set of test cases in `src/pygambit/tests`, which can be run
using `nose2`.

Once installed, simply ``import pygambit`` in your Python shell or
script to get started.

Editing this documentation
--------------------------

1. If you haven't already, clone the Gambit repository from GitHub: ::

    git clone https://github.com/gambitproject/gambit.git
    cd gambit

2. Either install the docs requirements into your existing PyGambit development environment, or create a new virtual environment and install both the requirements and PyGambit there. For example, you can use `venv` to create a new environment: ::

    python -m venv docenv
    source docenv/bin/activate

3. Install the requirements and make the docs: ::

    pip install .
    cd doc
    pip install -r requirements.txt
    make html  # or make livehtml for live server with auto-rebuild

4. Open ``doc/_build/html/index.html`` in your browser to view the documentation.

5. Make any changes you want to the `.rst` files in the ``doc`` directory and rebuld the documentation to check your changes.

6. Follow the usual GitHub workflow to commit your changes and push them to the repository.

7. Core developers will review your changes and merge to the master branch, which automatically deploys the documentation via the ReadTheDocs service.

.. TODO: Add instructions for the GitHub workflow during contributor docs refactoring.
   See https://github.com/gambitproject/gambit/issues/541

Bug reports
-----------

In the first instance, bug reports or feature requests should be
posted to the Gambit issue tracker, located at
`<http://github.com/gambitproject/gambit/issues>`_.

When reporting a bug, please be sure to include the following:

* The version(s) of Gambit you are using.  (If possible, it is helpful
  to know whether a bug exists in both the current stable/teaching and
  the current development/research versions.)
* The operating system(s) on which you encountered the bug.
* A detailed list of steps to reproduce the bug.  Be sure to include a
  sample game file or files if appropriate; it is often helpful to
  simplify the game if possible.