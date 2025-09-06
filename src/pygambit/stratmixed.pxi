#
# This file is part of Gambit
# Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/stratmixed.pxi
# Cython wrapper for mixed strategy profiles
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
from cython.operator cimport dereference as deref


@cython.cclass
class MixedStrategy:
    """A probability distribution over a player's strategies.

    A ``MixedStrategy`` represents the component of a ``MixedStrategyProfile``
    associated with a given ``Player``.  The  full profile is accessible via the `profile`
    attribute, and the player for whom the  ``MixedStrategy`` applies is accessible
    via `player`.
    """
    _profile = cython.declare(MixedStrategyProfile)
    _player = cython.declare(Player)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a MixedStrategy outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(profile: MixedStrategyProfile, player: Player) -> MixedStrategy:
        obj: MixedStrategy = MixedStrategy.__new__(MixedStrategy)
        obj._profile = profile
        obj._player = player
        return obj

    @property
    def profile(self) -> MixedStrategyProfile:
        """The full profile of which this is a part."""
        return self._profile

    @property
    def player(self) -> Player:
        """The player for whom this mixed strategy is defined."""
        return self._player

    def __repr__(self) -> str:
        return str([self.profile[s] for s in self.player.strategies])

    def _repr_latex_(self) -> str:
        if isinstance(self.profile, MixedStrategyProfileRational):
            return (
                r"$\left[" +
                ",".join(self.profile[strategy]._repr_latex_().replace("$", "")
                         for strategy in self.player.strategies) +
                r"\right]$"
            )
        else:
            return repr(self)

    def __eq__(self, other: typing.Any) -> bool:
        if isinstance(other, list):
            return [self[strategy] for strategy in self.player.strategies] == other
        if not isinstance(other, MixedStrategy) or self.player != other.player:
            return False
        return (
            [self[strategy] for strategy in self.player.strategies] ==
            [other[strategy] for strategy in other.player.strategies]
        )

    def __len__(self) -> int:
        return len(self.player.strategies)

    def __iter__(self) -> typing.Iterator[typing.Tuple[Strategy, ProfileDType], None, None]:
        """Iterate over the probabilities assigned to strategies by the mixed strategy.

        .. versionadded:: 16.2.0

        Yields
        ------
        strategy : Strategy
            A strategy for the player
        probability: float or Rational
            The probability the mixed strategy assigns to the strategy being played
        """
        for strategy in self.player.strategies:
            yield strategy, self[strategy]

    def __getitem__(self, index: StrategyReference) -> ProfileDType:
        """Returns the probability that the strategy referred to by `index` is played.

        Parameters
        ----------
        index : Strategy or str

            * If `index` is a ``Strategy``, returns the probability the strategy is played.
            * If `index` is a ``str``, attempts to resolve the referenced object by searching
              for a strategy with that label.

        Returns
        -------
        float or Rational
            The probability assigned to the strategy.

        Raises
        ------
        MismatchError
            If `index` is a ``Strategy`` that does not belong to this ``MixedStrategy``'s player.
        """
        self.profile._check_validity()
        if isinstance(index, Strategy):
            if index.player != self.player:
                raise MismatchError("strategy must belong to this player")
            return self.profile._getprob_strategy(index)
        if isinstance(index, str):
            try:
                return self.profile._getprob_strategy(self.player.strategies[index])
            except KeyError:
                raise KeyError(f"no strategy with label '{index}' for player") from None
        raise TypeError(f"strategy index must be Strategy or str, not {index.__class__.__name__}")

    def __setitem__(self, index: StrategyReference, value: typing.Any) -> None:
        """Sets the probability a strategy is played.

        Parameters
        ----------
        index : Strategy, or str
            The part of the profile to set:

            * If `index` is a ``Strategy``, sets the probability the strategy is played.
            * If `index` is a ``str``, attempts to resolve the referenced object by searching
              for a strategy with that label, and sets the probability for that strategy.

        value
            Any value which can be converted to the data type of the ``MixedStrategyProfile``.

        Raises
        ------
        MismatchError
            If `strategy` is a ``Strategy`` that does not belong to this ``MixedStrategy``'s
            player.
        """
        self.profile._check_validity()
        if isinstance(index, Strategy):
            if index.player != self.player:
                raise MismatchError("strategy must belong to this player")
            self.profile._setprob_strategy(index, value)
            return
        if isinstance(index, str):
            try:
                self.profile._setprob_strategy(self.player.strategies[index], value)
                return
            except KeyError:
                raise KeyError(f"no strategy with label '{index}' for player") from None
        raise TypeError(f"strategy index must be Strategy or str, not {index.__class__.__name__}")


