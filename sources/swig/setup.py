from distutils.core import setup, Extension

setup(name="_gambit",
      version="0.2007.01.21",
      py_modules=["gambit"],
      description="Software tools for game theory",
      author="The Gambit Project",
      author_email="gambit@econmail.tamu.edu",
      url="http://econweb.tamu.edu/gambit",
      ext_modules=[Extension("_gambit",
                             [ "gambit_wrap.cxx",
                               "libgambit/behav.cc",
                               "libgambit/behavitr.cc",
                               "libgambit/behavspt.cc",
                               "libgambit/dvector.cc",
                               "libgambit/file.cc",
                               "libgambit/game.cc",
                               "libgambit/integer.cc",
                               "libgambit/matrix.cc",
                               "libgambit/mixed.cc",
                               "libgambit/pvector.cc",
                               "libgambit/rational.cc",
                               "libgambit/sqmatrix.cc",
                               "libgambit/stratitr.cc",
                               "libgambit/stratspt.cc",
                               "libgambit/subgame.cc",
                               "libgambit/vector.cc" ],
                             include_dirs=["."])]
      )


