#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/stratspt.pxi
# Cython wrapper for strategy support profiles
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
import io
import cython
from cython.operator cimport dereference as deref
from libcpp.memory cimport unique_ptr


@cython.cclass
class _LabelSet:
    """Shared implementation for `StrategySupport` and `ActionSupport`: an immutable
    snapshot of a set of labels (strategies, or actions at an information set) taken
    from a support profile at retrieval time, together with the owner (a player label,
    or the History identifying an information set) the labels belong to.

    Not exported; only `StrategySupport` and `ActionSupport` are part of the public API.
    """
    _owner = cython.declare(object)
    _labels = cython.declare(tuple)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError(f"Cannot create an {type(self).__name__} outside a Game.")

    def __repr__(self) -> str:
        return str(list(self._labels))

    def __eq__(self, other: typing.Any) -> bool:
        if isinstance(other, (set, frozenset, list, tuple)):
            return set(self._labels) == set(other)
        if (not isinstance(other, type(self)) or
                self._owner != cython.cast(_LabelSet, other)._owner):
            return False
        return set(self._labels) == set(cython.cast(_LabelSet, other)._labels)

    def __len__(self) -> int:
        return len(self._labels)

    def __iter__(self) -> typing.Generator[str, None, None]:
        yield from self._labels

    def __contains__(self, label: str) -> bool:
        return label in self._labels


@cython.cclass
class StrategySupport(_LabelSet):
    """The labels of the strategies for a specified player in a
    `StrategySupportProfile`.

    An immutable snapshot taken from a ``StrategySupportProfile`` at retrieval time: it
    does not reflect later changes to the profile. The player is accessible via `player`.

    .. versionchanged:: 17.0.0

        No longer a live view onto the profile: holds its own copy of the strategy
        labels. Iterates over strategy labels (``str``) rather than ``Strategy``
        objects.
    """
    @staticmethod
    @cython.cfunc
    def wrap(player: str, strategies: tuple) -> StrategySupport:
        obj: StrategySupport = StrategySupport.__new__(StrategySupport)
        obj._owner = player
        obj._labels = strategies
        return obj

    @property
    def player(self) -> str:
        return self._owner


