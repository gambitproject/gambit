#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/_layout.py
# Representation of a game tree's layout for display
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
import dataclasses
import typing

if typing.TYPE_CHECKING:
    from ._hsel import History


@dataclasses.dataclass
class TreeLayoutCoordinates:
    """The layout coordinates of a single node in a game tree, computed for
    graphical display.

    .. versionchanged:: 17.0.0
        Renamed from `NodeCoordinates`.
    """

    level: int
    sublevel: int
    offset: float


class TreeLayout:
    """The layout of a game's tree, computed for graphical display.

    Maps each node's History to its `TreeLayoutCoordinates`.

    .. versionadded:: 17.0.0
    """

    def __init__(self, data: dict[History, TreeLayoutCoordinates]) -> None:
        self._data = data

    def __repr__(self) -> str:
        return f"TreeLayout({self._data!r})"

    def __len__(self) -> int:
        return len(self._data)

    def __iter__(self) -> collections.abc.Iterator[History]:
        return iter(self._data)

    def __contains__(self, history: History) -> bool:
        return history in self._data

    def __getitem__(self, history: History) -> TreeLayoutCoordinates:
        return self._data[history]

    def items(self) -> collections.abc.ItemsView[History, TreeLayoutCoordinates]:
        return self._data.items()
