.. _local_tutorials:

How to run PyGambit tutorials on your computer
==============================================

Running the PyGambit tutorials on your machine requires some familiarity with the basics of Python and how to use Git & GitHub.
The tutorials are available as Jupyter notebooks and can be run interactively using any program that supports Jupyter notebooks, such as JupyterLab or VSCode.

.. tip:: Create a virtual environment with Python 3.13 or higher

1. To download the tutorials, open your OS's command prompt and clone the Gambit repository from GitHub, then navigate to the tutorials directory: ::

    git clone https://github.com/gambitproject/gambit.git

2. Install the latest release of `pygambit` from PyPI ::

    pip install pygambit

    .. note::
     Alternatively, to install the latest development version of `pygambit`, run ::

        pip install .

2. Install `pygambit` and other requirements (including `JupyterLab` and other packages used by the tutorials) ::

    cd gambit/doc
    pip install -r requirements.txt

3. Open `JupyterLab` and click on any of the tutorial notebooks (files ending in `.ipynb`) ::

    cd tutorials
    jupyter lab
