#
# This file is part of Gambit
# Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/pygambit/stratspt.pxi
# Cython wrapper for strategy supports
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
class StrategySupportProfile:
    """A set-like object representing a subset of the strategies in game.
    A StrategySupportProfile always contains at least one strategy for each player
    in the game.
    """
    support = cython.declare(unique_ptr[c_StrategySupportProfile])

    def __init__(self,
                 game: Game,
                 strategies: typing.Optional[typing.Iterable[Strategy]] = None):
        if (strategies is not None and
                len(set([strat.player.number for strat in strategies])) != len(game.players)):
            raise ValueError(
               "A StrategySupportProfile must have at least one strategy for each player"
            )
        # There's at least one strategy for each player, so this forms a valid support profile
        self.support.reset(new c_StrategySupportProfile(game.game))
        if strategies is not None:
            for strategy in game.strategies:
                if strategy not in strategies:
                    deref(self.support).RemoveStrategy(cython.cast(Strategy, strategy).strategy)

    @property
    def game(self) -> Game:
        """The `Game` on which the support profile is defined."""
        g = Game()
        g.game = deref(self.support).GetGame()
        return g

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

    def __getitem__(self, index: int) -> Strategy:
        for pl in range(len(self.game.players)):
            if index < deref(self.support).NumStrategiesPlayer(pl+1):
                s = Strategy()
                s.strategy = deref(self.support).GetStrategy(pl+1, index+1)
                return s
            index = index - deref(self.support).NumStrategiesPlayer(pl+1)
        raise IndexError("StrategySupportProfile index out of range")

    def __contains__(self, strategy: Strategy) -> bool:
        if strategy not in self.game.strategies:
            raise MismatchError(
                "strategy is not part of the game on which the profile is defined."
            )
        return deref(self.support).Contains(strategy.strategy)

    def __iter__(self) -> typing.Generator[Strategy, None, None]:
        for pl in range(len(self.game.players)):
            for st in range(deref(self.support).NumStrategiesPlayer(pl+1)):
                s = Strategy()
                s.strategy = deref(self.support).GetStrategy(pl+1, st+1)
                yield s

    def __and__(self, other: StrategySupportProfile) -> StrategySupportProfile:
        return self.intersection(other)

    def __or__(self, other: StrategySupportProfile) -> StrategySupportProfile:
        return self.union(other)

    def __sub__(self, other: StrategySupportProfile) -> StrategySupportProfile:
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
        if deref(self.support).NumStrategiesPlayer(strategy.player.number + 1) == 1:
            raise UndefinedOperationError(
                "remove(): cannot remove last strategy of a player"
            )
        strategies = list(self)
        strategies.remove(strategy)
        return StrategySupportProfile(self.game, strategies)

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
        return StrategySupportProfile(self.game, set(self) - set(other))

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
        return StrategySupportProfile(self.game, set(self) & set(other))

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
        return StrategySupportProfile(self.game, set(self) | set(other))

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


def _undominated_strategies_solve(
        profile: StrategySupportProfile, strict: bool, external: bool
) -> StrategySupportProfile:
    result = StrategySupportProfile(profile.game)
    result.support.reset(
        new c_StrategySupportProfile(deref(profile.support).Undominated(strict, external))
    )
    return result
