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
class StrategySupport:
    """A set of strategies for a specified player in a `StrategySupportProfile`.

    An immutable snapshot taken from a ``StrategySupportProfile`` at retrieval time: it
    does not reflect later changes to the profile. The player is accessible via `player`.

    .. versionchanged:: 17.0.0

        No longer a live view onto the profile: holds its own copy of the strategies.
    """
    _player = cython.declare(Player)
    _strategies = cython.declare(tuple)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a StrategySupport outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(player: Player, strategies: tuple) -> StrategySupport:
        obj: StrategySupport = StrategySupport.__new__(StrategySupport)
        obj._player = player
        obj._strategies = strategies
        return obj

    @property
    def player(self) -> Player:
        return self._player

    def __repr__(self) -> str:
        return str(list(self._strategies))

    def __eq__(self, other: typing.Any) -> bool:
        if isinstance(other, (set, frozenset, list, tuple)):
            return set(self._strategies) == set(other)
        if not isinstance(other, StrategySupport) or self.player != other.player:
            return False
        return set(self._strategies) == set(cython.cast(StrategySupport, other)._strategies)

    def __len__(self) -> int:
        return len(self._strategies)

    def __iter__(self) -> typing.Generator[Strategy, None, None]:
        yield from self._strategies

    def __contains__(self, strategy: Strategy) -> bool:
        return strategy in self._strategies


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

    def __contains__(self, strategy: Strategy) -> bool:
        if strategy not in self.game.strategies:
            raise MismatchError(
                "strategy is not part of the game on which the profile is defined."
            )
        return deref(self.profile).Contains(strategy.strategy)

    def __iter__(self) -> typing.Generator[Strategy, None, None]:
        for player in deref(self.profile).GetGame().deref().GetPlayers():
            for strat in deref(self.profile).GetStrategies(player):
                yield Strategy.wrap(strat)

    def __getitem__(self, player: PlayerReference) -> StrategySupport:
        """Return a `StrategySupport` representing the strategies in the support
        belonging to `player`, as of now; it will not reflect any later changes to
        this profile.

        Parameters
        ----------
        player : Player
            The player to extract the support for

        Raises
        ------
        MismatchError
            If `player` is a `Player` from a different game.
        """
        resolved_player = cython.cast(Player, self.game._resolve_player(player, "__getitem__"))
        strategies = tuple(
            Strategy.wrap(s) for s in deref(self.profile).GetStrategies(resolved_player.player)
        )
        return StrategySupport.wrap(resolved_player, strategies)

    @cython.cfunc
    def _ensure_unshared(self) -> cython.void:
        """Clones the underlying profile if it is shared with another wrapper, so that
        the mutation about to happen is not observed by any other StrategySupportProfile.
        """
        if self.profile.use_count() != 1:
            self.profile = make_shared[c_StrategySupportProfile](deref(self.profile))

    @cython.cfunc
    def _set_support(self, player: Player, strategies: object) -> cython.void:
        """Validates and sets the whole support for player.

        Every entry of `strategies` must be one of the player's strategy labels, and
        at least one must be given.
        """
        labels = {s.label for s in player.strategies}
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
        for s in player.strategies:
            if s.label in given:
                deref(self.profile).AddStrategy(cython.cast(Strategy, s).strategy)
        for s in player.strategies:
            if s.label not in given:
                deref(self.profile).RemoveStrategy(cython.cast(Strategy, s).strategy)

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
        resolved_player = cython.cast(Player, self.game._resolve_player(player, "__setitem__"))
        self._set_support(resolved_player, strategies)

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

    def is_dominated(self, strategy: Strategy, strict: bool, external: bool = False) -> bool:
        return deref(self.profile).IsDominated(strategy.strategy, strict, external)


def _undominated_strategies_solve(
        profile: StrategySupportProfile, strict: bool, external: bool
) -> StrategySupportProfile:
    return StrategySupportProfile.wrap(
        make_shared[c_StrategySupportProfile](
            deref(profile.profile).Undominated(strict, external)
        )
    )
