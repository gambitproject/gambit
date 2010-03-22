from setuptools import setup, find_packages
from Cython.Distutils.extension import Extension
from Cython.Distutils import build_ext

import glob

setup(name="gambit",
      description="A library for doing game theory",
      author="Theodore Turocy",
      author_email="ted.turocy@gmail.com",
      url="http://www.gambit-project.org",
      packages=['gambit' ],
      ext_modules = [ Extension("gambit.lib.game",
                                sources=[ "gambit/lib/game.wrap.pyx" ] +
                                        glob.glob("../libgambit/*.cc"),
                                language="c++",
                                include_dirs=[ ".." ] ) ],
      cmdclass = {'build_ext': build_ext},
      entry_points="""
      [console_scripts]
      gambit-shell = gambit.cmdline:gambit_shell
      """
      )
