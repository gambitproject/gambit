#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/_nashsets.py
# Representation of the connected components of a game's set of Nash equilibria
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

import dataclasses
import typing

if typing.TYPE_CHECKING:
    from .gambit import MixedStrategyProfile


@dataclasses.dataclass(frozen=True)
class NashPolytope:
    """The product of a polytope of player 1's mixed strategies and a polytope of
    player 2's mixed strategies, every point of which is a Nash equilibrium of a
    two-player game, and which is maximal among such sets.

    .. versionadded:: 17.0.0

    Attributes
    ----------
    vertices : list of MixedStrategyProfile
        The extreme equilibria of this polytope.
    """

    vertices: list[MixedStrategyProfile]


@dataclasses.dataclass(frozen=True)
class NashComponent:
    """A connected component of the set of Nash equilibria of a two-player game.

    A component is the union of one or more `NashPolytope`.  It need not be convex.

    .. versionadded:: 17.0.0

    Attributes
    ----------
    polytopes : list of NashPolytope
        The polytopes whose union is this component.
    """

    polytopes: list[NashPolytope]

    @property
    def vertices(self) -> list[MixedStrategyProfile]:
        """The extreme equilibria of this component, without repeats."""
        result: list[MixedStrategyProfile] = []
        for polytope in self.polytopes:
            for vertex in polytope.vertices:
                if vertex not in result:
                    result.append(vertex)
        return result
