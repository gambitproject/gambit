.. _install:


Install
=======

Users installing Gambit have several options depending on their needs and their operating system.
We recommended most new users install the PyGambit package and read the :ref:`PyGambit documentation <pygambit>`.

Developers & contributors should refer to the :ref:`developer docs <developer>` which contain guides for building Gambit from source and contributing to the project.

Installing PyGambit
-------------------

PyGambit is available on `PyPI <https://pypi.org/project/pygambit/>`_. We recommend installing it into a Python virtual environment via `pip`::

   pip install pygambit


Older releases can be installed by specifying the version number.
Visit the `Gambit releases page on GitHub <https://github.com/gambitproject/gambit/releases>`_ for information on older versions.

.. _install-cli-gui:

Installing Gambit GUI & CLI tools
---------------------------------

To install the Gambit :ref:`GUI <section-gui>` and :ref:`CLI tools <command-line>`, visit the `Gambit releases page on GitHub <https://github.com/gambitproject/gambit/releases>`_ and download the appropriate installer or package for your operating system.
Each release includes pre-built binaries for Windows, macOS, and Linux distributions, accessible under the "Assets" section of each release.

.. dropdown:: Install on macOS with disk image
   :class-container: sd-border-0
   
   1. **Download the .dmg installer:**

      Visit the `Gambit releases page on GitHub <https://github.com/gambitproject/gambit/releases>`_ and download the `.dmg` file for the version of Gambit you wish to install.

   2. **Install the application:**

      Double click the `.dmg` file to mount it, then drag the Gambit application to your Applications folder.

   .. warning::
    You may need to adjust your macOS security settings to allow the installation of applications from unidentified developers.

    This can be done in ``System Preferences > Security & Privacy`` (see `Apple's documentation <https://support.apple.com/en-gb/guide/mac-help/mchleab3a043/mac>`_ for more details).

    If your administration privileges prevent this, try the Homebrew installation method below, or build from source as described in the :ref:`developer build instructions <build-mac>`.

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

      Visit the `Gambit releases page on GitHub <https://github.com/gambitproject/gambit/releases>`_ and download the source tarball for the version of Gambit you wish to install.

   2. **Extract the tarball:**

      Once downloaded, extract the tarball using the following command:

      .. code-block:: bash

         tar -xzf gambit-*.tar.gz

   3. **Build and install Gambit:**

      Navigate to the extracted directory and run:

      .. code-block:: bash

         ./configure
         make
         sudo make install

.. dropdown:: Install on Windows with installer
   :class-container: sd-border-0

   1. **Download the installer:**

      Visit the `Gambit releases page on GitHub <https://github.com/gambitproject/gambit/releases>`_ and download the `.msi`.

   2. **Run the installer:**

      Double click the downloaded `.msi` file and follow the on-screen instructions to complete the installation.