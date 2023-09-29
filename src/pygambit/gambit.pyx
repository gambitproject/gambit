#
# This file is part of Gambit
# Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/lib/libgambit.pyx
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
import typing

from .error import *

from cython.cimports.src.pygambit import gambit    # noqa


class Decimal(decimal.Decimal):
    pass


class Rational(fractions.Fraction):
    """Represents a rational number in specifying numerical data for a game,
    or in a strategy profile.

    .. versionadded:: 15.0.0
    """
    def _repr_latex_(self) -> str:
        if self.denominator != 1:
            return r'$\frac{%s}{%s}$' % (self.numerator, self.denominator)
        else:
            return r'$%s$' % self.numerator


@cython.cfunc
def rat_to_py(r: c_Rational):
    """Convert a C++ Rational number to a Python Rational."""
    return Rational(rat_str(r).decode('ascii'))


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
    return c_Number(value.encode('ascii'))


@cython.cclass
class Collection:
    """Represents a collection of related objects in a game."""
    def __repr__(self):
        return str(list(self))

    def __getitem__(self, i):
        if isinstance(i, str):
            try:
                return self[[x.label for x in self].index(i)]
            except ValueError:
                raise IndexError(f"no object with label '{i}'")
        else:
            raise TypeError(f"collection indexes must be int or str, not {i.__class__.__name__}")


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
include "mixed.pxi"
include "behav.pxi"
include "game.pxi"
include "nash.pxi"
