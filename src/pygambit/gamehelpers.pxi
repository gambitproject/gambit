#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/gamehelpers.pxi
# Private helper shared by the Game classes' node-resolution methods
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


def _node_infoset_kind(node: Node) -> tuple:
    """Whether `node` currently belongs to a personal player's information set, and
    whether it belongs to a chance event -- exactly one of the two, or neither if
    `node` is currently terminal.
    """
    handle: c_GameInfoset = cython.cast(Node, node)._infoset_handle()
    if handle == cython.cast(c_GameInfoset, NULL):
        return False, False
    is_chance = handle.deref().IsChanceInfoset()
    return not is_chance, is_chance
