#
# This file is part of Gambit
# Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/lib/player.pxi
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
from deprecated import deprecated

@cython.cclass
class Infosets(Collection):
    """The set of information sets at which a player has the decision."""
    player = cython.declare(c_GamePlayer)

    def __len__(self):
        """The number of information sets at which the player has the decision."""
        return self.player.deref().NumInfosets()

    def __getitem__(self, iset):
        if not isinstance(iset, int):
            return Collection.__getitem__(self, iset)
        s = Infoset()
        s.infoset = self.player.deref().GetInfoset(iset+1)
        return s


@cython.cclass
class Strategies(Collection):
    """The set of strategies available to a player."""
    player = cython.declare(c_GamePlayer)

    @deprecated(version='16.1.0',
                reason='Use Game.add_strategy() instead of Player.strategies.add()',
                category=FutureWarning)
    def add(self, label="") -> Strategy:
        """Add a new strategy to the set of the player's strategies.

        Returns
        -------
        The newly-created strategy

        Raises
        ------
        TypeError
            If called on a game which has an extensive representation.
        """
        g = Game()
        g.game = self.player.deref().GetGame()
        if g.is_tree:
            raise TypeError("Adding strategies is only applicable to games in strategic form")
        s = Strategy()
        s.strategy = self.player.deref().NewStrategy()
        s.label = str(label)
        return s

    def __len__(self):
        """The number of strategies for the player in the game."""
        return self.player.deref().NumStrategies()

    def __getitem__(self, st):
        if not isinstance(st, int):
            return Collection.__getitem__(self, st)
        s = Strategy()
        s.strategy = self.player.deref().GetStrategy(st+1)
        return s


@cython.cclass
class Player:
    """A player in a ``Game``."""
    player = cython.declare(c_GamePlayer)

    def __repr__(self):
        if self.is_chance:
            return f"<Player [CHANCE] in game '{self.game.title}'>"
        else:
            return (
                f"<Player [{self.number}] '{self.label}' "
                f"in game '{self.game.title}'>"
            )

    def __eq__(self, other: typing.Any) -> bool:
        return isinstance(other, Player) and self.player.deref() == cython.cast(Player, other).player.deref()

    def __hash__(self) -> int:
        return cython.cast(cython.long, self.player.deref())

    @property
    def game(self) -> Game:
        """Gets the ``Game`` to which the player belongs."""
        g = Game()
        g.game = self.player.deref().GetGame()
        return g

    @property
    def label(self) -> str:
        """Gets or sets the text label of the player."""
        return self.player.deref().GetLabel().decode('ascii')

    @label.setter
    def label(self, value: str) -> None:
        # check to see if the player's name has been used elsewhere
        if value in [i.label for i in self.game.players]:
            warnings.warn("Another player with an identical label exists")
        self.player.deref().SetLabel(value.encode('ascii'))

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
    def strategies(self) -> Strategies:
        """Returns the set of strategies belonging to the player."""
        s = Strategies()
        s.player = self.player
        return s

    @property
    def infosets(self) -> Infosets:
        """Returns the set of information sets at which the player has the decision."""
        s = Infosets()
        s.player = self.player
        return s

    @property
    def min_payoff(self) -> Rational:
        """Returns the smallest payoff for the player in any outcome of the game."""
        g = Game()
        g.game = self.player.deref().GetGame()
        return rat_to_py(g.game.deref().GetMinPayoff(self.number + 1))

    @property
    def max_payoff(self) -> Rational:
        """Returns the largest payoff for the player in any outcome of the game."""
        g = Game()
        g.game = self.player.deref().GetGame()
        return rat_to_py(g.game.deref().GetMaxPayoff(self.number + 1))
