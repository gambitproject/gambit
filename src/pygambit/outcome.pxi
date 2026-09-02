#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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
        if self.outcome.deref().IsNull():
            return f"Outcome(game={self.game}, label=None)"
        if self.label:
            return f"Outcome(game={self.game}, label='{self.label}')"
        else:
            return f"Outcome(game={self.game}, number={self.number})"

    def __eq__(self, other: typing.Any):
        if not isinstance(other, Outcome):
            return NotImplemented
        if (self.outcome.deref().IsNull()
                or cython.cast(Outcome, other).outcome.deref().IsNull()):
            # Null outcomes are not equal to anything, including themselves (cf. nan).
            return False
        return self.outcome.deref() == cython.cast(Outcome, other).outcome.deref()

    def __hash__(self) -> int:
        return cython.cast(cython.long, self.outcome.deref())

    def __bool__(self) -> bool:
        """``True`` for a real outcome; the null outcome is falsy."""
        return not self.outcome.deref().IsNull()

    @property
    def game(self) -> Game:
        """Returns the game with which this outcome is associated."""
        return Game.wrap(self.outcome.deref().GetGame())

    @property
    def label(self) -> str | None:
        """The text label associated with this outcome.

        The null outcome's label is ``None``; testing ``outcome.label is None`` is the
        idiomatic nullity check.

        .. versionchanged:: 16.7.0
            An outcome label must be nonempty and unique within the game; an empty or duplicate
            label now raises ``ValueError``.

        .. versionchanged:: 17.0.0
            A label may now be any well-formed UTF-8 text, not just ASCII; it must still
            contain no control characters, and must not begin/end with whitespace or have
            two consecutive whitespace characters.  "Whitespace" means any Unicode space
            separator (e.g. U+00A0 NO-BREAK SPACE), not just the ASCII space.
            The null outcome resolves with label ``None``.
        """
        if self.outcome.deref().IsNull():
            return None
        return self.outcome.deref().GetLabel().decode("utf-8")

    @label.setter
    def label(self, value: str) -> None:
        self.outcome.deref().SetLabel(value.encode("utf-8"))

    @property
    def number(self) -> int | None:
        """Returns the number of the outcome in the game.
        Outcomes are numbered starting with 0.

        The null outcome is not a member of the game's outcomes, so it has no number.

        .. versionchanged:: 17.0.0
            The null outcome resolves here with number ``None``.
        """
        if self.outcome.deref().IsNull():
            return None
        return self.outcome.deref().GetNumber() - 1

    def __getitem__(
            self, player: str
    ) -> decimal.Decimal | Rational:
        """The payoff to `player` at the outcome.

        The null outcome reports a zero payoff to every player of its game.

        Raises
        ------
        KeyError
            If no player of the outcome's game has label `player`.
        """
        game: Game = self.game
        resolved_player: c_GamePlayer = game._resolve_player(player, "Outcome.__getitem__")
        payoff = (
            self.outcome.deref().GetPayoff[string](resolved_player).decode("ascii")
        )
        if "." in payoff:
            return decimal.Decimal(payoff)
        else:
            return Rational(payoff)

    def __setitem__(self, player: str, value: typing.Any) -> None:
        """Set the payoff to `player` at the outcome.

        Parameters
        ----------
        player : str
            The label of the player for which to set the payoff.
        value : Any
            The value of the payoff.  This can be any numeric type, or any object that
            has a string representation which can be interpreted as a number.

        Raises
        ------
        KeyError
            If no player of the outcome's game has label `player`.
        ValueError
            If `value` cannot be interpreted as a number.
        UndefinedOperationError
            If this is the null outcome; payoffs cannot be set on it.
        """
        if self.outcome.deref().IsNull():
            raise UndefinedOperationError(
                "Payoffs cannot be set on the null outcome; "
                "use Game.make_outcome to create and attach an outcome"
            )
        game: Game = self.game
        resolved_player: c_GamePlayer = game._resolve_player(player, "Outcome.__setitem__")
        self.outcome.deref().SetPayoff(resolved_player, _to_number(value))
