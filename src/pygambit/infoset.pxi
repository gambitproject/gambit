#
# This file is part of Gambit
# Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
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

@cython.cclass
class Members(Collection):
    """The set of nodes which are members of an information set."""
    infoset = cython.declare(c_GameInfoset)

    def __init__(self, infoset: Infoset):
        self.infoset = infoset.infoset

    def __len__(self):
        return self.infoset.deref().NumMembers()

    def __getitem__(self, i):
        if not isinstance(i, int):
            return Collection.__getitem__(self, i)
        n = Node()
        n.node = self.infoset.deref().GetMember(i+1)
        return n


@cython.cclass
class Actions(Collection):
    """The set of actions which are available at an information set."""
    infoset = cython.declare(c_GameInfoset)

    def __len__(self):
        """The number of actions at the information set."""
        return self.infoset.deref().NumActions()

    def __getitem__(self, act):
        if not isinstance(act, int):
            return Collection.__getitem__(self, act)
        a = Action()
        a.action = self.infoset.deref().GetAction(act+1)
        return a


@cython.cclass
class Infoset:
    """An information set in a ``Game``."""
    infoset = cython.declare(c_GameInfoset)

    def __repr__(self) -> str:
        return (
            f"<Infoset [{self.infoset.deref().GetNumber()-1}] '{self.label}' "
            f"for player '{self.player.label}' in game '{self.game.title}'>"
         )

    def __eq__(self, other: typing.Any) -> bool:
        return isinstance(other, Infoset) and self.infoset.deref() == cython.cast(Infoset, other).infoset.deref()

    def __hash__(self) -> int:
        return cython.cast(cython.long, self.infoset.deref())

    def precedes(self, node: Node) -> bool:
        """Return whether this information set precedes `node` in the game tree."""
        return self.infoset.deref().Precedes(cython.cast(Node, node).node)

    @property
    def game(self) -> Game:
        """The ``Game`` to which the information set belongs."""
        g = Game()
        g.game = self.infoset.deref().GetGame()
        return g

    @property
    def label(self) -> str:
        """Get or set the text label of the information set."""
        return self.infoset.deref().GetLabel().decode('ascii')

    @label.setter
    def label(self, value: str) -> None:
        self.infoset.deref().SetLabel(value.encode('ascii'))

    @property
    def is_chance(self) -> bool:
        """Whether the information set belongs to the chance player."""
        return self.infoset.deref().IsChanceInfoset()

    @property
    def actions(self) -> Actions:
        """The set of actions at the information set."""
        a = Actions()
        a.infoset = self.infoset
        return a

    @property
    def members(self) -> Members:
        """The set of nodes which are members of the information set."""
        return Members(self)

    @property
    def player(self) -> Player:
        """The player who has the move at this information set."""
        p = Player()
        p.player = self.infoset.deref().GetPlayer()
        return p
