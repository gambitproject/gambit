Building Gambit from source
===========================

This page covers instructions for building Gambit from source.
This is for those who are interested in developing Gambit, or who
want to play around with the latest features before they make it
into a pre-compiled binary version.

This page requires at least some familiarity with programming.
Most users will want to stick with released distributions; see the 
:ref:`install` page for how to get the current version for
your operating system.
Following the instructions here will install the Gambit CLI, GUI and Python extension (PyGambit).

The steps you will need to follow to build from source are as follows:

1. Refer to the :ref:`contributor page <contributing-code>` which explains how to clone the Gambit repository from GitHub (you may first wish to create a fork).
2. :ref:`Install the necessary build tools and dependencies for your platform <source-dependencies>`.
3. :ref:`Follow the platform-specific instructions to build and install Gambit CLI and GUI components from source <cli-gui-from-source>`.
4. :ref:`Build the Python extension (PyGambit) <build-python>`.

.. _source-dependencies:

Install build tools and dependencies
------------------------------------

.. _cli-gui-from-source:

Install CLI and GUI from source
--------------------------------

After this, you will need to set up the build scripts by executing ::

  aclocal
  libtoolize
  automake --add-missing
  autoconf

For this, you will need to have automake, autoconf, and libtool2
installed on your system.

You can then configure and build Gambit like so:

.. code-block:: bash

    ./configure
    make
    sudo make install

.. note::
  Command-line options are available to modify the configuration process;
  do `./configure --help` for information.  Of these, the option which
  may be most useful is to disable the build of the graphical interface.

  By default Gambit will be installed in /usr/local.  You can change this
  by replacing configure step with one of the form ::

  `./configure --prefix=/your/path/here`

.. warning::
  The graphical interface relies on external calls to other
  programs built in this process, especially for the computation of
  equilibria.  It is strongly recommended that you install the Gambit
  executables to a directory in your path!

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

.. _build-mac:

.. dropdown:: Manual macOS Build Instructions
   :class-container: sd-border-0
   
   To build and install the Gambit GUI from source on macOS, follow these steps:

   1. **Install build dependencies:**

      .. code-block:: bash

         brew install automake autoconf libtool wxwidgets

      .. note::
         If you encounter interpreter errors with autom4te, you may need to ensure
         your Perl installation is correct or reinstall the autotools:

         .. code-block:: bash

            brew reinstall automake autoconf libtool wxwidgets

   2. **Build and install Gambit:**

      Navigate back to the Gambit source directory and run:

      .. code-block:: bash

         aclocal
         automake --add-missing
         autoconf
         ./configure
         make
         sudo make install

   3. **Create macOS application bundle:**

      To create a distributable DMG file:

      .. code-block:: bash

         make osx-dmg

   4. **Install the application:**

      After creating the DMG file, open it and drag the Gambit application to your Applications folder.

The graphical interface and wxWidgets
-------------------------------------

Note that Gambit requires wxWidgets version 3.2.x or higher.
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
