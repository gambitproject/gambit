#
# This file is part of Gambit
# Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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
import cython
from cython.operator cimport dereference as deref
from libcpp.memory cimport unique_ptr


@cython.cclass
class StrategySupport:
    """A set of strategies for a specified player in a `StrategySupportProfile`.
    """
    _profile = cython.declare(StrategySupportProfile)
    _player = cython.declare(Player)

    def __init__(self, profile: StrategySupportProfile, player: Player) -> None:
        self._profile = profile
        self._player = player

    @property
    def player(self) -> Player:
        return self._player

    def __iter__(self) -> typing.Generator[Strategy, None, None]:
        for strat in deref(self._profile.support).GetStrategies(self._player.player):
            s = Strategy()
            s.strategy = strat
            yield s


@cython.cclass
class StrategySupportProfile:
    """A set-like object representing a subset of the strategies in game.
    A StrategySupportProfile always contains at least one strategy for each player
    in the game.
    """
    support = cython.declare(unique_ptr[c_StrategySupportProfile])

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a StrategySupportProfile outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(game: Game) -> StrategySupportProfile:
        obj: StrategySupportProfile = StrategySupportProfile.__new__(StrategySupportProfile)
        obj.support.reset(new c_StrategySupportProfile(game.game))
        return obj

    @property
    def game(self) -> Game:
        """The `Game` on which the support profile is defined."""
        return Game.wrap(deref(self.support).GetGame())

    def __repr__(self) -> str:
        return f"StrategySupportProfile(game={self.game})"

    def __len__(self) -> int:
        """Returns the total number of strategies in the support profile."""
        return deref(self.support).MixedProfileLength()

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, StrategySupportProfile) and
            deref(self.support) == deref(cython.cast(StrategySupportProfile, other).support)
        )

    def __le__(self, other: StrategySupportProfile) -> bool:
        return self.issubset(other)

    def __ge__(self, other: StrategySupportProfile) -> bool:
        return self.issuperset(other)

    def __contains__(self, strategy: Strategy) -> bool:
        if strategy not in self.game.strategies:
            raise MismatchError(
                "strategy is not part of the game on which the profile is defined."
            )
        return deref(self.support).Contains(strategy.strategy)

    def __iter__(self) -> typing.Generator[Strategy, None, None]:
        for player in deref(self.support).GetGame().deref().GetPlayers():
            for strat in deref(self.support).GetStrategies(player):
                yield Strategy.wrap(strat)

    def __getitem__(self, player: PlayerReference) -> StrategySupport:
        """Return a `StrategySupport` representing the strategies in the support
        belonging to `player`.

        Parameters
        ----------
        player : Player
            The player to extract the support for

        Raises
        ------
        MismatchError
            If `player` is a `Player` from a different game.
        """
        return StrategySupport(
            self,
            cython.cast(Player, self.game._resolve_player(player, "__getitem__"))
        )

    def __and__(self, other: StrategySupportProfile) -> StrategySupportProfile:
        """Operator version of set intersection on support profiles.

        See also
        --------
        intersection
        """
        return self.intersection(other)

    def __or__(self, other: StrategySupportProfile) -> StrategySupportProfile:
        """Operator version of set union on support profiles.

        See also
        --------
        union
        """
        return self.union(other)

    def __sub__(self, other: StrategySupportProfile) -> StrategySupportProfile:
        """Operator version of set difference on support profiles.

        See also
        --------
        difference
        """
        return self.difference(other)

    def remove(self, strategy: Strategy) -> StrategySupportProfile:
        """Creates a new support profile without the given strategy.

        Parameters
        ----------
        strategy : Strategy
            The strategy to remove from the profile.

        Raises
        ------
        UndefinedOperationError
            If `strategy` is the only strategy in the support for its player.
        """
        if strategy not in self.game.strategies:
            raise MismatchError(
                "remove(): strategy is not part of the game on which the profile is defined."
            )
        if deref(self.support).GetStrategies(
                cython.cast(Player, strategy.player).player
        ).size() == 1:
            raise UndefinedOperationError(
                "remove(): cannot remove last strategy of a player"
            )
        strategies = list(self)
        strategies.remove(strategy)
        return self.game.strategy_support_profile(lambda x: x in strategies)

    def difference(self, other: StrategySupportProfile) -> StrategySupportProfile:
        """Create a support profile which contains all strategies in this profile that
        are not in `other`.

        Parameters
        ----------
        other : StrategySupportProfile
            The support profile to subtract from this one.

        Returns
        -------
        StrategySupportProfile
            The support profile resulting from the operation.

        Raises
        ------
        MismatchError
            If the support profiles are defined on different games.
        """
        if self.game != other.game:
            raise MismatchError("difference(): support profiles are defined on different games")
        return self.game.strategy_support_profile(lambda x: x in set(self) - set(other))

    def intersection(self, other: StrategySupportProfile) -> StrategySupportProfile:
        """Create a support profile which contains all strategies that are in both this and
        another profile.

        Parameters
        ----------
        other : StrategySupportProfile
            The support profile to intersect with this one.

        Returns
        -------
        StrategySupportProfile
            The support profile resulting from the operation.

        Raises
        ------
        MismatchError
            If the support profiles are defined on different games.
        """
        if self.game != other.game:
            raise MismatchError("intersection(): support profiles are defined on different games")
        return self.game.strategy_support_profile(lambda x: x in set(self) & set(other))

    def union(self, other: StrategySupportProfile) -> StrategySupportProfile:
        """Create a support profile which contains all strategies that are in either this or
        another profile.

        Parameters
        ----------
        other : StrategySupportProfile
            The other support profile to add to this one.

        Returns
        -------
        StrategySupportProfile
            The support profile resulting from the operation.

        Raises
        ------
        MismatchError
            If the support profiles are defined on different games.
        """
        if self.game != other.game:
            raise MismatchError("union(): support profiles are defined on different games")
        return self.game.strategy_support_profile(lambda x: x in set(self) | set(other))

    def issubset(self, other: StrategySupportProfile) -> bool:
        """Test for whether this support is contained in another.

        Parameters
        ----------
        other : StrategySupportProfile
            The other support profile to compare to.

        Returns
        -------
        bool
            `True` if every strategy in the profile is also in `other`.

        Raises
        ------
        MismatchError
            If the support profiles are defined on different games.
        """
        if self.game != other.game:
            raise MismatchError("issubset(): support profiles are defined on different games")
        return deref(self.support).IsSubsetOf(deref(other.support))

    def issuperset(self, other: StrategySupportProfile) -> bool:
        """Test for whether another support is contained in this one.

        Parameters
        ----------
        other : StrategySupportProfile
            The other support profile to compare to.

        Returns
        -------
        bool
            `True` if every strategy in `other` is in this profile.

        Raises
        ------
        MismatchError
            If the support profiles are defined on different games.
        """
        if self.game != other.game:
            raise MismatchError("issuperset(): support profiles are defined on different games")
        return other.is_subset_of(self)

    def restrict(self) -> Game:
        """Creates a deep copy of the support profile's game, including only the strategies
        in the support.

        .. versionchanged:: 16.1.0
            In 16.0.x, this returned a `StrategicRestriction` object.  Strategic restrictions
            have been removed in favor of using deep copies of games.
        """
        return Game.parse_game(WriteGame(deref(self.support)).decode("ascii"))

    def is_dominated(self, strategy: Strategy, strict: bool, external: bool = False) -> bool:
        return deref(self.support).IsDominated(strategy.strategy, strict, external)


def _undominated_strategies_solve(
        profile: StrategySupportProfile, strict: bool, external: bool
) -> StrategySupportProfile:
    result = StrategySupportProfile.wrap(profile.game)
    result.support.reset(
        new c_StrategySupportProfile(deref(profile.support).Undominated(strict, external))
    )
    return result
