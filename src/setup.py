#
# This file is part of Gambit
# Copyright (c) 1994-2016, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/setup.py
# Setuptools configuration file for Gambit Python extension
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

import glob
import setuptools
import Cython.Build


cppgambit = setuptools.Extension(
    "pygambit.lib.libgambit",
    sources=(
        ["pygambit/lib/libgambit.pyx"] +
        glob.glob("core/*.cc") +
        glob.glob("games/*.cc") +
        glob.glob("games/agg/*.cc") +
        glob.glob("solvers/*/*.c") +
        glob.glob("solvers/*/*.cc") +
        ["tools/lp/nfglp.cc",
         "tools/lp/efglp.cc",
         "tools/logit/path.cc",
         "tools/logit/nfglogit.cc",
         "tools/logit/efglogit.cc"]
    ),
    language="c++",
    include_dirs=["."]
)


setuptools.setup(
    name="pygambit",
    version="16.0.1",
    description="Software tools for game theory",
    author="Theodore Turocy",
    author_email="ted.turocy@gmail.com",
    url="http://www.gambit-project.org",
    python_requires=">=3.7",
    packages=['pygambit', 'pygambit.games', 'pygambit.lib'],
    ext_modules=Cython.Build.cythonize(cppgambit)
)
