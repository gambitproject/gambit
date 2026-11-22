.. _editing-docs:

Documentation
=============

`Sphinx <https://www.sphinx-doc.org>`__ builds this documentation from ``doc``.  ReadTheDocs
builds every pull request and ``master`` according to ``.readthedocs.yaml``.  The catalog's
documentation in ``catalog/doc`` is built and hosted separately.

Layout
------

* ``doc/conf.py``: Sphinx configuration, including the bibliography style.
* ``doc/index.rst``: the front page.  Other pages are grouped by filename prefix:

  * ``install.rst``: installation.
  * ``pygambit.rst`` and ``pygambit.api.rst``: the ``pygambit`` guide, tutorials, and API
    reference.  ``autosummary`` generates the API reference from docstrings with templates in
    ``doc/_templates/autosummary``.
  * ``tools.*.rst``: the command-line tools.
  * ``gui.*.rst``: the graphical interface.
  * ``formats.*.rst``: the game and output file formats.
  * ``algorithms.rst``, ``catalog_generators.rst`` and ``biblio.rst``: the algorithms, game
    generators, and bibliography.
  * ``developer.*.rst``: these pages.

* ``doc/tutorials``: Jupyter notebooks rendered by ``nbsphinx``.  The
  ``advanced_tutorials`` and ``interoperability_tutorials`` subdirectories contain advanced
  tutorials and integrations.  ``doc/pygambit.rst`` lists the notebooks, and the test suite runs
  them (see :ref:`testing`).  Their dependencies belong in ``pyproject.toml``'s ``doc`` group.
* ``doc/references.bib``: the bibliography (see :ref:`updating-bibliography`).
* ``doc/_static`` and ``doc/_templates``: stylesheets, images, and page templates.

Building locally
----------------

1. `Install Pandoc <https://pandoc.org/installing.html>`_.

2. Install ``pygambit`` with the ``doc`` dependencies: see :ref:`build-python`.

3. Build the documentation: ::

    cd doc
    make html  # or make livehtml for live server with auto-rebuild

Open the generated ``doc/_build/html/index.html`` in a browser.

.. _updating-bibliography:

The bibliography
----------------

BibTeX entries in ``doc/references.bib`` are shared with the catalog documentation.  Each entry's
``category`` field determines its section on the bibliography page:

* ``articles_equilibria`` for articles on computation of equilibria.
* ``articles_general`` for general game theory articles and texts.
* ``textbooks`` for textbooks and general references.

Documentation and game files cite entries as ``:cite:p:`key```, using the entry's BibTeX key.

``doc/conf.py`` defines the custom Harvard style in Python.  A traditional ``.bst`` would emit
raw LaTeX that Sphinx cannot render as HTML.
