.. _section-gui:

***
GUI
***

Gambit's graphical user interface provides an
"integrated development environment" to help visually construct
games and to investigate their main strategic features.

The graphical interface is largely intended for the interactive
construction and analysis of small to medium games. Repeating the
caution from the introduction of this manual, the computation time
required for the equilibrium analysis of games increases rapidly in
the size of the game. The graphical interface is ideal for students
learning about the fundamentals of game theory, or for practitioners
prototyping games of interest.

In graduating to larger applications,
users are encouraged to make use of the underlying Gambit libraries
and programs directly.  For greater control over computing Nash and
quantal response equilibria of a game, see the section on
:ref:`the command-line tools <command-line>`.
To build larger games or to explore parameter spaces of a game
systematically, it is recommended to use
:ref:`the Python package <pygambit>`.

Installation
------------

To install the Gambit GUI, visit the `Gambit releases page on GitHub <https://github.com/gambitproject/gambit/releases>`_ and download the appropriate installer or package for your operating system.
Each release includes pre-built binaries for Windows, macOS, and Linux distributions, accessible under the "Assets" section of each release.

.. dropdown:: Manual macOS Build Instructions
   :class-container: sd-border-0
   
   To build and install the Gambit GUI from source on macOS, follow these steps:

   1. **Install build dependencies:**

      .. code-block:: bash

         brew install automake autoconf libtool

      .. note::
         If you encounter interpreter errors with autom4te, you may need to ensure
         your Perl installation is correct or reinstall the autotools:

         .. code-block:: bash

            brew reinstall automake autoconf libtool

   2. **Download and build wxWidgets:**

      .. code-block:: bash

         curl -L -O https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.8/wxWidgets-3.2.8.tar.bz2
         tar xjf wxWidgets-3.2.8.tar.bz2
         cd wxWidgets-3.2.8
         mkdir build-release
         cd build-release
         ../configure --disable-shared --disable-sys-libs
         make -j4
         sudo make install

   3. **Build and install Gambit:**

      Navigate back to the Gambit source directory and run:

      .. code-block:: bash

         aclocal
         automake --add-missing
         autoconf
         ./configure
         make
         sudo make install

   4. **Create macOS application bundle:**

      To create a distributable DMG file:

      .. code-block:: bash

         make osx-dmg

   5. **Install the application:**

      After creating the DMG file, open it and drag the Gambit application to your Applications folder.

.. toctree::
   :maxdepth: 2

   gui.general
   gui.efg
   gui.nfg
   gui.dominance
   gui.nash
   gui.export
