#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/gamecollections.pxi
# Cython wrappers for the collections of nodes, subgames, outcomes, and players
# belonging to a game
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
class GameSubgames:
    """Represents the set of subgames in a game."""
    game = cython.declare(c_Game)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create GameSubgames outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(game: c_Game) -> GameSubgames:
        obj: GameSubgames = GameSubgames.__new__(GameSubgames)
        obj.game = game
        return obj

    def __repr__(self) -> str:
        return f"GameSubgames(game={Game.wrap(self.game)})"

    def __len__(self) -> int:
        """The number of subgames in the game."""
        if not self.game.deref().IsTree():
            return 0
        return self.game.deref().GetSubgames().size()

    def __iter__(self) -> typing.Iterator[Subgame]:
        """Iterate over the game subgames in postorder."""
        if not self.game.deref().IsTree():
            return
        for subgame in self.game.deref().GetSubgames():
            yield Subgame.wrap(subgame)


@cython.cclass
class GameOutcomes:
    """Represents the set of outcomes in a game."""
    game = cython.declare(c_Game)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create GameOutcomes outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(game: c_Game) -> GameOutcomes:
        obj: GameOutcomes = GameOutcomes.__new__(GameOutcomes)
        obj.game = game
        return obj

    def __repr__(self) -> str:
        return f"GameOutcomes(game={Game.wrap(self.game)})"

    def __len__(self) -> int:
        """The number of outcomes in the game."""
        return self.game.deref().GetOutcomes().size()

    def __iter__(self) -> typing.Iterator[Outcome]:
        for outcome in self.game.deref().GetOutcomes():
            yield Outcome.wrap(outcome)

    def __getitem__(self, label: str) -> Outcome:
        """Returns the outcome with text label `label`.

        Parameters
        ----------
        label : str
            The text label of the outcome to return.  Lookup is by exact match;
            leading/trailing whitespace is stripped from `label`.

        Raises
        ------
        KeyError
            If no outcome in the game has label `label`.
        ValueError
            If `label` is empty or all whitespace, or if more than one outcome has label `label`.
        TypeError
            If `label` is not a string.

        .. versionchanged:: 16.7.0
            Integer indexing is no longer supported; reference an outcome by its label, or iterate
            over the collection.  String lookup now requires an exact match of the label;
            previously, leading/trailing whitespace was stripped from `label` before comparison.
        """
        return _resolve_by_label(self, label, "Game", "outcome", "outcomes")


@cython.cclass
class GamePlayers:
    """The labels of the (personal) players in a game.

    .. versionchanged:: 17.0.0
        Iterates over player labels (``str``) rather than ``Player`` objects;
        indexing by label is no longer supported (a label is already in hand once
        iterated) -- use ``in`` to test membership.  The chance player is no longer
        exposed here (it never was included in iteration); the ``Infoset``/``Event``
        split on ``Node`` already distinguishes personal from chance nodes.
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
