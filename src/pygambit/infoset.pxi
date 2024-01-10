#
# This file is part of Gambit
# Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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
from deprecated import deprecated

@cython.cclass
class InfosetMembers:
    """The set of nodes which are members of an information set."""
    infoset = cython.declare(c_GameInfoset)

    def __init__(self, infoset: Infoset):
        self.infoset = infoset.infoset

    def __len__(self) -> int:
        return self.infoset.deref().NumMembers()

    def __iter__(self) -> typing.Iterator[Node]:
        for i in range(self.infoset.deref().NumMembers()):
            m = Node()
            m.node = self.infoset.deref().GetMember(i + 1)
            yield m

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
            m = Node()
            m.node = self.infoset.deref().GetMember(index + 1)
            return m
        raise TypeError(f"Member index must be int or str, not {index.__class__.__name__}")


@cython.cclass
class InfosetActions:
    """The set of actions which are available at an information set."""
    infoset = cython.declare(c_GameInfoset)

    @deprecated(version='16.1.0',
                reason='Use Game.add_action instead of Actions.add.',
                category=FutureWarning)
    def add(self, action=None):
        """Add an action at the information set.  If `action` is not null, the new action
        is inserted before `action`.

        .. deprecated:: 16.1.0
           Use `Game.add_action` instead of `Actions.add`.
        """
        if action is None:
            self.infoset.deref().InsertAction(cython.cast(c_GameAction, NULL))
        elif isinstance(action, Action):
            if cython.cast(Infoset, action.infoset).infoset != self.infoset:
                raise MismatchError("The new action should be from the same infoset")
            self.infoset.deref().InsertAction(cython.cast(Action, action).action)
        else:
            raise TypeError("insert_action takes an Action object as its input")

    def __len__(self) -> int:
        """The number of actions at the information set."""
        return self.infoset.deref().NumActions()

    def __iter__(self) -> typing.Iterator[Action]:
        for i in range(self.infoset.deref().NumActions()):
            a = Action()
            a.action = self.infoset.deref().GetAction(i + 1)
            yield a

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
            a = Action()
            a.action = self.infoset.deref().GetAction(index + 1)
            return a
        raise TypeError(f"Action index must be int or str, not {index.__class__.__name__}")


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

    @deprecated(version='16.1.0',
                reason='Use Game.reveal instead of Infoset.game.',
                category=FutureWarning)
    def reveal(self, player: Player) -> None:
        """Reveal the move made at the information set to `player`.

        Revealing the move modifies all subsequent information sets for `player` such
        that any two nodes which are successors of two different actions at this
        information set are placed in different information sets for `player`.

        Revelation is a one-shot operation; it is not enforced with respect to any
        revisions made to the game tree subsequently.

        .. deprecated:: 16.1.0
           Use `Game.reveal` instead of `Infoset.reveal`.

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
        self.infoset.deref().Reveal(cython.cast(Player, player).player)

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
    def actions(self) -> InfosetActions:
        """The set of actions at the information set."""
        a = InfosetActions()
        a.infoset = self.infoset
        return a

    @property
    def members(self) -> InfosetMembers:
        """The set of nodes which are members of the information set."""
        return InfosetMembers(self)

    @property
    def player(self) -> Player:
        """The player who has the move at this information set."""
        p = Player()
        p.player = self.infoset.deref().GetPlayer()
        return p
