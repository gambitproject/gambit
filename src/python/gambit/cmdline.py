#
# This file is part of Gambit
# Copyright (c) 1994-2016, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/cmdline.py
# Command-line programs for Gambit
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
"""
Command-line programs for Gambit
"""

from IPython.frontend.terminal.embed import InteractiveShellEmbed

def gambit_shell():
    """
    Start an ipython session after initializing the environment
    """
    import gambit
    import gambit.nash
    import gambit.qre
    
    # Everything in this dictionary will be added to the top-level
    # namespace in the shell.
    ns = { 'gambit': gambit, 'nash': gambit.nash, 'qre': gambit.qre }

    shell = InteractiveShellEmbed()
    shell.user_ns = ns
    shell()

    # Anything that should happen after the session terminates would go here

