.. _releases:

Releases & Downloads
====================

The current stable release of Gambit is **|release|**.

Downloads
---------

.. tab-set::

   .. tab-item:: PyGambit (Python)

      PyGambit is available on `PyPI <https://pypi.org/project/pygambit/>`_ and is the
      recommended way to use Gambit from Python::

         pip install pygambit

      To install a specific older version::

         pip install pygambit==X.Y.Z

   .. tab-item:: Windows

      Download the Windows installer (`.msi`) for Gambit |release| from the
      `GitHub releases page <https://github.com/gambitproject/gambit/releases/tag/v|release|>`__.

      Double-click the downloaded `.msi` file and follow the on-screen instructions.

   .. tab-item:: macOS

      **Disk image (.dmg)**

      Download the macOS disk image for Gambit |release| from the
      `GitHub releases page <https://github.com/gambitproject/gambit/releases/tag/v|release|>`__.

      Double-click the `.dmg` to mount it, then drag Gambit to your Applications folder.

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

      **Homebrew**

      .. code-block:: bash

         brew install gambit

      .. warning::
         Homebrew installation has not been set up or tested by the Gambit development team.

   .. tab-item:: Linux (source tarball)

      Download the source tarball for Gambit |release| from the
      `GitHub releases page <https://github.com/gambitproject/gambit/releases/tag/v|release|>`__.

      Extract and build::

         tar -xzf gambit-|release|.tar.gz
         cd gambit-|release|
         ./configure
         make
         sudo make install

      Run ``./configure --help`` for available options, including ``--prefix`` to change
      the installation directory.

      .. warning::
         The graphical interface relies on the other Gambit executables built in this
         process.  Install the executables to a directory that is in your ``PATH``.

Older releases
--------------

All past releases are available on the
`GitHub releases page <https://github.com/gambitproject/gambit/releases>`__.
See the :ref:`install` page for platform-specific installation instructions.

Full changelog
--------------

.. toctree::
   :hidden:

   changelog

:doc:`View full changelog <changelog>`
