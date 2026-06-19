.. _install:


Install
===================

Users installing Gambit have several options depending on their needs and their operating system.
We recommended most new users install the PyGambit package and read the :ref:`PyGambit documentation <pygambit>`.

Developers & contributors should refer to the :ref:`developer docs <developer>` which contain guides for building Gambit from source and contributing to the project.

Installing PyGambit
-------------------

PyGambit is available on `PyPI <https://pypi.org/project/pygambit/>`_. We recommend installing it into a Python virtual environment via `pip`::

   pip install pygambit

To install a specific older version::

   pip install pygambit==X.Y.Z

.. _releases:

Releases & Downloads
--------------------

The current stable release of Gambit is **|release|**.

.. dropdown:: Install on Windows
   :class-container: sd-border-0

   1. **Download the installer:**

      Download the `.msi` for Gambit |release| from the
      `GitHub releases page <https://github.com/gambitproject/gambit/releases/tag/v|release|>`__.

   2. **Run the installer:**

      Double-click the downloaded `.msi` file and follow the on-screen instructions to complete the installation.

.. _install-cli-gui:

.. dropdown:: Install on macOS with disk image
   :class-container: sd-border-0

   1. **Download the .dmg installer:**

      Download the `.dmg` for Gambit |release| from the
      `GitHub releases page <https://github.com/gambitproject/gambit/releases/tag/v|release|>`__.

   2. **Install the application:**

      Double-click the `.dmg` file to mount it, then drag the Gambit application to your Applications folder.

   .. warning::
    Gambit's macOS application is **not signed or notarized** by Apple.
    Gatekeeper will block opening it by default.

    To open it anyway, right-click (or Control-click) the Gambit application and choose
    **Open** from the context menu, then confirm in the dialog that appears.
    Alternatively, go to **System Settings → Privacy & Security** and click **Open Anyway**
    after the first blocked launch attempt.

    If your administrator privileges prevent this, use the Homebrew installation
    or build from source (see the :ref:`developer build instructions <building-from-source>`).

    Signing and notarization for macOS releases is tracked in
    `issue #712 <https://github.com/gambitproject/gambit/issues/712>`__.

.. dropdown:: Install on macOS via Homebrew
   :class-container: sd-border-0

   1. Check that you have Homebrew installed by running `brew --version`. If not, follow the instructions at `https://brew.sh/ <https://brew.sh/>`_.

   2. Install Gambit using Homebrew:

      .. code-block:: bash

         brew install gambit

   .. warning::
    Homebrew installation has not been set up or tested by the Gambit development team.

.. dropdown:: Install on Linux or macOS from source tarball
   :class-container: sd-border-0

   1. **Download the source tarball:**

      Download the source tarball for Gambit |release| from the
      `GitHub releases page <https://github.com/gambitproject/gambit/releases/tag/v|release|>`__.

   2. **Extract the tarball:**

      Once downloaded, extract the tarball using the following command:

      .. code-block:: bash

         tar -xzf gambit-|release|.tar.gz

   3. **Build and install Gambit:**

      Navigate to the extracted directory and run:

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

Older releases
--------------

All past releases are available on the
`GitHub releases page <https://github.com/gambitproject/gambit/releases>`__.

Full changelog
--------------

.. toctree::
   :hidden:

   changelog

:doc:`View full changelog <changelog>`
