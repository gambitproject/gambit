#
# This file is part of Gambit
# Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/outcome.pxi
# Cython wrapper for outcomes
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
from cython.operator cimport dereference as deref
from libcpp.memory cimport shared_ptr

import typing


@cython.cclass
class Outcome:
    """An outcome in a ``Game``."""
    outcome = cython.declare(c_GameOutcome)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create an Outcome outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(outcome: c_GameOutcome) -> Outcome:
        obj: Outcome = Outcome.__new__(Outcome)
        obj.outcome = outcome
        return obj

    def __repr__(self) -> str:
        if self.label:
            return f"Outcome(game={self.game}, label='{self.label}')"
        else:
            return f"Outcome(game={self.game}, number={self.number})"

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, Outcome) and
            self.outcome.deref() == cython.cast(Outcome, other).outcome.deref()
        )

    def __hash__(self) -> int:
        return cython.cast(cython.long, self.outcome.deref())

    @property
    def game(self) -> Game:
        """Returns the game with which this outcome is associated."""
        return Game.wrap(self.outcome.deref().GetGame())

    @property
    def label(self) -> str:
        """The text label associated with this outcome."""
        return self.outcome.deref().GetLabel().decode("ascii")

    @label.setter
    def label(self, value: str) -> None:
        if value in [i.label for i in self.game.outcomes]:
            warnings.warn("Another outcome with an identical label exists")
        self.outcome.deref().SetLabel(value.encode("ascii"))

    @property
    def number(self) -> int:
        """Returns the number of the outcome in the game.
        Outcomes are numbered starting with 0.
        """
        return self.outcome.deref().GetNumber() - 1

    def __getitem__(
            self, player: typing.Union[Player, str]
    ) -> typing.Union[decimal.Decimal, Rational]:
        """The payoff to `player` at the outcome.

        Raises
        ------
        MismatchError
            If `player` is a ``Player`` from a different game than the outcome.
        """
        resolved_player = cython.cast(Player,
                                      self.game._resolve_player(player, "Outcome.__getitem__"))
        payoff = (
            cython.cast(bytes,
                        self.outcome.deref().GetPayoff(resolved_player.player).as_string())
            .decode("ascii")
        )
        if "." in payoff:
            return decimal.Decimal(payoff)
        else:
            return Rational(payoff)

    def __setitem__(self, player: typing.Union[Player, str], value: typing.Any) -> None:
        """Set the payoff to `player` at the outcome.

        Parameters
        ----------
        player : Player or str
            A reference to the player for which to set the payoff.
        value : Any
            The value of the payoff.  This can be any numeric type, or any object that
            has a string representation which can be interpreted as a number.

        Raises
        ------
        MismatchError
            If `player` is a ``Player`` from a different game than the outcome.
        ValueError
            If `value` cannot be interpreted as a number.
        """
        resolved_player = cython.cast(Player,
                                      self.game._resolve_player(player, "Outcome.__setitem__"))
        self.outcome.deref().SetPayoff(resolved_player.player, _to_number(value))


@cython.cclass
class TreeGameOutcome:
    """Represents an outcome in a strategic game derived from an extensive game."""
    c_game = cython.declare(c_Game)
    psp = cython.declare(shared_ptr[c_PureStrategyProfile])

    def __init__(self):
        raise ValueError("Cannot create an Outcome outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(game: c_Game, psp: shared_ptr[c_PureStrategyProfile]) -> TreeGameOutcome:
        obj: TreeGameOutcome = TreeGameOutcome.__new__(TreeGameOutcome)
        obj.c_game = game
        obj.psp = psp
        return obj

    @property
    def game(self) -> Game:
        """Returns the game with which this outcome is associated."""
        return Game.wrap(self.c_game)

    def __repr__(self):
        return f"<Outcome '{self.label}' in game '{self.game.title}'>"

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, TreeGameOutcome) and
            deref(self.psp).deref() == deref(cython.cast(TreeGameOutcome, other).psp).deref()
        )

    def __getitem__(self, player: typing.Union[Player, str]) -> Rational:
        """The payoff to `player` at the outcome.

        Parameters
        ----------
        player : Player or str
            A reference to the player to get the payoff for

        Returns
        -------
        Rational
            The expected payoff to the player.  Because this is calculated in a derived
            strategic game, it will always be represented as a ``Rational`` even if
            game data are represented as ``Decimal``.

        Raises
        ------
        MismatchError
            If `player` is a ``Player`` from a different game than the outcome.
        """
        resolved_player = cython.cast(Player,
                                      self.game._resolve_player(player, "Outcome.__getitem__"))
        return rat_to_py(deref(self.psp).deref().GetPayoff(resolved_player.player))

    def delete(self):
        raise UndefinedOperationError("Cannot modify outcomes in a derived strategic game.")

    @property
    def label(self) -> str:
        """The text label associated with this outcome."""
        return "(%s)" % (
            ",".join(
                [deref(deref(self.psp).deref()).GetStrategy(cython.cast(Player, player).player)
                 .deref().GetLabel().c_str().decode()
                 for player in self.game.players]
            )
        )
