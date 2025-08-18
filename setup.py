#
# This file is part of Gambit
# Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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

import Cython.Build
import setuptools

cppgambit_include_dirs = ["src"]
cppgambit_cflags = (
    ["-std=c++17"] if platform.system() == "Darwin"
    else ["/std:c++17"] if platform.system() == "Windows"
    else []
)

cppgambit_core = (
    "cppgambit_core",
    {
        "sources": glob.glob("src/core/*.cc"),
        "obj_deps": {"": glob.glob("src/core/*.h") + glob.glob("src/core/*.imp")},
        "include_dirs": cppgambit_include_dirs,
        "cflags": cppgambit_cflags,
    }
)

cppgambit_games = (
    "cppgambit_games",
    {
        "sources": glob.glob("src/games/*.cc") + glob.glob("src/games/agg/*.cc"),
        "obj_deps": {
            "": (
                glob.glob("src/core/*.h") + glob.glob("src/core/*.imp") +
                glob.glob("src/games/*.h") + glob.glob("src/games/*.imp") +
                glob.glob("src/games/agg/*.h")
            )
        },
        "include_dirs": cppgambit_include_dirs,
        "cflags": cppgambit_cflags,
    }
)


def solver_library_config(library_name: str, paths: list) -> tuple:
    return (
        library_name,
        {
            "sources": [fn for solver in paths for fn in glob.glob(f"src/solvers/{solver}/*.cc")],
            "obj_deps": {
                "": (
                    glob.glob("src/core/*.h") + glob.glob("src/games/*.h") +
                    glob.glob("src/games/agg/*.h") +
                    [fn for solver in paths for fn in glob.glob(f"src/solvers/{solver}/*.h")]
                )
            },
            "include_dirs": cppgambit_include_dirs,
            "cflags": cppgambit_cflags,
        }
    )


cppgambit_bimatrix = solver_library_config("cppgambit_bimatrix",
                                           ["linalg", "lp", "lcp", "enummixed"])
cppgambit_liap = solver_library_config("cppgambit_liap", ["liap"])
cppgambit_logit = solver_library_config("cppgambit_logit", ["logit"])
cppgambit_gtracer = solver_library_config("cppgambit_gtracer", ["gtracer", "ipa", "gnm"])
cppgambit_simpdiv = solver_library_config("cppgambit_simpdiv", ["simpdiv"])
cppgambit_enumpoly = solver_library_config("cppgambit_enumpoly", ["nashsupport", "enumpoly"])


libgambit = setuptools.Extension(
    "pygambit.gambit",
    sources=["src/pygambit/gambit.pyx"],
    language="c++",
    include_dirs=["src", "src/pygambit"],
    extra_compile_args=(
        ["-std=c++17"] if platform.system() == "Darwin"
        else ["/std:c++17"] if platform.system() == "Windows"
        else []
    )
)


def readme():
    with open("src/README.rst") as f:
        return f.read()


setuptools.setup(
    name="pygambit",
    version="16.3.1",
    description="The package for computation in game theory",
    long_description=readme(),
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: GNU General Public License v2 or later (GPLv2+)",
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
    url="https://www.gambit-project.org",
    project_urls={
        "Documentation": "https://gambitproject.readthedocs.io/",
        "Source": "https://github.com/gambitproject/gambit",
        "Tracker": "https://github.com/gambitproject/gambit/issues",
    },
    python_requires=">=3.9",
    install_requires=[
        "numpy",
        "scipy",
        "deprecated",
    ],
    libraries=[cppgambit_bimatrix, cppgambit_liap, cppgambit_logit, cppgambit_simpdiv,
               cppgambit_gtracer, cppgambit_enumpoly,
               cppgambit_games, cppgambit_core],
    package_dir={"": "src"},
    packages=["pygambit"],
    ext_modules=Cython.Build.cythonize(libgambit,
                                       language_level="3str",
                                       compiler_directives={"binding": True})
)
