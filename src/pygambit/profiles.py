#
# This file is part of Gambit
# Copyright (c) 1994-2016, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/profiles.py
# Base classes for strategy profiles
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
Base classes for strategy profiles.
"""

class Solution(object):
    """
    Generic object representing a strategy profile which is
    (part of) a solution of a game.
    """
    def __init__(self, profile):  self._profile = profile
    def __len__(self):            return len(self._profile)
    def __getitem__(self, i):     return self._profile[i]
    def __setitem__(self, i, v):
        raise TypeError("solution profile object does not support probability assignment")
    def __getattr__(self, attr):  return getattr(self._profile, attr)
