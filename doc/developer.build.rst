.. _building-from-source:

Building Gambit from source
===========================

This page covers building Gambit from a repository clone.  To install a released version, see :ref:`install`.

Gambit has two independent builds from the same C++ sources (see :ref:`repository-layout`):

* :ref:`pygambit <build-python>` and its ``gambit-*`` tools use ``pip`` and require a C++20
  compiler and Python 3.11 or later.
* The :ref:`graphical interface <cli-gui-from-source>` uses autotools and wxWidgets.


.. _getting-source:

Getting the source code
-----------------------

Clone the repository from GitHub ::

    git clone https://github.com/gambitproject/gambit.git
    cd gambit

.. _branches:

Branches
^^^^^^^^

* ``master`` is the latest development version.
* Topic branches contain work in progress.
* ``maintX`` and ``maintX_Y`` track stable major and minor versions.  Releases are tagged
  ``vX.Y.Z`` on these branches and listed on the
  `GitHub releases page <https://github.com/gambitproject/gambit/releases>`__.

To build a branch other than ``master``, check it out before building ::

    git fetch origin
    git switch <branch>

.. _source-dependencies:

Build tools and dependencies
----------------------------

The packages below provide a C++ compiler and autotools, plus wxWidgets and the tools that fetch
the catalog of games (``curl`` or ``wget``, ``sha256sum`` or ``shasum``, and ``unzip``) for the
graphical interface; macOS has the latter already.  Only the compiler is needed for ``pygambit``.
On macOS, install it with ``xcode-select --install``.

.. dropdown:: Install on macOS via Homebrew
   :class-container: sd-border-0

   1. Install `Homebrew <https://brew.sh/>`_ if ``brew --version`` fails.

   2. Install the dependencies:

      .. code-block:: bash

         brew install automake autoconf libtool wxwidgets

      .. note::
         For ``autom4te`` interpreter errors, check Perl or reinstall the packages:

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

         sudo apt install libwxgtk3.2-dev curl unzip

.. dropdown:: Install on Windows
   :class-container: sd-border-0

   The recommended way to build Gambit on modern Windows is to use the
   MSYS2 / MinGW-w64 environment.

   1. Install `MSYS2 <https://www.msys2.org/>`_ and follow its update instructions.

   2. Open the "MSYS2 UCRT64" terminal, not the plain MSYS shell.

   3. Install the build dependencies:

      .. code-block:: bash

         # Restart the terminal after this command if prompted.
         pacman -Syu
         pacman -S --needed mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-automake \
         mingw-w64-ucrt-x86_64-autoconf mingw-w64-ucrt-x86_64-libtool mingw-w64-ucrt-x86_64-make

   4. Install wxWidgets:

      .. code-block:: bash

         pacman -S --needed mingw-w64-ucrt-x86_64-wxwidgets3.2 curl unzip




.. _build-python:

Building pygambit
-----------------

From the repository root, install :ref:`pygambit <pygambit>` and its development dependencies in
a virtual environment:

.. code-block:: bash

   python -m venv venv
   source venv/bin/activate
   python -m pip install -v ".[test,doc]"

This compiles the C++ and Cython code and installs ``pygambit`` and the ``gambit-*`` tools.
``pyproject.toml`` defines these optional dependency groups:

* ``test``: the packages needed to run the :ref:`test suite <testing>`.
* ``doc``: the packages needed to :ref:`build this documentation <editing-docs>`.
* ``draw``: `gtdraw <https://github.com/gambitproject/gtdraw>`_, which draws game trees in the
  tutorials and the catalog; it is included in both ``test`` and ``doc``.

Re-run ``pip install`` after changing C++ or Cython.  An editable install (``pip install -e``)
picks up pure-Python changes only.

.. _cli-gui-from-source:

Building the graphical interface
--------------------------------

From the repository root (in the "MSYS2 UCRT64" terminal on Windows), run:

.. code-block:: bash

   aclocal
   libtoolize
   automake --add-missing
   autoconf
   ./configure
   make
   # Skip this on Windows:
   sudo make install

For an out-of-tree build, create a build directory and invoke ``configure`` by its relative path,
such as ``../configure``.

The graphical interface is installed with the catalog of games release pinned in
``build_support/catalog.toml``.  ``make`` downloads it into ``.catalog/`` in the source tree,
checks its SHA-256 and unpacks it; this needs the network once per release.  Source tarballs made
by ``make dist`` include the release, so building from one needs no network.

``configure`` accepts the following options, among others (see ``./configure --help``):

* ``--prefix=PATH``: install under ``PATH`` instead of ``/usr/local``.
* ``--with-wx-prefix=PREFIX``: the location of wxWidgets, if it is not installed in a standard
  place such as ``/usr`` or ``/usr/local``.
* ``--enable-werror``: treat compiler warnings as errors.  This is off by default, and is used by
  the Linux continuous-integration build.


.. dropdown:: Build macOS application bundle
   :class-container: sd-border-0

   Create a distributable DMG:

   .. code-block:: bash

      make osx-dmg

   Open the DMG and drag Gambit to the Applications folder.

.. dropdown:: Creating a Windows installer
   :class-container: sd-border-0

   Create an ``.msi`` installer with the `WiX toolset <https://github.com/wixtoolset>`__:

   .. code-block:: powershell

      make msw-msi

   Run the generated ``gambit-X.Y.Z.msi`` to install Gambit.

.. _wxwidgets-versions:

Supported wxWidgets versions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The GUI requires wxWidgets 3.2 or later.  Official macOS binaries use the latest stable 3.2.x;
continuous integration also tests the latest 3.3.x (Homebrew's ``wxwidgets@3.3``), but releases
do not yet use it.
