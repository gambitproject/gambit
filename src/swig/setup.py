from distutils.core import setup, Extension


libgambit_ext = Extension("gambit._libgambit",
                          [ "gambit/libgambit_wrap.cxx",
                            "../libgambit/behav.cc",
                            "../libgambit/behavitr.cc",
                            "../libgambit/behavspt.cc",
                            "../libgambit/dvector.cc",
                            "../libgambit/file.cc",
                            "../libgambit/game.cc",
                            "../libgambit/integer.cc",
                            "../libgambit/matrix.cc",
                            "../libgambit/mixed.cc",
                            "../libgambit/pvector.cc",
                            "../libgambit/rational.cc",
                            "../libgambit/sqmatrix.cc",
                            "../libgambit/stratitr.cc",
                            "../libgambit/stratspt.cc",
                            "../libgambit/subgame.cc",
                            "../libgambit/vector.cc" ],
                          include_dirs=[".."])
                          

setup(name="gambit",
      version="0.2007.12.04",
      py_modules=["gambit.libgambit", "gambit.msgame",
                  "gambit.nash", "gambit.qretools", "gambit.pctrace"],
      description="Software tools for game theory",
      author="The Gambit Project",
      author_email="gambit@econmail.tamu.edu",
      url="http://gambit.sourceforge.net",
      ext_modules=[libgambit_ext]
      )


