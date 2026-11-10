#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/_distributions.py
# Validation of the weights given for a mixed strategy or a mixed action
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

from __future__ import annotations

import collections.abc
import typing

from ._numbers import Rational, to_number_string

_Prob = typing.TypeVar("_Prob", float, Rational)


class Owner(typing.NamedTuple):
    """How the owner of a distribution is named in error messages."""

    item: str
    """What the labels name: ``"strategy"`` or ``"action"``."""
    unknown: str
    """The start of the message for a label the owner does not have."""
    required: str
    """What a non-sparse distribution must give one weight for."""


PLAYER_STRATEGIES = Owner(
    item="strategy",
    unknown="not a strategy label for this player",
    required="each of the player's strategies, by label",
)
INFOSET_ACTIONS = Owner(
    item="action",
    unknown="not an action label at this information set",
    required="each action at the information set",
)


def to_float_prob(value: typing.Any) -> float:
    """Coerce a value (int, float, str, Decimal, or Rational) into a float probability."""
    normalized = to_number_string(value)
    try:
        return float(normalized)
    except ValueError:
        # normalized is a fraction-form string (e.g. "1/2"), which float() rejects
        return float(Rational(normalized))


def to_rational_prob(value: typing.Any) -> Rational:
    """Coerce a value (int, float, str, Decimal, or Rational) into a Rational probability."""
    return Rational(to_number_string(value))


def validate_distribution(
    labels: collections.abc.Collection[str],
    distribution: object,
    sparse: bool,
    to_prob: collections.abc.Callable[[typing.Any], _Prob],
    noun: str,
    owner: Owner,
) -> dict[str, _Prob]:
    """Validate the weights `distribution` gives to `labels`, and return a weight for each label.

    `distribution` must be a Mapping whose keys are all in `labels`. If `sparse` is False it
    must have every label as a key; if True, labels it omits get weight zero. Weights are
    converted by `to_prob` and must be non-negative and not all zero.  `noun` names the
    distribution ("mixed strategy", "mixed action") and `owner` the thing it belongs to, in
    the error messages.

    Raises
    ------
    TypeError
        If `distribution` is not a Mapping.
    ValueError
        If a key is not in `labels`, if `sparse` is False and a label is missing, if a
        weight cannot be converted, if a weight is negative, or if all weights are zero.
    """
    if not isinstance(distribution, collections.abc.Mapping):
        raise TypeError(
            f"a {noun} must be set from a Mapping from {owner.item} label to "
            f"weight, not {distribution.__class__.__name__}"
        )
    known = set(labels)
    given = set(distribution.keys())
    unknown = given - known
    if unknown:
        raise ValueError(f"{owner.unknown}: {', '.join(sorted(unknown))}")
    if not sparse and given != known:
        raise ValueError(
            f"a distribution must specify exactly one weight for {owner.required}, "
            "unless sparse=True"
        )
    zero = to_prob(0)
    values = {label: zero for label in labels}
    values.update({label: to_prob(weight) for label, weight in distribution.items()})
    if any(v < 0 for v in values.values()):
        raise ValueError(f"a {noun}'s weights must be non-negative")
    if all(v == 0 for v in values.values()):
        raise ValueError(f"a {noun}'s weights must not all be zero")
    return values