@cython.cclass
class StrategySupportProfile:
    """A set-like object representing a subset of the strategies in game.
    A StrategySupportProfile always contains at least one strategy for each player
    in the game.
    """
    profile = cython.declare(shared_ptr[c_StrategySupportProfile])

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a StrategySupportProfile outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(profile: shared_ptr[c_StrategySupportProfile]) -> StrategySupportProfile:
        obj: StrategySupportProfile = StrategySupportProfile.__new__(StrategySupportProfile)
        obj.profile = profile
        return obj

    @property
    def game(self) -> Game:
        """The `Game` on which the support profile is defined."""
        return Game.wrap(deref(self.profile).GetGame())

    def __repr__(self) -> str:
        return f"StrategySupportProfile(game={self.game})"

    def __len__(self) -> int:
        """Returns the total number of strategies in the support profile."""
        return deref(self.profile).MixedProfileLength()

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, StrategySupportProfile) and
            deref(self.profile) == deref(cython.cast(StrategySupportProfile, other).profile)
        )

    def __iter__(self) -> typing.Generator[StrategySupport, None, None]:
        """Iterate over the strategy supports in the profile, one per player.

        Yields
        ------
        support : StrategySupport
            The player's strategy support specified in the profile
        """
        for player in self.game.players:
            yield self[player]

    def __getitem__(self, player: str) -> StrategySupport:
        """Return a `StrategySupport` representing the labels of the strategies in the
        support belonging to the player with label `player`, as of now; it will not
        reflect any later changes to this profile.

        Parameters
        ----------
        player : str
            The label of the player to extract the support for.

        Raises
        ------
        KeyError
            If no player in the game has the label `player`.
        """
        game: Game = self.game
        resolved_player: c_GamePlayer = game._resolve_player(
            player, "StrategySupportProfile.__getitem__"
        )
        strategies = tuple(
            s.deref().GetLabel().decode("utf-8")
            for s in deref(self.profile).GetStrategies(resolved_player)
        )
        return StrategySupport.wrap(player, strategies)

    @cython.cfunc
    def _ensure_unshared(self) -> cython.void:
        """Clones the underlying profile if it is shared with another wrapper, so that
        the mutation about to happen is not observed by any other StrategySupportProfile.
        """
        if self.profile.use_count() != 1:
            self.profile = make_shared[c_StrategySupportProfile](deref(self.profile))

    @cython.cfunc
    def _set_support(self, player: str, strategies: object) -> cython.void:
        """Validates and sets the whole support for player.

        Every entry of `strategies` must be one of the player's strategy labels, and
        at least one must be given.
        """
        game: Game = self.game
        player_strategies = game.get_strategies(player)
        labels = set(player_strategies)
        given = set(strategies)
        unknown = given - labels
        if unknown:
            raise ValueError(
                f"not a strategy label for this player: {', '.join(sorted(unknown))}"
            )
        if not given:
            raise ValueError("a support must contain at least one strategy for the player")
        self._ensure_unshared()
        # Strategies to keep are added first, so that a subsequent removal is never asked
        # to remove the last remaining strategy for the player.
        for s in player_strategies:
            if s in given:
                deref(self.profile).AddStrategy(game._resolve_strategy(player, s, "_set_support"))
        for s in player_strategies:
            if s not in given:
                deref(self.profile).RemoveStrategy(
                    game._resolve_strategy(player, s, "_set_support")
                )

    def __setitem__(self, player: str, strategies: typing.Iterable[str]) -> None:
        """Sets the support for the player with label `player` to exactly the given
        strategies.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        player : str
            The label of the player whose support is to be set.
        strategies : Iterable[str]
            The labels of the strategies which should be in the support for the
            player. Every one of the player's other strategies is removed from the
            support.

        Raises
        ------
        KeyError
            If no player in the game has the label `player`.
        ValueError
            If any entry of `strategies` is not one of the player's strategy labels,
            or if `strategies` is empty.
        """
        self._set_support(player, strategies)

    def copy(self) -> StrategySupportProfile:
        """Creates a copy of the support profile.

        .. versionadded:: 17.0.0

            The copy shares its underlying data with the original until one of them is
            next assigned into, at which point the one being assigned into transparently
            takes its own private copy first. Both profiles are fully independent from
            each other's perspective; this only affects when the underlying duplication
            happens, not whether it happens.
        """
        return StrategySupportProfile.wrap(self.profile)

    def restrict(self) -> Game:
        """Creates a deep copy of the support profile's game, including only the strategies
        in the support.

        .. versionchanged:: 16.1.0
            In 16.0.x, this returned a `StrategicRestriction` object.  Strategic restrictions
            have been removed in favor of using deep copies of games.
        """
        with io.StringIO(WriteNfgFileSupport(deref(self.profile)).decode()) as f:
            return read_nfg(f)

    def is_dominated(
        self, player: str, strategy: str, strict: bool, external: bool = False
    ) -> bool:
        """Returns whether the strategy with label `strategy`, belonging to the player
        with label `player`, is dominated.

        Parameters
        ----------
        player : str
            The label of the player to whom the strategy belongs.
        strategy : str
            The label of the strategy to check.
        strict : bool
            If `True`, only checks for strict dominance.
        external : bool, default False
            The default is to consider dominance only by strategies which are in
            the support for the player. If `True`, strategies which are dominated
            by another strategy not in the support profile are also considered.

        Raises
        ------
        KeyError
            If no player in the game has the label `player`, or the player has no
            strategy with the label `strategy`.
        """
        game: Game = self.game
        handle = game._resolve_strategy(player, strategy, "is_dominated", "strategy")
        return deref(self.profile).IsDominated(handle, strict, external)


def _undominated_strategies_solve(
        profile: StrategySupportProfile, strict: bool, external: bool
) -> StrategySupportProfile:
    return StrategySupportProfile.wrap(
        make_shared[c_StrategySupportProfile](
            deref(profile.profile).Undominated(strict, external)
        )
    )