@cython.cclass
class MixedStrategyProfile:
    """Represents a mixed strategy profile over the strategies in a ``Game``.

    A mixed strategy profile is a dict-like object, mapping each strategy in a game to
    the corresponding probability with which that strategy is played.

    Mixed strategy profiles may represent probabilities as either exact (rational)
    numbers, or floating-point numbers.  These may not be combined in the same mixed
    strategy profile.

    .. versionchanged:: 16.1.0
        Profiles are accessed as dict-like objects; indexing by integer player or strategy
        indices is no longer supported.

    See Also
    --------
    Game.mixed_strategy_profile
        Creates a new mixed strategy profile on a game.
    MixedBehaviorProfile
        Represents a mixed behavior profile over a ``Game`` with an extensive
        representation.
    """
    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a MixedStrategyProfile outside a Game.")

    def __repr__(self) -> str:
        return MultiIndexSeriesFormatter().format(self.as_dict())

    def _repr_latex_(self) -> str:
        return (
            r"$\left[" +
            ",".join([self[player]._repr_latex_().replace("$", "")
                      for player in self.game.players]) +
            r"\right]$"
        )

    def as_dict(self) -> dict[tuple, float]:
        result = {}
        for player in self.game.players:
            for strategy in player.strategies:
                result[(player.label, strategy.label)] = self[player][strategy]
        return result

    @property
    def game(self) -> Game:
        """The game on which this mixed strategy profile is defined."""
        return self._game

    def mixed_strategies(self) -> typing.Iterator[typing.Tuple[Player, MixedStrategy], None, None]:
        """Iterate over the mixed strategies in the profile.

        .. versionadded:: 16.2.0

        Yields
        ------
        player : Player
            A player in the game
        strategy : MixedStrategy
            The player's mixed strategy specified in the profile
        """
        for player in self.game.players:
            yield player, self[player]

    def __iter__(self) -> typing.Iterator[typing.Tuple[Strategy, ProfileDType], None, None]:
        """Iterate over the probabilities assigned to strategies by the profile.

        .. versionadded:: 16.2.0

        Yields
        ------
        strategy : Strategy
            A strategy in the game
        probability: float or Rational
            The probability the profile assigns to the strategy being played
        """
        for strategy in self.game.strategies:
            yield strategy, self[strategy]

    def __getitem__(
            self,
            index: typing.Union[PlayerReference, StrategyReference]
    ) -> typing.Union[MixedStrategy, ProfileDType]:
        """Access a component of the mixed strategy profile specified by `index`.

        Parameters
        ----------
        index : Player, Strategy, or str
            The part of the profile to return:

            * If `index` is a ``Player``, returns a ``MixedStrategy`` over the player's strategies.
            * If `index` is a ``Strategy``, returns the probability the strategy is played.
            * If `index` is a ``str``, attempts to resolve the referenced object by first searching
              for a player with that label, and then for a strategy with that label.

        Raises
        ------
        MismatchError
            If `player` is a ``Player`` from a different game, or `strategy` is a ``Strategy``
             from a different game.
        """
        self._check_validity()
        if isinstance(index, Strategy):
            if index.game != self.game:
                raise MismatchError("strategy must belong to this game")
            return self._getprob_strategy(index)
        if isinstance(index, Player):
            if index.game != self.game:
                raise MismatchError("player must belong to this game")
            return MixedStrategy.wrap(self, index)
        if isinstance(index, str):
            try:
                return MixedStrategy.wrap(self, self.game._resolve_player(index, "__getitem__"))
            except KeyError:
                pass
            try:
                return self._getprob_strategy(self.game._resolve_strategy(index, "__getitem__"))
            except KeyError:
                raise KeyError(f"no player or strategy with label '{index}'")
        raise TypeError(
            f"profile index must be Player, Strategy, or str, not {index.__class__.__name__}"
        )

    def _setprob_player(self, player: Player, value: typing.Any) -> None:
        """Helper function to set the mixed strategy for a player."""
        if len(value) != len(player.strategies):
            raise ValueError(
                "when setting a mixed strategy, must specify exactly one value per strategy"
            )
        for s, v in zip(player.strategies, value):
            self._setprob_strategy(s, v)

    def __setitem__(
            self,
            index: typing.Union[PlayerReference, StrategyReference],
            value: typing.Any
    ) -> None:
        """Sets a probability or a mixed strategy to `value`.

        Parameters
        ----------
        index : Player, Strategy, or str
            The part of the profile to set:

            * If `index` is a ``Player``, sets the ``MixedStrategy`` over the player's strategies.
            * If `index` is a ``Strategy``, sets the probability the strategy is played.
            * If `index` is a ``str``, attempts to resolve the referenced object by first searching
              for a player with that label, and then for a strategy with that label.

        value
            Any value which can be converted to the data type of the ``MixedStrategyProfile``.

        Raises
        ------
        MismatchError
            If `player` is a ``Player`` from a different game, or `strategy` is a ``Strategy``
            from a different game.
        """
        self._check_validity()
        if isinstance(index, Strategy):
            if index.game != self.game:
                raise MismatchError("strategy must belong to this game")
            self._setprob_strategy(index, value)
            return
        if isinstance(index, Player):
            if index.game != self.game:
                raise MismatchError("player must belong to this game")
            self._setprob_player(index, value)
            return
        if isinstance(index, str):
            try:
                self._setprob_player(self.game._resolve_player(index, "__setitem__"), value)
                return
            except KeyError:
                pass
            try:
                self._setprob_strategy(self.game._resolve_strategy(index, "__setitem__"), value)
            except KeyError:
                raise KeyError(f"no player or strategy with label '{index}'")
            return
        raise TypeError(
            f"profile index must be Player, Strategy, or str, not {index.__class__.__name__}"
        )

    def payoff(self, player: PlayerReference) -> ProfileDType:
        """Returns the expected payoff to a player if all players play
        according to the profile.

        Parameters
        ----------
        player : Player or str
            The player to get the payoff for.  If a string is passed, the
            player is determined by finding the player with that label, if any.

        Raises
        ------
        MismatchError
            If `player` is a `Player` from a different game.
        KeyError
            If `player` is a string and no player in the game has that label.
        """
        self._check_validity()
        return self._payoff(self.game._resolve_player(player, "payoff"))

    def strategy_value(self, strategy: StrategyReference) -> ProfileDType:
        """Returns the expected payoff to playing the strategy, if all other
        players play according to the profile.

        Parameters
        ----------
        strategy : Strategy or str
            The strategy to get the payoff for.  If a string is passed, the
            strategy is determined by finding the strategy with that label, if any.

        Raises
        ------
        MismatchError
            If `strategy` is a `Strategy` from a different game.
        KeyError
            If `strategy` is a string and no strategy in the game has that label.
        """
        self._check_validity()
        return self._strategy_value(self.game._resolve_strategy(strategy, "strategy_value"))

    def strategy_regret(self, strategy: StrategyReference) -> ProfileDType:
        """Returns the regret to playing `strategy`, if all other
        players play according to the profile.

        The regret is defined as the difference between the payoff of the
        best-response strategy and the payoff of `strategy`.  By convention, the
        regret is always non-negative.

        .. versionchanged:: 16.2.0

            Changed from `regret()` to disambiguate from other regret concepts.

        Parameters
        ----------
        strategy : Strategy or str
            The strategy to get the regret for.  If a string is passed, the
            strategy is determined by finding the strategy with that label, if any.

        Raises
        ------
        MismatchError
            If `strategy` is a `Strategy` from a different game.
        KeyError
            If `strategy` is a string and no strategy in the game has that label.

        See Also
        --------
        player_regret
        max_regret
        """
        self._check_validity()
        return self._strategy_regret(self.game._resolve_strategy(strategy, "strategy_regret"))

    def player_regret(self, player: PlayerReference) -> ProfileDType:
        """Returns the regret of `player` for playing their mixed strategy, if all other
        players play according to the profile.

        The regret is defined as the difference between the payoff of the
        best-response strategy and the payoff of the player's mixed strategy.
        By convention, the regret is always non-negative.

        .. versionadded:: 16.2.0

        Parameters
        ----------
        player : Player or str
            The player to get the regret for.  If a string is passed, the
            player is determined by finding the player with that label, if any.

        Raises
        ------
        MismatchError
            If `player` is a `Player` from a different game.
        KeyError
            If `player` is a string and no player in the game has that label.

        See Also
        --------
        strategy_regret
        max_regret
        """
        self._check_validity()
        return self._player_regret(self.game._resolve_player(player, "player_regret"))

    def max_regret(self) -> ProfileDType:
        """Returns the maximum regret of any player.

        A profile is a Nash equilibrium if and only if `max_regret()` is 0.

        .. versionadded:: 16.2.0

        See Also
        --------
        strategy_regret
        player_regret
        liap_value
        """
        self._check_validity()
        return self._max_regret()

    def strategy_value_deriv(self,
                             strategy: StrategyReference,
                             other: StrategyReference) -> ProfileDType:
        """Returns the derivative of the payoff to playing `strategy`, with respect to the
        probability that `other` is played.

        Raises
        ------
        MismatchError
            If `strategy` or `other` is a `Strategy` from a different game.
        KeyError
            If `strategy` or `other` is a string and no strategy in the game has that label.
        """
        self._check_validity()
        return self._strategy_value_deriv(
            self.game._resolve_strategy(strategy, "strategy_value_deriv", "strategy"),
            self.game._resolve_strategy(strategy, "strategy_value_deriv", "other")
        )

    def liap_value(self) -> ProfileDType:
        """Returns the Lyapunov value (see [McK91]_) of the strategy profile.

        The Lyapunov value is a non-negative number which is zero exactly at
        Nash equilibria.

        See Also
        --------
        max_regret
        """
        self._check_validity()
        return self._liap_value()

    def as_behavior(self) -> MixedBehaviorProfile:
        """Creates a mixed behavior profile which is equivalent to this
        mixed strategy profile.

        Returns
        -------
        MixedBehaviorProfile
            The equivalent mixed behavior profile.

        Raises
        ------
        UndefinedOperationError
            If the game does not have a tree representation.
        """
        if not self.game.is_tree:
            raise UndefinedOperationError(
                "Mixed behavior profiles are not defined for strategic games"
            )
        self._check_validity()
        return self._as_behavior()

    def normalize(self) -> MixedStrategyProfile:
        """Create a profile with the same strategy proportions as this
        one, but normalised so probabilities for each player sum to one.
        Requires that all players have non-negative entries that are not all equal to zero.

        Returns
        -------
        MixedStrategyProfile
            The normalized mixed strategy profile.

        Raises
        ------
        ValueError
            If the input mixed strategy of any player is all zero or has a negative entry.
        """
        self._check_validity()
        if self._all_zero_probs():
            raise ValueError(
                "Trying to normalize a MixedStrategyProfile, "
                "but one player's probabilities are all zero"
            )
        if self._negative_prob():
            raise ValueError(
                "Trying to normalize a MixedStrategyProfile, "
                "but a player has a negative probability"
            )
        return self._normalize()

    def copy(self) -> MixedStrategyProfile:
        """Creates a copy of the mixed strategy profile."""
        self._check_validity()
        return self._copy()

    def _all_zero_probs(self) -> bool:
        """Returns True if at least one player has only zero probabilities."""
        return any([all([self._getprob_strategy(s) == 0 for s in p.strategies])
                    for p in self.game.players])

    def _negative_prob(self) -> bool:
        """Returns True if at least one player has a negative probability."""
        return any([any([self._getprob_strategy(s) < 0 for s in p.strategies])
                    for p in self.game.players])


