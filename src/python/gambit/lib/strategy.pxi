#
# This file is part of Gambit
# Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/lib/strategy.pxi
# Cython wrapper for strategies
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
from gambit.lib.error import UndefinedOperationError

cdef class Strategy:
    cdef c_GameStrategy strategy
    cdef StrategicRestriction restriction

    def __repr__(self):
        return "<Strategy [%d] '%s' for player '%s' in game '%s'>" % \
                (self.number, self.label,
                 self.player.label,
                 self.player.game.title)
    
    def __richcmp__(Strategy self, other, whichop):
        if isinstance(other, Strategy):
            if whichop == 2:
                return self.strategy.deref() == ((<Strategy>other).strategy).deref()
            elif whichop == 3:
                return self.strategy.deref() != ((<Strategy>other).strategy).deref()
            else:
                raise NotImplementedError
        else:
            if whichop == 2:
                return False
            elif whichop == 3:
                return True
            else:
                raise NotImplementedError

    def __hash__(self):
        return long(<long>self.strategy.deref())

    property label:
        def __get__(self):
            return self.strategy.deref().GetLabel().c_str()
        def __set__(self, char *value):
            if self.restriction is not None:
                raise UndefinedOperationError("Changing objects in a restriction is not supported")
            if value in [ i.label for i in self.player.strategies ]:
                warnings.warn("This player has another strategy with an identical label")
            cdef cxx_string s
            s.assign(value)
            self.strategy.deref().SetLabel(s)

    property player:
        def __get__(self):
            p = Player()
            p.restriction = self.restriction
            p.player = self.strategy.deref().GetPlayer()
            return p

    property number:
        def __get__(self):
            return self.strategy.deref().GetNumber() - 1 

    def unrestrict(self):
        cdef Strategy s
        s = Strategy()
        s.strategy = self.strategy
        return s
