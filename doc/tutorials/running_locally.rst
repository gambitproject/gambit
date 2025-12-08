.. _local_tutorials:

How to run PyGambit tutorials on your computer
==============================================

Running the PyGambit tutorials on your machine requires some familiarity with the basics of Python and how to use Git & GitHub.
The tutorials are available as Jupyter notebooks and can be run interactively using any program that supports Jupyter notebooks, such as JupyterLab or VSCode.

.. tip:: Create a virtual environment with Python 3.13 or higher

1. To download the tutorials, open your OS's command prompt and clone the Gambit repository from GitHub, then navigate to the tutorials directory: ::

    git clone https://github.com/gambitproject/gambit.git

2. Install `pygambit`:

   * To install the latest release from PyPI::

       pip install pygambit

   * Alternatively, to install the latest development version::

       pip install .

3. Install other requirements (including `JupyterLab`) used by the tutorials ::

    cd gambit/doc
    pip install -r requirements.txt

   .. warning::
       Windows users wishing to run the "Using Gambit with OpenSpiel" tutorial will need to install OpenSpiel manually; see the `OpenSpiel installation instructions <https://openspiel.readthedocs.io/en/latest/windows.html>`_ for details.

4. Open `JupyterLab` and click on any of the tutorial notebooks (files ending in `.ipynb`) ::

    cd tutorials
    jupyter lab