@cython.cclass
class MixedStrategyProfileDouble(MixedStrategyProfile):
    profile = cython.declare(shared_ptr[c_MixedStrategyProfile[double]])

    @staticmethod
    @cython.cfunc
    def wrap(profile: shared_ptr[c_MixedStrategyProfile[float]]) -> MixedStrategyProfileDouble:
        obj: MixedStrategyProfileDouble = (
            MixedStrategyProfileDouble.__new__(MixedStrategyProfileDouble)
        )
        obj.profile = profile
        return obj

    def _check_validity(self) -> None:
        if deref(self.profile).IsInvalidated():
            raise GameStructureChangedError()

    def __len__(self) -> int:
        return deref(self.profile).MixedProfileLength()

    def _getprob_strategy(self, strategy: Strategy) -> float:
        return deref(self.profile).getitem_strategy(strategy.strategy)

    def _setprob_strategy(self, strategy: Strategy, value) -> None:
        setitem_mspd_strategy(deref(self.profile), strategy.strategy, value)

    def _payoff(self, player: Player) -> float:
        return deref(self.profile).GetPayoff(player.player)

    def _strategy_value(self, strategy: Strategy) -> float:
        return deref(self.profile).GetPayoff(strategy.strategy)

    def _strategy_regret(self, strategy: Strategy) -> float:
        return deref(self.profile).GetRegret(strategy.strategy)

    def _player_regret(self, player: Player) -> float:
        return deref(self.profile).GetRegret(player.player)

    def _max_regret(self) -> float:
        return deref(self.profile).GetMaxRegret()

    def _strategy_value_deriv(self, strategy: Strategy, other: Strategy) -> float:
        return deref(self.profile).GetPayoffDeriv(
            strategy.player.number + 1, strategy.strategy, other.strategy
        )

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, MixedStrategyProfileDouble) and
            deref(self.profile) == deref(cython.cast(MixedStrategyProfileDouble, other).profile)
        )

    def _liap_value(self) -> float:
        return deref(self.profile).GetLiapValue()

    def _copy(self) -> MixedStrategyProfileDouble:
        return MixedStrategyProfileDouble.wrap(
            make_shared[c_MixedStrategyProfile[double]](deref(self.profile))
        )

    def _as_behavior(self) -> MixedBehaviorProfileDouble:
        return MixedBehaviorProfileDouble.wrap(
            make_shared[c_MixedBehaviorProfile[double]](deref(self.profile))
        )

    def _normalize(self) -> MixedStrategyProfileDouble:
        return MixedStrategyProfileDouble.wrap(
            make_shared[c_MixedStrategyProfile[double]](deref(self.profile).Normalize())
        )

    @property
    def _game(self) -> Game:
        return Game.wrap(deref(self.profile).GetGame())


