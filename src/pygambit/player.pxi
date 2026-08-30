#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/player.pxi
# Cython wrapper for players
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
import cython


@cython.cclass
class PlayerActions:
    """Represents the set of all actions available to a player at some information set."""
    player = cython.declare(Player)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create PlayerActions outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(player: Player) -> PlayerActions:
        obj: PlayerActions = PlayerActions.__new__(PlayerActions)
        obj.player = player
        return obj

    def __repr__(self) -> str:
        return f"PlayerActions(player={self.player})"

    def __len__(self) -> int:
        return sum(
            len(node.infoset.actions)
            for node in self.player.game.get_infosets(self.player.label)
        )

    def __iter__(self) -> typing.Iterator[Action]:
        for node in self.player.game.get_infosets(self.player.label):
            yield from node.infoset.actions

    def __getitem__(self, label: str) -> Action:
        """Returns the player's action with text label `label`.

        Parameters
        ----------
        label : str
            The text label of the action to return.  Lookup is by exact match;
            leading/trailing whitespace is stripped from `label`.

        Raises
        ------
        KeyError
            If the player has no action with label `label`.
        ValueError
            If `label` is empty or all whitespace, or if more than one of the player's actions
            has label `label`.
        TypeError
            If `label` is not a string.

        .. versionchanged:: 16.7.0
            Integer indexing is no longer supported; reference an action by its label, or iterate
            over the collection.  String lookup now requires an exact match of the label;
            previously, leading/trailing whitespace was stripped from `label` before comparison.
        """
        return _resolve_by_label(self, label, "Player", "action", "actions")


@cython.cclass
class PlayerStrategies:
    """The labels of the strategies available to a player.

    .. versionchanged:: 17.0.0
        Iterates over strategy labels (``str``) rather than ``Strategy`` objects;
        indexing by label is no longer supported (a label is already in hand once
        iterated) -- use ``in`` to test membership.
    """
    player = cython.declare(c_GamePlayer)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create PlayerStrategies outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(player: c_GamePlayer) -> PlayerStrategies:
        obj: PlayerStrategies = PlayerStrategies.__new__(PlayerStrategies)
        obj.player = player
        return obj

    def __repr__(self) -> str:
        return f"PlayerStrategies(player={Player.wrap(self.player)})"

    def __len__(self) -> int:
        """The number of strategies for the player in the game."""
        return self.player.deref().GetStrategies().size()

    def __iter__(self) -> typing.Iterator[str]:
        for strategy in self.player.deref().GetStrategies():
            yield strategy.deref().GetLabel().decode("utf-8")

    def __contains__(self, label: str) -> bool:
        return any(
            strategy.deref().GetLabel().decode("utf-8") == label
            for strategy in self.player.deref().GetStrategies()
        )


@cython.cclass
class PlayerSequences:
    """The collection of sequences available to a player."""
    player = cython.declare(c_GamePlayer)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create PlayerSequences outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(player: c_GamePlayer) -> PlayerSequences:
        obj: PlayerSequences = PlayerSequences.__new__(PlayerSequences)
        obj.player = player
        return obj

    def __repr__(self) -> str:
        return f"PlayerSequences(player={Player.wrap(self.player)})"

    def __len__(self) -> int:
        """The number of sequences for the player in the game."""
        return self.player.deref().GetSequences().size()

    def __iter__(self) -> typing.Iterator[Sequence]:
        for sequence in self.player.deref().GetSequences():
            yield Sequence.wrap(sequence)


@cython.cclass
class Player:
    """A player in a ``Game``."""
    player = cython.declare(c_GamePlayer)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a Player outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(player: c_GamePlayer) -> Player:
        obj: Player = Player.__new__(Player)
        obj.player = player
        return obj

    def __repr__(self) -> str:
        if self.is_chance:
            return f"ChancePlayer(game={self.game})"
        if self.label:
            return f"Player(game={self.game}, label='{self.label}')"
        else:
            return f"Player(game={self.game}, number={self.number})"

    def __eq__(self, other: typing.Any):
        if not isinstance(other, Player):
            return NotImplemented
        return self.player.deref() == cython.cast(Player, other).player.deref()

    def __hash__(self) -> int:
        return cython.cast(cython.long, self.player.deref())

    @property
    def game(self) -> Game:
        """Gets the ``Game`` to which the player belongs."""
        return Game.wrap(self.player.deref().GetGame())

    @property
    def label(self) -> str:
        """The text label of the player.

        .. versionchanged:: 17.0.0
            A label may now be any well-formed UTF-8 text, not just ASCII; it must still
            contain no control characters, and must not begin/end with whitespace or have
            two consecutive whitespace characters.  "Whitespace" means any Unicode space
            separator (e.g. U+00A0 NO-BREAK SPACE), not just the ASCII space.

            The label is now read-only, and must be nonempty and unique among the game's
            players; use `Game.relabel_players` to change it.
        """
        return self.player.deref().GetLabel().decode("utf-8")

    @property
    def number(self) -> int:
        """Returns the number of the player in its game.
        Players are numbered starting with 0.
        """
        return self.player.deref().GetNumber() - 1

    @property
    def is_chance(self) -> bool:
        """Returns whether the player is the chance player."""
        return self.player.deref().IsChance() != 0

    @property
    def strategies(self) -> PlayerStrategies:
        """Returns the collection of strategies belonging to the player."""
        return PlayerStrategies.wrap(self.player)

    @property
    def sequences(self) -> PlayerSequences:
        """Returns the collection of sequences belonging to the player."""
        return PlayerSequences.wrap(self.player)

    @property
    def actions(self) -> PlayerActions:
        """Returns the set of actions available to the player at some information set.

        Raises
        ------
        UndefinedOperationError
            If the game does not have a tree representation.
        """
        if not self.game.is_tree:
            raise UndefinedOperationError(
                "Operation only defined for games with a tree representation"
            )
        return PlayerActions.wrap(self)

    @property
    def min_payoff(self) -> Rational:
        """Returns the smallest payoff for the player in any play of the game.

        .. versionchanged:: 16.5.0
           Changed from reporting minimum payoff in any (non-null) outcome to the minimum
           payoff in any play of the game.

        See Also
        --------
        Player.max_payoff
        Game.min_payoff
        """
        return rat_to_py(self.player.deref().GetGame().deref().GetPlayerMinPayoff(self.player))

    @property
    def max_payoff(self) -> Rational:
        """Returns the largest payoff for the player in any play of the game.

        .. versionchanged:: 16.5.0
           Changed from reporting maximum payoff in any (non-null) outcome to the maximum
           payoff in any play of the game.

        See Also
        --------
        Player.min_payoff
        Game.max_payoff
        """
        return rat_to_py(self.player.deref().GetGame().deref().GetPlayerMaxPayoff(self.player))
