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

   3. Install GUI dependencies (replace X.X with the latest version available, e.g., 3.2):

      .. code-block:: bash

         sudo apt-cache search libwxgt*
         sudo apt install libwxgtkX.X-dev

      .. note::
         Note that Gambit requires wxWidgets version 3.2.x or higher.

.. dropdown:: Install on Windows
   :class-container: sd-border-0

    .. warning::
       TODO: Add Windows build dependencies instructions here.

.. _cli-gui-from-source:

Install CLI and GUI from source
--------------------------------

.. dropdown:: Install on Windows
   :class-container: sd-border-0

    .. warning::
       For Windows users wanting to compile Gambit on their own, you'll need to use either the Cygwin or MinGW environments.  We do compilation and testing of Gambit on Windows using MinGW.

The rest of these instructions are applicable to **Linux** and **macOS** users.
Navigate to the Gambit source directory and run:

.. code-block:: bash

    aclocal
    libtoolize
    automake --add-missing
    autoconf
    ./configure
    make
    sudo make install

.. note::
  If you don't want to build the graphical interface, you can pass the argument `--disable-gui` to the configure step, for example, ::

  `./configure --disable-gui`

.. warning::
  For Linux users:
  If wxWidgets it isn't installed in a standard place (e.g., /usr or
  /usr/local), you'll need to tell configure where to find it with the
  `--with-wx-prefix=PREFIX` option, for example::

    `./configure --with-wx-prefix=/home/mylogin/wx`

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

.. dropdown:: Build macOS application bundle
   :class-container: sd-border-0

   1. **Create macOS application bundle:**

      To create a distributable DMG file:

      .. code-block:: bash

         make osx-dmg

   2. **Install the application:**

      After creating the DMG file, open it and drag the Gambit application to your Applications folder.

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
