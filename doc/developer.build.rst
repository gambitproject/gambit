.. _building-from-source:

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

.. dropdown:: Install on macOS via Homebrew
   :class-container: sd-border-0
   
   1. Check that you have Homebrew installed by running `brew --version`. If not, follow the instructions at `https://brew.sh/ <https://brew.sh/>`_.

   2. Install build dependencies:

      .. code-block:: bash

         brew install automake autoconf libtool wxwidgets

      .. note::
         If you encounter interpreter errors with autom4te, you may need to ensure
         your Perl installation is correct or reinstall the autotools:

         .. code-block:: bash

            brew reinstall automake autoconf libtool wxwidgets

.. dropdown:: Install on Linux (Debian/Ubuntu) via apt
   :class-container: sd-border-0

   1. Update your package lists:

      .. code-block:: bash

         sudo apt update

   2. Install general build dependencies:

      .. code-block:: bash

         sudo apt install build-essential automake autoconf libtool

   3. Install GUI dependencies:

      .. code-block:: bash

         sudo apt install libwxgtk3.2-dev

.. dropdown:: Install on Windows
   :class-container: sd-border-0

   The recommended way to build Gambit on modern Windows is to use the
   MSYS2 / MinGW-w64 environment.

   1. Download and install MSYS2 from https://www.msys2.org/ and follow
      the update instructions there (you will typically run ``pacman -Syu``
      and restart the MSYS2 terminal as instructed).

   2. Open the "MSYS2 MinGW 64-bit" terminal (important: use the MinGW
      shell, not the plain MSYS shell).

   3. Install general build dependencies

      .. code-block:: bash

         # update package DB & core packages first (may require closing/reopening the shell)
         pacman -Syu

         # install compiler toolchain, autotools and libtool
         pacman -S --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-automake \
         mingw-w64-x86_64-autoconf mingw-w64-x86_64-libtool mingw-w64-x86_64-make

   4. Install GUI dependencies

      .. code-block:: bash

         pacman -S --needed mingw-w64-x86_64-wxwidgets3.2

   .. note::
      When building for a different target (32-bit) substitute the corresponding MinGW packages (``mingw-w64-i686-*``).


.. _cli-gui-from-source:

Install CLI and GUI from source
--------------------------------

Navigate to the Gambit source directory (use the "MSYS2 MinGW 64-bit" terminal on Windows) and run:

.. code-block:: bash

   aclocal
   libtoolize
   automake --add-missing
   autoconf
   ./configure
   make
   # Skip this on Windows:
   sudo make install

.. dropdown:: Build macOS application bundle
   :class-container: sd-border-0

   1. Create macOS application bundle:

      To create a distributable DMG file:

      .. code-block:: bash

         make osx-dmg

   2. Install the application:

      After creating the DMG file, open it and drag the Gambit application to your Applications folder.

.. dropdown:: Creating a Windows installer
   :class-container: sd-border-0

   1. Create a ``.msi`` installer using the WiX toolset:

      .. code-block:: powershell

         mkdir installer
         copy gambit* installer
         "$env:WIX\bin\candle" gambit.wxs
         "$env:WIX\bin\light" -ext WixUIExtension gambit.wixobj

   2. Install the application:

      Run the generated ``gambit-X.Y.Z.msi`` file to install Gambit on your system.

.. note::
  Command-line options are available to modify the configuration process;
  do `./configure --help` for information.  Of these, the option which
  may be most useful is to disable the build of the graphical interface.

  By default Gambit will be installed in /usr/local.  You can change this
  by replacing configure step with one of the form ::

  ./configure --prefix=/your/path/here

.. note::
   If you don't want to build the graphical interface, you can pass the argument ``--disable-gui`` to the configure step, for example::

      ./configure --disable-gui

.. warning::
   If wxWidgets isn't installed in a standard place (e.g., ``/usr`` or
   ``/usr/local``), you'll need to tell ``configure`` where to find it with the
   ``--with-wx-prefix=PREFIX`` option, for example::

      ./configure --with-wx-prefix=/home/mylogin/wx

.. warning::
  The graphical interface relies on external calls to other
  programs built in this process, especially for the computation of
  equilibria.  It is strongly recommended that you install the Gambit
  executables to a directory in your path!

.. _build-python:

Building the Python extension
-----------------------------

The :ref:`pygambit Python package <pygambit>` is in ``src/pygambit``
in the Gambit source tree. We recommend to install `pygambit`
as part of a virtual environment rather than in the system's Python.
Use `pip` to install from the **root directory of the source tree**, optionally including the `-e` flag for an editable install:

.. code-block:: bash

   python -m pip install .

There is a set of test cases in `src/pygambit/tests`, which can be run
using `nose2`.

Once installed, simply ``import pygambit`` in your Python shell or
script to get started.
