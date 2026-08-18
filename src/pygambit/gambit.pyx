#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/libgambit.pyx
# Cython wrapper for Gambit C++ library
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
import collections.abc
import decimal
import fractions
import typing

import cython

from .error import *


class Decimal(decimal.Decimal):
    pass


class Rational(fractions.Fraction):
    """Represents a rational number in specifying numerical data for a game,
    or in a strategy profile.

    .. versionadded:: 15.0.0
    """
    def _repr_latex_(self) -> str:
        if self.denominator != 1:
            return r"$\frac{%s}{%s}$" % (self.numerator, self.denominator)
        else:
            return r"$%s$" % self.numerator


@cython.cfunc
def rat_to_py(r: c_Rational):
    """Convert a C++ Rational number to a Python Rational."""
    return Rational(to_string(r).decode("ascii"))


@cython.cfunc
def _to_number_string(value: typing.Any) -> str:
    """Coerce a value (int, float, str, Decimal, or Rational) into a canonical string
    representation of a number, following Gambit's usual numeric coercion rules.
    """
    if isinstance(value, (int, Decimal, Rational)):
        return str(value)
    if "/" in str(value):
        try:
            return str(Rational(str(value)))
        except ValueError:
            raise ValueError(f"Cannot convert '{value}' to a number") from None
    # This slightly indirect way of converting deals best with
    # rounding of floating point numbers - so calling code gets
    # the value it expects when using a float
    try:
        return str(Decimal(str(value)))
    except decimal.InvalidOperation:
        raise ValueError(f"Cannot convert '{value}' to a number") from None


@cython.cfunc
def _to_number(value: typing.Any) -> c_Number:
    """Convert a value into a game Number representation."""
    return c_Number(_to_number_string(value).encode("ascii"))


@cython.cfunc
def _resolve_by_label(collection, label: str, scope: str, kind: str, kind_plural: str):
    """Resolve a member of a game collection by its text label.

    Game collections are accessed by label, not by position.  Lookup is by exact label match.

    Failure modes:
      * an empty label raises ``ValueError``;
      * a label matching no member raises ``KeyError``;
      * a label matching more than one member raises ``ValueError``.
    """
    if not label:
        raise ValueError(f"{kind} label cannot be empty")
    matches = [x for x in collection if x.label == label]
    if not matches:
        raise KeyError(f"{scope} has no {kind} with label '{label}'")
    if len(matches) > 1:
        raise ValueError(f"{scope} has multiple {kind_plural} with label '{label}'")
    return matches[0]


PlayerReference = Player | str
StrategyReference = Strategy | str
InfosetReference = Infoset | str
ActionReference = Action | str
NodeReference = Node | str
NodeReferenceSet = typing.Iterable[NodeReference]

ProfileDType = float | Rational


@cython.cclass
class _LabeledVector:
    """Shared implementation for a read-only mapping from a label to a computed value.

    Not part of the public API; subclass this (see ``PlayerIndexedVector`` and
    ``StrategyIndexedVector``) so that the concrete type of a computed quantity carries its
    own meaning (e.g. a ``PayoffVector`` and a ``PlayerRegretVector`` never compare equal to
    each other, even if they happen to hold the same numbers).
    """
    _values = cython.declare(dict)
    _label_kind = "label"

    def __init__(self, values: collections.abc.Mapping) -> None:
        self._values = dict(values)

    def __repr__(self) -> str:
        return str(self._values)

    def _repr_latex_(self) -> str:
        values = list(self._values.values())
        if not values or not hasattr(values[0], "_repr_latex_"):
            return repr(self)
        return (
            r"$\left\{" +
            ",".join(
                r"\text{" + label + "}:" + value._repr_latex_().replace("$", "")
                for label, value in self._values.items()
            ) +
            r"\right\}$"
        )

    def __eq__(self, other: typing.Any) -> bool:
        if isinstance(other, collections.abc.Mapping):
            return self._values == dict(other)
        if type(other) is not type(self):
            return False
        return self._values == cython.cast(_LabeledVector, other)._values

    def __len__(self) -> int:
        return len(self._values)

    def __iter__(self) -> typing.Iterator[typing.Tuple[str, typing.Any], None, None]:
        yield from self._values.items()

    def __getitem__(self, label: str) -> typing.Any:
        try:
            return self._values[label]
        except KeyError:
            raise KeyError(f"no {self._label_kind} with label '{label}'") from None


@cython.cclass
class PlayerIndexedVector(_LabeledVector):
    """A read-only mapping from player label to a computed value, one entry per player
    in a game.
    """
    _label_kind = "player"


@cython.cclass
class StrategyIndexedVector(_LabeledVector):
    """A read-only mapping from strategy label to a computed value, one entry per
    strategy belonging to a single player.
    """
    _label_kind = "strategy"


######################
# Includes
######################

include "action.pxi"
include "infoset.pxi"
include "strategy.pxi"
include "player.pxi"
include "outcome.pxi"
include "node.pxi"
include "stratspt.pxi"
include "stratmixed.pxi"
include "behavmixed.pxi"
include "game.pxi"
include "nash.pxi"
