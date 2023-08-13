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
    """Represents an information set in a :py:class:`Game`."""
    cdef c_GameInfoset infoset

    def __repr__(self) -> str:
        return (
            f"<Infoset [{self.infoset.deref().GetNumber()-1}] '{self.label}' "
            f"for player '{self.player.label}' in game '{self.game.title}'>"
         )
    
    def __richcmp__(Infoset self, other, whichop) -> bool:
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

    def __hash__(self) -> long:
        return long(<long>self.infoset.deref())

    def precedes(self, node: Node) -> bool:
        return self.infoset.deref().Precedes((<Node>node).node)

    def reveal(self, player: Player) -> None:
        """Reveals the move made at the information set to `player`.

        Revealing the move modifies all subsequent information sets for `player` such
        that any two nodes which are successors of two different actions at this
        information set are placed in different information sets for `player`.

        Revelation is a one-shot operation; it is not enforced with respect to any
        revisions made to the game tree subsequently.

        Parameters
        ----------
        player : Player
            The player to which to reveal the move at this information set.

        Raises
        ------
        TypeError
            on passing an object that is not a :py:class:`Player`.
        MismatchError
            on revealing to a player that is from a different game.
        """
        if not isinstance(player, Player):
            raise TypeError(
                f"player must be of type Player, not {player.__class__.__name__}"
            )
        if player.game != self.game:
            raise MismatchError(
                "player must belong to the same game as the information set"
            )
        self.infoset.deref().Reveal((<Player>player).player)

    @property
    def game(self) -> Game:
        """Gets the :py:class:`Game` to which the information set belongs."""
        cdef Game g
        g = Game()
        g.game = self.infoset.deref().GetGame()
        return g

    @property
    def label(self) -> str:
        """Gets or sets the text label of the information set."""
        return self.infoset.deref().GetLabel().decode('ascii')

    @label.setter
    def label(self, value: str) -> None:
        self.infoset.deref().SetLabel(value.encode('ascii'))

    @property
    def is_chance(self) -> bool:
        """Returns `True` if the information set belongs to the chance player."""
        return self.infoset.deref().IsChanceInfoset()

    @property
    def actions(self) -> Actions:
        """Returns the set of actions at the information set."""
        cdef Actions a
        a = Actions()
        a.infoset = self.infoset
        return a

    @property
    def members(self) -> Members:
        """Returns the set of nodes which are members of the information set."""
        cdef Members m
        m = Members()
        m.infoset = self.infoset
        return m

    @property
    def player(self) -> Player:
        """Gets or sets the player who has the move at this information set.

        Raises
        ------
        TypeError
            on setting to an object that is not a :py:class:`Player`.
        MismatchError
            on setting to a player that is from a different game.
        """
        p = Player()
        p.player = self.infoset.deref().GetPlayer()
        return p

    @player.setter
    def player(self, player: Player) -> None:
        if not isinstance(player, Player):
            raise TypeError(
                f"player must be of type Player, not {player.__class__.__name__}"
            )
        elif player.game != self.game:
            raise MismatchError(
                "player must belong to the same game as the information set"
            )
        self.infoset.deref().SetPlayer((<Player>player).player)
