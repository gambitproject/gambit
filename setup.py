#
# This file is part of Gambit
# Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
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
import platform
import setuptools
import Cython.Build

# By compiling this separately as a C library, we avoid problems
# with passing C++-specific flags when building the extension
lrslib = ('lrslib', {'sources': glob.glob("src/solvers/lrs/*.c")})

cppgambit = (
    'cppgambit',
    {
        'sources': (
            glob.glob("src/core/*.cc") +
            glob.glob("src/games/*.cc") +
            glob.glob("src/games/agg/*.cc") +
            [fn for fn in glob.glob("src/solvers/*/*.cc") if "enumpoly" not in fn]
         ),
         'include_dirs': ["src"],
         'cflags': (
             ["-std=c++11"] if platform.system() == "Darwin" else []
         )
    }
)
    

libgambit = setuptools.Extension(
    "pygambit.gambit",
    sources=["src/pygambit/gambit.pyx"],
    language="c++",
    include_dirs=["src", "src/pygambit"],
    extra_compile_args=(
        ["-std=c++11"] if platform.system() == "Darwin" else []
    )
)


def readme():
    with open("src/README.rst") as f:
        return f.read()
    

setuptools.setup(
    name="pygambit",
    version="16.1.1",
    description="The package for computation in game theory",
    long_description=readme(),
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: GNU General Public License v2 or later (GPLv2+)",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Programming Language :: Python :: Implementation :: CPython",
        "Topic :: Scientific/Engineering :: Mathematics"
    ],
    keywords="game theory Nash equilibrium",
    license="GPL2+",
    author="Theodore Turocy",
    author_email="ted.turocy@gmail.com",
    url="http://www.gambit-project.org",
    project_urls={
        'Documentation': 'https://gambitproject.readthedocs.io/',
        'Source': 'https://github.com/gambitproject/gambit',
        'Tracker': 'https://github.com/gambitproject/gambit/issues',
    },
    python_requires=">=3.8",
    install_requires=[
        'lxml',  # used for reading/writing GTE files
        'numpy',
        'scipy',
        'deprecated',
    ],
    libraries=[cppgambit, lrslib],
    package_dir={'': 'src'},
    packages=['pygambit'],
    ext_modules=Cython.Build.cythonize(libgambit,
                                       language_level="3str",
                                       compiler_directives={'binding': True})
)
