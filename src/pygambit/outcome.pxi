#
# This file is part of Gambit
# Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/lib/outcome.pxi
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
from cython.operator cimport dereference as deref
from libcpp.memory cimport shared_ptr

@cython.cclass
class Outcome:
    """An outcome in a `Game`."""
    outcome = cython.declare(c_GameOutcome)

    def __repr__(self):
        return (
            f"<Outcome [{self.outcome.deref().GetNumber()-1}] '{self.label}' "
            f"in game '{self.game.title}'>"
        )
    
    def __eq__(self, other: typing.Any) -> bool:
        return isinstance(other, Outcome) and self.outcome.deref() == (<Outcome> other).outcome.deref()

    def __ne__(self, other: typing.Any) -> bool:
        return not isinstance(other, Outcome) or self.outcome.deref() != (<Outcome> other).outcome.deref()

    def __hash__(self) -> int:
        return cython.cast(cython.long, self.outcome.deref())

    def delete(self):
        """Deletes the outcome from its game.  If the game is an extensive game, any
        node at which this outcome is attached has its outcome reset to null.  If this game
        is a strategic game, any contingency at which this outcome is attached as its outcome
        reset to null.
        """
        cython.cast(Game, self.game).game.deref().DeleteOutcome(self.outcome)

    @property
    def game(self) -> Game:
        """Returns the game with which this outcome is associated."""
        g = Game()
        g.game = self.outcome.deref().GetGame()
        return g

    @property
    def label(self) -> str:
        """The text label associated with this outcome."""
        return self.outcome.deref().GetLabel().decode('ascii')

    @label.setter
    def label(self, value: str) -> None:
        if value in [i.label for i in self.game.outcomes]:
            warnings.warn("Another outcome with an identical label exists")
        self.outcome.deref().SetLabel(value.encode('ascii'))

    def __getitem__(self, player):
        """Returns the payoff to ``player`` at the outcome.  ``player``
        may be a `Player`, a string, or an integer.
        If a string, returns the payoff to the player with that string
        as its label.  If an integer, returns the payoff to player
        number ``player``.
        """
        py_string = cython.declare(bytes)
        if isinstance(player, Player):
            py_string = self.outcome.deref().GetPayoff(player.number+1).as_string().c_str()
        elif isinstance(player, str):
            number = self.game.players[player].number
            py_string = self.outcome.deref().GetPayoff(number+1).as_string().c_str()
        elif isinstance(player, int):
            py_string = self.outcome.deref().GetPayoff(player+1).as_string().c_str()
        if "." in py_string.decode(b'ascii'):
            return decimal.Decimal(py_string.decode(b'ascii'))
        else:
            return Rational(py_string.decode(b'ascii'))

    def __setitem__(self, pl: int, value: typing.Any) -> None:
        """
        Set the payoff value to a specified player for the outcome.

        Parameters
        ----------
        pl : int
            The player number for which to set the payoff
        value : Any
            The value of the payoff.  This can be any numeric type, or any object that
            has a string representation which can be interpreted as an integer,
            decimal, or rational number.

        Raises
        ------
        ValueError
              If ``payoff`` cannot be interpreted as a decimal or rational number.
        """
        self.outcome.deref().SetPayoff(pl+1, _to_number(value))


@cython.cclass
class TreeGameOutcome:
    """Represents an outcome in a strategic game derived from an extensive game."""
    psp = cython.declare(shared_ptr[c_PureStrategyProfile])
    c_game = cython.declare(c_Game)

    @property
    def game(self) -> Game:
        """Returns the game with which this outcome is associated."""
        g = Game()
        g.game = self.c_game
        return g

    def __repr__(self):
        return f"<Outcome '{self.label}' in game '{self.game.title}'>"

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, TreeGameOutcome) and
            deref(self.psp).deref() == deref(cython.cast(TreeGameOutcome, other).psp).deref()
        )

    def __ne__(self, other: typing.Any) -> bool:
        return (
            not isinstance(other, TreeGameOutcome) or
            deref(self.psp).deref() != deref(cython.cast(TreeGameOutcome, other).psp).deref()
        )

    def __getitem__(self, player):
        if isinstance(player, Player):
            return rat_to_py(deref(self.psp).deref().GetPayoff(player.number+1))
        elif isinstance(player, str):
            number = self.game.players[player].number
            return rat_to_py(deref(self.psp).deref().GetPayoff(number+1))
        elif isinstance(player, int):
            if player < 0 or player >= self.c_game.deref().NumPlayers():
                raise IndexError("Index out of range")
            return rat_to_py(deref(self.psp).deref().GetPayoff(player+1))
        raise TypeError("player index should be a Player, int or str instance; {} passed"
                        .format(player.__class__.__name__))

    def __setitem__(self, pl, value):
        raise NotImplementedError("Cannot modify outcomes in a derived strategic game.")

    def delete(self):
        raise NotImplementedError("Cannot modify outcomes in a derived strategic game.")

    @property
    def label(self) -> str:
        """The text label associated with this outcome."""
        return "(%s)" % (
            ",".join(
                [deref(self.psp).deref().GetStrategy((<Player>player).player).deref().GetLabel().c_str()
                 for player in self.game.players]
            )
        )
