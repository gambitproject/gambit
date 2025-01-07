#################
Welcome to Gambit
#################



**Gambit** is a library of game theory software and tools for the
construction and analysis of finite extensive and strategic games.
Gambit is fully-cross platform, and is supported on Linux, Mac OS X,
and Microsoft Windows.

Key features of Gambit include:

* A :ref:`graphical user interface <section-gui>`, which uses
  `wxWidgets <http://www.wxwidgets.org>`_ to provide a common
  interface with native look-and-feel across platforms.
* All equilibrium-computing algorithms are available as
  :ref:`command-line tools <command-line>`, callable from scripts and
  other programs.
* A :ref:`Python API <pygambit>` for developing scripting applications.


Gambit is Free/Open Source software, released under the terms of the
`GNU General Public License <https://www.gnu.org/copyleft/gpl.html>`_,
Version 2.

We hope you will find Gambit useful for both teaching and research
applications. If you do use Gambit in a class, or in a paper, we would
like to hear about it. We are especially interested in finding out
what you like about Gambit, and where you think improvements could be
made.

If you are citing Gambit in a paper, we suggest a citation of the form:

  Savani, Rahul and Turocy, Theodore L. (2025)
  Gambit: The package for computation in game theory, Version 16.2.1.
  https://www.gambit-project.org.

Replace the version number and year as appropriate if you use a
different release.


.. grid::

   .. grid-item-card:: Python user guide
      :columns: 6

      An introduction to using the ``pygambit`` package
      in Python.

      .. button-ref:: pygambit-user
         :ref-type: ref
         :click-parent:
         :color: secondary
         :expand:


   .. grid-item-card:: Python API reference
      :columns: 6

      The complete reference to all the functionality
      of ``pygambit``.

      .. button-ref:: pygambit-api
         :ref-type: ref
         :click-parent:
         :color: secondary
         :expand:


   .. grid-item-card:: Command-line tools
      :columns: 6

      All Gambit's methods for equilibrium computation are
      available via command-line programs.

      .. button-ref:: command-line
         :ref-type: ref
         :click-parent:
         :color: secondary
         :expand:

   .. grid-item-card:: Graphical interface
      :columns: 6

      Gambit's graphical interface lets you interactively
      create, explore, and find equilibria of games.

      .. button-ref:: section-gui
         :ref-type: ref
         :click-parent:
         :color: secondary
         :expand:



.. toctree::
   :hidden:
   :maxdepth: 1

   intro
   tools
   pygambit
   gui
   samples
   build
   formats
   biblio

.. toctree::
   :hidden:

   contents

.. Or, see a :ref:`more detailed table of contents <full-contents>`.

.. Indices and tables
.. ==================

.. * :ref:`genindex`
.. * :ref:`modindex`
.. * :ref:`search`
