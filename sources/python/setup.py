#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Distutils script to build Gambit Python extension
#
# This file is part of Gambit
# Copyright (c) 2003, The Gambit Project
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

from distutils.core import setup, Extension

libs = [ 'nash', 'game', 'poly', 'pelican', 'numerical', 'math', 'base' ]

libdirs = [ '.', '../base', '../math', '../numerical',
            '../game', '../nash', '../poly', '../pelican' ]


ext = Extension('gambit', [ 'pyefg.cc',
                            'pygambit.cc', 'pymixed.cc',
                            'pynash.cc', 'pynfg.cc', 'pyplayer.cc' ],
                include_dirs=[ '..' ],
                library_dirs=libdirs,
                libraries=libs,
                extra_compile_args=['-O'],
                define_macros=[('VERSION', '"0.97.1.0"')])

setup(name = 'gambit',
      version = '0.97.1.0',
      description = 'Gambit',
      author = 'The Gambit Project',
      author_email = 'gambit@econmail.tamu.edu',
      url = 'http://econweb.tamu.edu/gambit',
      long_description = 'A Python interface to the Gambit API',
      ext_modules=[ext])


