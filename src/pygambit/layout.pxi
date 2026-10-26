#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/layout.pxi
# Computing and representing a game tree's layout for display
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
import dataclasses


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

    def __iter__(self) -> typing.Iterator[History]:
        return iter(self._data)

    def __contains__(self, history: History) -> bool:
        return history in self._data

    def __getitem__(self, history: History) -> TreeLayoutCoordinates:
        return self._data[history]

    def items(self) -> typing.ItemsView[History, TreeLayoutCoordinates]:
        return self._data.items()


@cython.cfunc
def _layout_tree(game: ExtensiveGame) -> object:
    layout = CreateLayout(game.game)
    data = {}
    for node, history in game._all_histories().items():
        data[history] = TreeLayoutCoordinates(
            deref(layout).GetNodeLevel(cython.cast(Node, node).node),
            deref(layout).GetNodeSublevel(cython.cast(Node, node).node),
            deref(layout).GetNodeOffset(cython.cast(Node, node).node))
    return TreeLayout(data)


def layout_tree(game: ExtensiveGame) -> TreeLayout:
    """Computes the layout of `game`'s tree for graphical display.

    .. versionchanged:: 17.0.0
        Returns a `TreeLayout` (History-keyed) instead of a
        `dict[Node, NodeCoordinates]`.

    Parameters
    ----------
    game : ExtensiveGame
        The game whose tree layout to compute.

    Returns
    -------
    TreeLayout
        The layout of `game`'s tree.
    """
    return _layout_tree(game)
