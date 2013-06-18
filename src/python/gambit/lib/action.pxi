#
# This file is part of Gambit
# Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/lib/action.pxi
# Cython wrapper for actions
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

cdef class Action:
    cdef c_GameAction action

    def __repr__(self):
         return "<Action [%d] '%s' at infoset '%s' for player '%s' in game '%s'>" % \
                (self.action.deref().GetNumber()-1, self.label,
                 self.action.deref().GetInfoset().deref().GetLabel().c_str(),
                 self.action.deref().GetInfoset().deref().GetPlayer().deref().GetLabel().c_str(),
                 self.action.deref().GetInfoset().deref().GetPlayer().deref().GetGame().deref().GetTitle().c_str())
    
    def __richcmp__(Action self, other, whichop):
        if isinstance(other, Action):
            if whichop == 2:
                return self.action.deref() == ((<Action>other).action).deref()
            elif whichop == 3:
                return self.action.deref() != ((<Action>other).action).deref()
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
        return long(<long>self.action.deref())

    def delete(self):
        if len(self.infoset.actions) == 1:
            raise UndefinedOperationError("it is not possible to delete the \
                                            last action of an infoset")
        self.action.deref().DeleteAction()

    def precedes(self, node):
        if isinstance(node, Node):
            return self.action.deref().Precedes(((<Node>node).node))
        else:
            raise TypeError("Precedes takes a Node object as its input")

    property label:
        def __get__(self):
            return self.action.deref().GetLabel().c_str()
        def __set__(self, char *value):
            cdef cxx_string s
            s.assign(value)
            self.action.deref().SetLabel(s)

    property infoset:
        def __get__(self):
            cdef Infoset i
            i = Infoset()
            i.infoset = self.action.deref().GetInfoset()
            return i

    property prob:
        def __get__(self):
            cdef bytes py_string
            cdef cxx_string dummy_str
            py_string = self.action.deref().GetInfoset().deref().GetActionProb(
                self.action.deref().GetNumber(), dummy_str).c_str()
            if "." in py_string:
                return decimal.Decimal(py_string)
            else:
                return fractions.Fraction(py_string)
        
        def __set__(self, value):
            cdef cxx_string s
            if isinstance(value, (int, decimal.Decimal, fractions.Fraction)):
                v = str(value)
                s.assign(v)
                self.action.deref().GetInfoset().deref().SetActionProb(
                    self.action.deref().GetNumber(), s)
            else:
                raise TypeError, "numeric argument required for action " \
                                 "probability"
            