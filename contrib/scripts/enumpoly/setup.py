# This is a distutils/py2exe script to build the Windows binary version
# of gambit-enumphc

from distutils.core import setup
import py2exe

setup(console=["enumphc.py"],
      data_files=[(".",
                   [ "phc.exe", "README" ])])
