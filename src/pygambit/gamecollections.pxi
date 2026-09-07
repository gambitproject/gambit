#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/gamecollections.pxi
# Cython wrappers for the collections belonging to a game
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

@cython.cclass
class GamePlayers:
    """The labels of the (personal) players in a game.

    .. versionchanged:: 17.0.0
        Iterates over player labels (``str``) rather than ``Player`` objects;
        indexing by label is no longer supported (a label is already in hand once
        iterated) -- use ``in`` to test membership.  The chance player is no longer
        exposed here (it never was included in iteration); ``Node.player`` already
        distinguishes personal from chance nodes.
    """
    game = cython.declare(c_Game)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create GamePlayers outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(game: c_Game) -> GamePlayers:
        obj: GamePlayers = GamePlayers.__new__(GamePlayers)
        obj.game = game
        return obj

    def __repr__(self) -> str:
        return f"GamePlayers(game={Game.wrap(self.game)})"

    def __len__(self) -> int:
        """Returns the number of players in the game."""
        return self.game.deref().NumPlayers()

    def __iter__(self) -> typing.Iterator[str]:
        for player in self.game.deref().GetPlayers():
            yield player.deref().GetLabel().decode("utf-8")

    def __contains__(self, label: str) -> bool:
        return any(
            player.deref().GetLabel().decode("utf-8") == label
            for player in self.game.deref().GetPlayers()
        )
