from setuptools import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

# setuptools DWIM monkey-patch madness
# http://mail.python.org/pipermail/distutils-sig/2007-September/thread.html#8204
import sys
if 'setuptools.extension' in sys.modules:
    m = sys.modules['setuptools.extension']
    m.Extension.__dict__ = m._Extension.__dict__
    
import glob
libgame = Extension("gambit.lib.libgambit",
                    sources=[ "gambit/lib/libgambit.pyx" ] +
                            glob.glob("gambit/lib/*.pxi") +
                            glob.glob("../libgambit/*.cc") +
                            glob.glob("../libagg/*.cc"),
                    language="c++",
                    include_dirs=[ ".." ] )

setup(name="gambit",
      description="A library for doing game theory",
      author="Theodore Turocy",
      author_email="ted.turocy@gmail.com",
      url="http://www.gambit-project.org",
      packages=['gambit', 'gambit.games', 'gambit.lib'],
      ext_modules=[libgame],
      cmdclass = {'build_ext': build_ext},
      entry_points="""
      [console_scripts]
      gambit-shell = gambit.cmdline:gambit_shell
      """
      )
