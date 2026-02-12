.. _local_tutorials:

How to run PyGambit tutorials on your computer
==============================================

Running the PyGambit tutorials on your machine requires some familiarity with the basics of Python and how to use Git & GitHub.
The tutorials are available as Jupyter notebooks and can be run interactively using any program that supports Jupyter notebooks, such as JupyterLab or VSCode.

.. tip:: Create a virtual environment with Python 3.10 or higher.

1. To download the tutorials, open your OS's command prompt and clone the Gambit repository from GitHub: ::

    git clone https://github.com/gambitproject/gambit.git

2. Install `pygambit` with additional documentation dependencies:

   * To install the latest release from PyPI::

       pip install pygambit[doc]

   * Alternatively, to install the latest development version (from the top level of the gambit repo)::

       pip install .[doc]

   .. warning::
       Windows users wishing to run the "Using Gambit with OpenSpiel" tutorial will need to install OpenSpiel manually; see the `OpenSpiel installation instructions <https://openspiel.readthedocs.io/en/latest/windows.html>`_ for details.

3. Open `JupyterLab` and click on any of the tutorial notebooks in ``doc/tutorials`` (files ending in `.ipynb`) ::

    cd doc/tutorials
    jupyter lab
