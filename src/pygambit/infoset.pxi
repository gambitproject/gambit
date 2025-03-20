#
# This file is part of Gambit
# Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/infoset.pxi
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
class InfosetMembers:
    """The set of nodes which are members of an information set."""
    infoset = cython.declare(c_GameInfoset)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create InfosetMembers outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(infoset: c_GameInfoset) -> InfosetMembers:
        obj: InfosetMembers = InfosetMembers.__new__(InfosetMembers)
        obj.infoset = infoset
        return obj

    def __repr__(self) -> str:
        return f"InfosetMembers(infoset={Infoset.wrap(self.infoset)})"

    def __len__(self) -> int:
        return self.infoset.deref().GetMembers().size()

    def __iter__(self) -> typing.Iterator[Node]:
        for member in self.infoset.deref().GetMembers():
            yield Node.wrap(member)

    def __getitem__(self, index: typing.Union[int, str]) -> Node:
        if isinstance(index, str):
            if not index.strip():
                raise ValueError("Node label cannot be empty or all whitespace")
            matches = [x for x in self if x.label == index.strip()]
            if not matches:
                raise KeyError(f"Infoset has no member with label '{index}'")
            if len(matches) > 1:
                raise ValueError(f"Infoset has multiple members with label '{index}'")
            return matches[0]
        if isinstance(index, int):
            return Node.wrap(self.infoset.deref().GetMember(index + 1))
        raise TypeError(f"Member index must be int or str, not {index.__class__.__name__}")


@cython.cclass
class InfosetActions:
    """The set of actions which are available at an information set."""
    infoset = cython.declare(c_GameInfoset)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create InfosetActions outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(infoset: c_GameInfoset) -> InfosetActions:
        obj: InfosetActions = InfosetActions.__new__(InfosetActions)
        obj.infoset = infoset
        return obj

    def __repr__(self) -> str:
        return f"InfosetActions(infoset={Infoset.wrap(self.infoset)})"

    def __len__(self):
        """The number of actions at the information set."""
        return self.infoset.deref().GetActions().size()

    def __iter__(self) -> typing.Iterator[Action]:
        for action in self.infoset.deref().GetActions():
            yield Action.wrap(action)

    def __getitem__(self, index: typing.Union[int, str]) -> Action:
        if isinstance(index, str):
            if not index.strip():
                raise ValueError("Action label cannot be empty or all whitespace")
            matches = [x for x in self if x.label == index.strip()]
            if not matches:
                raise KeyError(f"Infoset has no action with label '{index}'")
            if len(matches) > 1:
                raise ValueError(f"Infoset has multiple actions with label '{index}'")
            return matches[0]
        if isinstance(index, int):
            return Action.wrap(self.infoset.deref().GetAction(index + 1))
        raise TypeError(f"Action index must be int or str, not {index.__class__.__name__}")


@cython.cclass
class Infoset:
    """An information set in a ``Game``."""
    infoset = cython.declare(c_GameInfoset)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create an Infoset outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(infoset: c_GameInfoset) -> Infoset:
        obj: Infoset = Infoset.__new__(Infoset)
        obj.infoset = infoset
        return obj

    def __repr__(self) -> str:
        if self.label:
            return f"Infoset(player={self.player}, label='{self.label}')"
        else:
            return f"Infoset(player={self.player}, number={self.number})"

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, Infoset) and
            self.infoset.deref() == cython.cast(Infoset, other).infoset.deref()
        )

    def __hash__(self) -> int:
        return cython.cast(cython.long, self.infoset.deref())

    def precedes(self, node: Node) -> bool:
        """Return whether this information set precedes `node` in the game tree."""
        return self.infoset.deref().Precedes(cython.cast(Node, node).node)

    @property
    def game(self) -> Game:
        """The ``Game`` to which the information set belongs."""
        return Game.wrap(self.infoset.deref().GetGame())

    @property
    def label(self) -> str:
        """Get or set the text label of the information set."""
        return self.infoset.deref().GetLabel().decode("ascii")

    @label.setter
    def label(self, value: str) -> None:
        self.infoset.deref().SetLabel(value.encode("ascii"))

    @property
    def number(self) -> int:
        """Returns the number of the information set for its player.
        Information sets are numbered starting with 0.
        """
        return self.infoset.deref().GetNumber() - 1

    @property
    def is_chance(self) -> bool:
        """Whether the information set belongs to the chance player."""
        return self.infoset.deref().IsChanceInfoset()

    @property
    def actions(self) -> InfosetActions:
        """The set of actions at the information set."""
        return InfosetActions.wrap(self.infoset)

    @property
    def members(self) -> InfosetMembers:
        """The set of nodes which are members of the information set.

        The order in which members are iterated is dependent on the order of
        operations used to define the game.  A standard ordering, in which members
        are iterated in the order encountered in a depth-first traversal of the tree,
        can be obtained by calling `Game.sort_infosets` on the game after construction.

        .. versionchanged:: 16.4.0
           The ordering of members is now dependent on the order of operations;
           previously, members sets were (expensively) re-sorted after every change
           to the game tree.

        See also
        --------
        Game.sort_infosets
        """
        return InfosetMembers.wrap(self.infoset)

    @property
    def player(self) -> Player:
        """The player who has the move at this information set."""
        return Player.wrap(self.infoset.deref().GetPlayer())

    @property
    def plays(self) -> typing.List[Node]:
        """Returns a list of all terminal `Node` objects consistent with it.
        """
        return [
            Node.wrap(n) for n in self.infoset.deref().GetGame().deref().GetPlays(self.infoset)
        ]
