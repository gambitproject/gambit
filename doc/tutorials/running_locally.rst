.. _local_tutorials:

How to run PyGambit tutorials on your computer
==============================================

The PyGambit tutorials are available as Jupyter notebooks and can be run interactively using any program that supports Jupyter notebooks, such as JupyterLab or VSCode.
You will need a working installation of Python 3 (tested with 3.9 and later) on your machine.

1. To download the tutorials, open your OS's command prompt and clone the Gambit repository from GitHub, then navigate to the tutorials directory: ::

    git clone https://github.com/gambitproject/gambit.git
    cd gambit/doc/tutorials

2. Install `PyGambit` and `JupyterLab`. We recommend creating a new virtual environment and installing both the requirements there. e.g. ::

    python -m venv pygambit-env
    source pygambit-env/bin/activate
    pip install pygambit jupyterlab

3. Open `JupyterLab` and click on any of the tutorial notebooks (files ending in `.ipynb`) ::

    jupyter lab
