#
# This file is part of Gambit
# Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/pygambit/strategy.pxi
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

@cython.cclass
class Strategy:
    """A plan of action for a ``Player`` in a ``Game``."""
    strategy = cython.declare(c_GameStrategy)

    def __repr__(self) -> str:
        if self.label:
            return f"Strategy(player={self.player}, label='{self.label}')"
        else:
            return f"Strategy(player={self.player}, number={self.number})"

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, Strategy) and
            self.strategy.deref() == cython.cast(Strategy, other).strategy.deref()
        )

    def __hash__(self) -> int:
        return cython.cast(cython.long, self.strategy.deref())

    @property
    def label(self) -> str:
        """Get or set the text label associated with the strategy."""
        return self.strategy.deref().GetLabel().decode("ascii")

    @label.setter
    def label(self, value: str) -> None:
        self.strategy.deref().SetLabel(value.encode("ascii"))

    @property
    def game(self) -> Game:
        """The game to which the strategy belongs."""
        g = Game()
        g.game = self.strategy.deref().GetPlayer().deref().GetGame()
        return g

    @property
    def player(self) -> Player:
        """The player to which the strategy belongs."""
        p = Player()
        p.player = self.strategy.deref().GetPlayer()
        return p

    @property
    def number(self) -> int:
        """The number of the strategy."""
        return self.strategy.deref().GetNumber() - 1
