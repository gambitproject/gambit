#
# This file is part of Gambit
# Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/player.pxi
# Cython wrapper for players
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
import cython


@cython.cclass
class PlayerInfosets:
    """The set of information sets at which a player has the decision."""
    player = cython.declare(c_GamePlayer)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create PlayerInfosets outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(player: c_GamePlayer) -> PlayerInfosets:
        obj: PlayerInfosets = PlayerInfosets.__new__(PlayerInfosets)
        obj.player = player
        return obj

    def __repr__(self) -> str:
        return f"PlayerInfosets(player={Player.wrap(self.player)})"

    def __len__(self) -> int:
        """The number of information sets at which the player has the decision."""
        return self.player.deref().NumInfosets()

    def __iter__(self) -> typing.Iterator[Infoset]:
        for i in range(self.player.deref().NumInfosets()):
            yield Infoset.wrap(self.player.deref().GetInfoset(i + 1))

    def __getitem__(self, index: typing.Union[int, str]) -> Infoset:
        if isinstance(index, str):
            if not index.strip():
                raise ValueError("Infoset label cannot be empty or all whitespace")
            matches = [x for x in self if x.label == index.strip()]
            if not matches:
                raise KeyError(f"Player has no infoset with label '{index}'")
            if len(matches) > 1:
                raise ValueError(f"Player has multiple infosets with label '{index}'")
            return matches[0]
        if isinstance(index, int):
            return Infoset.wrap(self.player.deref().GetInfoset(index + 1))
        raise TypeError(f"Infoset index must be int or str, not {index.__class__.__name__}")


@cython.cclass
class PlayerActions:
    """Represents the set of all actions available to a player at some information set."""
    player = cython.declare(Player)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create PlayerActions outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(player: Player) -> PlayerActions:
        obj: PlayerActions = PlayerActions.__new__(PlayerActions)
        obj.player = player
        return obj

    def __repr__(self) -> str:
        return f"PlayerActions(player={self.player})"

    def __len__(self) -> int:
        return sum(len(s.actions) for s in self.player.actions)

    def __iter__(self) -> typing.Iterator[Action]:
        for infoset in self.player.infosets:
            yield from infoset.actions

    def __getitem__(self, index: typing.Union[int, str]) -> Action:
        if isinstance(index, str):
            if not index.strip():
                raise ValueError("Action label cannot be empty or all whitespace")
            matches = [x for x in self if x.label == index.strip()]
            if not matches:
                raise KeyError(f"Player has no action with label '{index}'")
            if len(matches) > 1:
                raise ValueError(f"Player has multiple actions with label '{index}'")
            return matches[0]
        if isinstance(index, int):
            for i, action in enumerate(self):
                if i == index:
                    return action
            else:
                raise IndexError("Index out of range")
        raise TypeError(f"Action index must be int or str, not {index.__class__.__name__}")


@cython.cclass
class PlayerStrategies:
    """The set of strategies available to a player."""
    player = cython.declare(c_GamePlayer)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create PlayerStrategies outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(player: c_GamePlayer) -> PlayerStrategies:
        obj: PlayerStrategies = PlayerStrategies.__new__(PlayerStrategies)
        obj.player = player
        return obj

    def __repr__(self) -> str:
        return f"PlayerStrategies(player={Player.wrap(self.player)})"

    def __len__(self):
        """The number of strategies for the player in the game."""
        return self.player.deref().NumStrategies()

    def __iter__(self) -> typing.Iterator[Strategy]:
        for i in range(self.player.deref().NumStrategies()):
            yield Strategy.wrap(self.player.deref().GetStrategy(i + 1))

    def __getitem__(self, index: typing.Union[int, str]) -> Strategy:
        if isinstance(index, str):
            if not index.strip():
                raise ValueError("Strategy label cannot be empty or all whitespace")
            matches = [x for x in self if x.label == index.strip()]
            if not matches:
                raise KeyError(f"Player has no strategy with label '{index}'")
            if len(matches) > 1:
                raise ValueError(f"Player has multiple strategies with label '{index}'")
            return matches[0]
        if isinstance(index, int):
            return Strategy.wrap(self.player.deref().GetStrategy(index + 1))
        raise TypeError(f"Strategy index must be int or str, not {index.__class__.__name__}")


@cython.cclass
class Player:
    """A player in a ``Game``."""
    player = cython.declare(c_GamePlayer)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a Player outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(player: c_GamePlayer) -> Player:
        obj: Player = Player.__new__(Player)
        obj.player = player
        return obj

    def __repr__(self) -> str:
        if self.is_chance:
            return f"ChancePlayer(game={self.game})"
        if self.label:
            return f"Player(game={self.game}, label='{self.label}')"
        else:
            return f"Player(game={self.game}, number={self.number})"

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, Player) and
            self.player.deref() == cython.cast(Player, other).player.deref()
        )

    def __hash__(self) -> int:
        return cython.cast(cython.long, self.player.deref())

    @property
    def game(self) -> Game:
        """Gets the ``Game`` to which the player belongs."""
        return Game.wrap(self.player.deref().GetGame())

    @property
    def label(self) -> str:
        """Gets or sets the text label of the player."""
        return self.player.deref().GetLabel().decode("ascii")

    @label.setter
    def label(self, value: str) -> None:
        # check to see if the player's name has been used elsewhere
        if value in [i.label for i in self.game.players]:
            warnings.warn("Another player with an identical label exists")
        self.player.deref().SetLabel(value.encode("ascii"))

    @property
    def number(self) -> int:
        """Returns the number of the player in its game.
        Players are numbered starting with 0.
        """
        return self.player.deref().GetNumber() - 1

    @property
    def is_chance(self) -> bool:
        """Returns whether the player is the chance player."""
        return self.player.deref().IsChance() != 0

    @property
    def strategies(self) -> PlayerStrategies:
        """Returns the set of strategies belonging to the player."""
        return PlayerStrategies.wrap(self.player)

    @property
    def infosets(self) -> PlayerInfosets:
        """Returns the set of information sets at which the player has the decision."""
        return PlayerInfosets.wrap(self.player)

    @property
    def actions(self) -> PlayerActions:
        """Returns the set of actions available to the player at some information set."""
        return PlayerActions.wrap(self)

    @property
    def min_payoff(self) -> Rational:
        """Returns the smallest payoff for the player in any outcome of the game."""
        return rat_to_py(self.player.deref().GetGame().deref().GetMinPayoff(self.number + 1))

    @property
    def max_payoff(self) -> Rational:
        """Returns the largest payoff for the player in any outcome of the game."""
        return rat_to_py(self.player.deref().GetGame().deref().GetMaxPayoff(self.number + 1))
