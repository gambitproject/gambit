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
