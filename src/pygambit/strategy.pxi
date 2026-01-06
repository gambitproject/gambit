#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a Strategy outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(strategy: c_GameStrategy) -> Strategy:
        obj: Strategy = Strategy.__new__(Strategy)
        obj.strategy = strategy
        return obj

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
        if value == self.label:
            return
        if value == "" or value in (strategy.label for strategy in self.player.strategies):
            warnings.warn("In a future version, strategies for a player must have unique labels",
                          FutureWarning)
        self.strategy.deref().SetLabel(value.encode("ascii"))

    @property
    def game(self) -> Game:
        """The game to which the strategy belongs."""
        return Game.wrap(self.strategy.deref().GetPlayer().deref().GetGame())

    @property
    def player(self) -> Player:
        """The player to which the strategy belongs."""
        return Player.wrap(self.strategy.deref().GetPlayer())

    @property
    def number(self) -> int:
        """The number of the strategy."""
        return self.strategy.deref().GetNumber() - 1

    def action(self, infoset: Infoset | str) -> Action | None:
        """Get the action prescribed by a strategy for a given information set.

        .. versionadded:: 16.4.0

        Parameters
        ----------
        infoset
            The information set for which to find the prescribed action.
            Can be an Infoset object or its string label.

        Returns
        -------
        Action or None
            The prescribed action or None if the strategy is not defined for this
            information set, that is, the information set is unreachable under this strategy.

        Raises
        ------
        UndefinedOperationError
            If the game is not an extensive-form (tree) game.
        ValueError
            If the information set belongs to a different player than the strategy.
        """
        if not self.game.is_tree:
            raise UndefinedOperationError(
                "Strategy.action is only defined for strategies in extensive-form games."
            )

        resolved_infoset: Infoset = self.game._resolve_infoset(infoset, "Strategy.action")

        if resolved_infoset.player != self.player:
            raise ValueError(
                f"Information set {resolved_infoset} belongs to player "
                f"'{resolved_infoset.player.label}', but this strategy "
                f"belongs to player '{self.player.label}'."
            )

        action: c_GameAction = self.strategy.deref().GetAction(resolved_infoset.infoset)
        if not action:
            return None
        return Action.wrap(action)
