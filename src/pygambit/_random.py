#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/_random.py
# Random probability distributions for random strategy and behavior profiles
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

import typing

import numpy as np
import scipy.stats

from ._numbers import Rational

_Item = typing.TypeVar("_Item")


def _dirichlet_distribution(
    items: list[_Item], gen: np.random.Generator | None
) -> dict[_Item, float]:
    """A uniform-random probability distribution over `items` (a flat Dirichlet(1,...,1)
    draw), keyed by item, as `float`. Shared by `random_strategy_profile`/
    `random_behavior_profile`'s `denom=None` case.
    """
    weights = scipy.stats.dirichlet(alpha=[1 for _ in items], seed=gen).rvs(size=1)[0]
    return dict(zip(items, weights, strict=True))


def _grid_distribution(
    items: list[_Item], denom: int, gen: np.random.Generator | None
) -> dict[_Item, Rational]:
    """A uniform-random probability distribution over `items`, restricted to the grid
    with denominator `denom` (a uniformly-random composition of `denom` into
    `len(items)` nonnegative parts), keyed by item, as `Rational`. Shared by
    `random_strategy_profile`/`random_behavior_profile`'s `denom` grid case.
    """
    k = len(items)
    sample = (
        [0]
        + sorted((gen or np.random).choice(np.arange(1, denom + k), size=k - 1, replace=False))
        + [denom + k]
    )
    return {
        item: Rational(hi - lo - 1, denom)
        for item, (hi, lo) in zip(items, zip(sample[1:], sample[:-1], strict=True), strict=True)
    }
