#
# This file is part of Gambit
# Copyright (c) 1994-2022, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/lib/action.pxi
# Cython wrapper for actions
#
# This program is free software; you can redistribute it and/or modify
# It under the terms of the GNU General Public License as published by
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
from libcpp.string cimport string

cdef class Action:
    cdef c_GameAction action

    def __repr__(self):
        return (
            f"<Action [{self.action.deref().GetNumber()-1}] '{self.label}' "
            f"at infoset '{self.infoset.label}' "
            f"for player '{self.infoset.player.label}' "
            f"in game '{self.game.title}'>"
        )
    
    def __richcmp__(Action self, other, whichop):
        if isinstance(other, Action):
            if whichop == 2:
                return self.action.deref() == (<Action>other).action.deref()
            elif whichop == 3:
                return self.action.deref() != (<Action>other).action.deref()
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
            raise UndefinedOperationError(
                "It is not possible to delete the last action of an infoset"
            )
        self.action.deref().DeleteAction()

    def precedes(self, node):
        if isinstance(node, Node):
            return self.action.deref().Precedes((<Node>node).node)
        else:
            raise TypeError("Precedes takes a Node object as its input")

    property label:
        def __get__(self):
            return self.action.deref().GetLabel().decode('ascii')
        def __set__(self, value):
            cdef string s
            s = value.encode('ascii')
            self.action.deref().SetLabel(s)

    property infoset:
        def __get__(self):
            cdef Infoset i
            i = Infoset()
            i.infoset = self.action.deref().GetInfoset()
            return i

    property prob:
        def __get__(self):
            cdef string py_string
            py_string = <string>(
                self.action.deref().GetInfoset().deref().GetActionProb(
                    self.action.deref().GetNumber()
                )
            )
            if "." in py_string.decode('ascii'):
                return decimal.Decimal(py_string.decode('ascii'))
            else:
                return Rational(py_string.decode('ascii'))
        
        def __set__(self, value: typing.Any):
            """
            Set the probability a chance action is played.

            Parameters
            ----------
            value : Any
                The probability the action is played.  This can be any numeric type, or any object that
                has a string representation which can be interpreted as an integer,
                decimal, or rational number.
            """
            self.action.deref().GetInfoset().deref().SetActionProb(
                self.action.deref().GetNumber(), _to_number(value)
            )