@cython.cclass
class MixedStrategyProfileRational(MixedStrategyProfile):
    profile = cython.declare(shared_ptr[c_MixedStrategyProfile[c_Rational]])

    @staticmethod
    @cython.cfunc
    def wrap(
        profile: shared_ptr[c_MixedStrategyProfile[c_Rational]]
    ) -> MixedStrategyProfileRational:
        obj: MixedStrategyProfileRational = (
            MixedStrategyProfileRational.__new__(MixedStrategyProfileRational)
        )
        obj.profile = profile
        return obj

    def _check_validity(self) -> None:
        if deref(self.profile).IsInvalidated():
            raise GameStructureChangedError()

    def __len__(self) -> int:
        return deref(self.profile).MixedProfileLength()

    def _getprob_strategy(self, strategy: Strategy) -> Rational:
        return rat_to_py(deref(self.profile).getitem_strategy(strategy.strategy))

    def _setprob_strategy(self, strategy: Strategy, value) -> None:
        if not isinstance(value, (int, fractions.Fraction)):
            raise TypeError("probability should be int or Fraction instance; received {}"
                            .format(value.__class__.__name__))
        setitem_mspr_strategy(deref(self.profile), strategy.strategy,
                              to_rational(str(value).encode("ascii")))

    def _payoff(self, player: Player) -> Rational:
        return rat_to_py(deref(self.profile).GetPayoff(player.player))

    def _strategy_value(self, strategy: Strategy) -> Rational:
        return rat_to_py(deref(self.profile).GetPayoff(strategy.strategy))

    def _strategy_regret(self, strategy: Strategy) -> Rational:
        return rat_to_py(deref(self.profile).GetRegret(strategy.strategy))

    def _player_regret(self, player: Player) -> Rational:
        return rat_to_py(deref(self.profile).GetRegret(player.player))

    def _max_regret(self) -> Rational:
        return rat_to_py(deref(self.profile).GetMaxRegret())

    def _strategy_value_deriv(self, strategy: Strategy, other: Strategy) -> Rational:
        return rat_to_py(deref(self.profile).GetPayoffDeriv(
            strategy.player.number + 1, strategy.strategy, other.strategy
        ))

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, MixedStrategyProfileRational) and
            deref(self.profile) == deref(cython.cast(MixedStrategyProfileRational, other).profile)
        )

    def _liap_value(self) -> Rational:
        return rat_to_py(deref(self.profile).GetLiapValue())

    def _copy(self) -> MixedStrategyProfileRational:
        return MixedStrategyProfileRational.wrap(
            make_shared[c_MixedStrategyProfile[c_Rational]](deref(self.profile))
        )

    def _as_behavior(self) -> MixedBehaviorProfileRational:
        return MixedBehaviorProfileRational.wrap(
            make_shared[c_MixedBehaviorProfile[c_Rational]](deref(self.profile))
        )

    def _normalize(self) -> MixedStrategyProfileRational:
        return MixedStrategyProfileRational.wrap(
            make_shared[c_MixedStrategyProfile[c_Rational]](deref(self.profile).Normalize())
        )

    @property
    def _game(self) -> Game:
        return Game.wrap(deref(self.profile).GetGame())
