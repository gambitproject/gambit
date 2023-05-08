#
# This file is part of Gambit
# Copyright (c) 1994-2022, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/lib/infoset.pxi
# Cython wrapper for information sets
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

cdef class Members(Collection):
    """Represents a collection of members of an infoset."""
    cdef c_GameInfoset infoset

    def __len__(self):
        return self.infoset.deref().NumMembers()

    def __getitem__(self, i):
        if not isinstance(i, int):
            return Collection.__getitem__(self, i)
        cdef Node n
        n = Node()
        n.node = self.infoset.deref().GetMember(i+1)
        return n


cdef class Actions(Collection):
    """Represents a collection of actions at an infoset."""
    cdef c_GameInfoset infoset

    def add(self, action=None):
        if action is None:
            self.infoset.deref().InsertAction(<c_GameAction>NULL)
        elif isinstance(action, Action):
            if (<Infoset>action.infoset).infoset != self.infoset:
                raise MismatchError("The new action should be from the same infoset")
            self.infoset.deref().InsertAction((<Action>action).action)
        else:
            raise TypeError("insert_action takes an Action object as its input")

    def __len__(self):
        return self.infoset.deref().NumActions()

    def __getitem__(self, act):
        if not isinstance(act, int):
            return Collection.__getitem__(self, act)
        cdef Action a
        a = Action()
        a.action = self.infoset.deref().GetAction(act+1)
        return a

cdef class Infoset:
    cdef c_GameInfoset infoset

    def __repr__(self):
        return (
            f"<Infoset [{self.infoset.deref().GetNumber()-1}] '{self.label}' "
            f"for player '{self.player.label}' in game '{self.game.title}'>"
         )
    
    def __richcmp__(self, other, whichop):
        if isinstance(other, Infoset):
            if whichop == 2:
                return self.infoset.deref() == (<Infoset>other).infoset.deref()
            elif whichop == 3:
                return self.infoset.deref() != (<Infoset>other).infoset.deref()
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
        return long(<long>self.infoset.deref())

    def precedes(self, node: Node) -> bool:
        return self.infoset.deref().Precedes((<Node>node).node)

    def reveal(self, player: Player) -> None:
        self.infoset.deref().Reveal((<Player>player).player)
            
    property game:
        def __get__(self):
            cdef Game g
            g = Game()
            g.game = self.infoset.deref().GetGame()
            return g
        
    property label:
        def __get__(self):
            return self.infoset.deref().GetLabel().decode('ascii')
        def __set__(self, value: str):
            self.infoset.deref().SetLabel(value.encode('ascii'))

    property is_chance:
        def __get__(self) -> bool:
            return self.infoset.deref().IsChanceInfoset()

    property actions:
        def __get__(self):
            cdef Actions a
            a = Actions()
            a.infoset = self.infoset
            return a

    property members:
        def __get__(self):
            cdef Members m
            m = Members()
            m.infoset = self.infoset
            return m

    property player:
        def __get__(self):
            p = Player()
            p.player = self.infoset.deref().GetPlayer()
            return p

        def __set__(self, player: Player):
            if player.game != self.game:
                raise MismatchError("player at an infoset must belong to the same game")
            self.infoset.deref().SetPlayer((<Player>player).player)
