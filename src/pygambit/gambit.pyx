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
import decimal
import fractions
import warnings
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
def _to_number(value: typing.Any) -> c_Number:
    """Convert a value into a game Number representation."""
    if isinstance(value, (int, Decimal, Rational)):
        value = str(value)
    elif "/" in str(value):
        try:
            value = str(Rational(str(value)))
        except ValueError:
            raise ValueError(f"Cannot convert '{value}' to a number") from None
    else:
        # This slightly indirect way of converting deals best with
        # rounding of floating point numbers - so calling code gets
        # the value it expects when using a float
        try:
            value = str(Decimal(str(value)))
        except decimal.InvalidOperation:
            raise ValueError(f"Cannot convert '{value}' to a number") from None
    return c_Number(value.encode("ascii"))


@cython.cfunc
def _resolve_by_label(collection, label, scope: str, kind: str, kind_plural: str):
    """Resolve a member of a game collection by its text label.

    Game collections are accessed by label, not by position.  Lookup is by exact
    label match; leading/trailing whitespace is stripped from `label` before comparison.
    Failure modes:
      * an ``int`` raises ``TypeError`` (integer indexing was removed in 16.7.0);
      * any other non-``str`` raises ``TypeError``;
      * an empty or all-whitespace label raises ``ValueError``;
      * a label matching no member raises ``KeyError``;
      * a label matching more than one member raises ``ValueError``.

    The `label` parameter is left unannotated: a concrete type annotation is compiled by Cython
    into an enforced argument check, raising a generic ``TypeError``
    before this function's migration message runs.
    """
    if isinstance(label, int):
        raise TypeError(
            f"{scope} {kind_plural} cannot be indexed by position; reference a "
            f"{kind} by its label, or iterate over the collection. "
            f"(Integer indexing was removed in 16.7.0.)"
        )
    if not isinstance(label, str):
        raise TypeError(
            f"{kind} must be referenced by a str label, not {label.__class__.__name__}"
        )
    stripped_label = label.strip()
    if not stripped_label:
        raise ValueError(f"{kind} label cannot be empty or all whitespace")
    matches = [x for x in collection if x.label == stripped_label]
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
