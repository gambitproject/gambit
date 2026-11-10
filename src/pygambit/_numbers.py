#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/_numbers.py
# Number types and number-string coercion used by the extension
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
import typing


class Decimal(decimal.Decimal):
    pass


class Rational(fractions.Fraction):
    """Represents a rational number in specifying numerical data for a game,
    or in a strategy profile.

    .. versionadded:: 15.0.0
    """

    def _repr_latex_(self) -> str:
        if self.denominator != 1:
            return rf"$\frac{{{self.numerator}}}{{{self.denominator}}}$"
        else:
            return rf"${self.numerator}$"


def to_number_string(value: typing.Any) -> str:
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
