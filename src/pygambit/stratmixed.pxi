#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

    A ``MixedStrategy`` is an immutable snapshot of the component of a
    ``MixedStrategyProfile`` associated with a given ``Player``, taken at the moment it
    was retrieved from the profile.  It does not reflect any later changes to the
    profile, and cannot itself be modified; the player for whom the ``MixedStrategy``
    applies is accessible via `player`.

    .. versionchanged:: 17.0.0

        No longer a live view onto the profile: holds its own copy of the probabilities,
        and can no longer be assigned into. Set a player's whole distribution via
        ``MixedStrategyProfile.__setitem__`` instead.
    """
    _player = cython.declare(Player)
    _values = cython.declare(dict)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a MixedStrategy outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(player: Player, values: dict) -> MixedStrategy:
        obj: MixedStrategy = MixedStrategy.__new__(MixedStrategy)
        obj._player = player
        obj._values = values
        return obj

    @property
    def player(self) -> Player:
        """The player for whom this mixed strategy is defined."""
        return self._player

    def __repr__(self) -> str:
        return str(self._values)

    def _repr_latex_(self) -> str:
        values = list(self._values.values())
        if not values or not hasattr(values[0], "_repr_latex_"):
            return repr(self)
        return (
            r"$\left\{" +
            ",".join(
                r"\text{" + label + "}:" + value._repr_latex_().replace("$", "")
                for label, value in self._values.items()
            ) +
            r"\right\}$"
        )

    def __eq__(self, other: typing.Any) -> bool:
        if isinstance(other, collections.abc.Mapping):
            return self._values == dict(other)
        if not isinstance(other, MixedStrategy) or self.player != other.player:
            return False
        return self._values == other._values

    def __len__(self) -> int:
        return len(self._values)

    def __iter__(self) -> typing.Iterator[typing.Tuple[str, ProfileDType], None, None]:
        """Iterate over the probabilities assigned to strategies by the mixed strategy.

        .. versionchanged:: 17.0.0

            Yields the strategy's label instead of the ``Strategy`` object.

        Yields
        ------
        label : str
            The label of a strategy for the player
        probability: float or Rational
            The probability the mixed strategy assigns to the strategy being played
        """
        yield from self._values.items()

    def __getitem__(self, strategy: str) -> ProfileDType:
        """Returns the probability that the strategy with label `index` is played.

        Parameters
        ----------
        strategy : str
            The label of the strategy to look up.

        Returns
        -------
        float or Rational
            The probability assigned to the strategy.

        Raises
        ------
        KeyError
            If no strategy for this player has the label `index`.
        """
        try:
            return self._values[strategy]
        except KeyError:
            raise KeyError(f"no strategy with label '{strategy}' for player") from None


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
        return str([self[player.label] for player in self.game.players])

    def _repr_latex_(self) -> str:
        return (
            r"$\left[" +
            ",".join([self[player.label]._repr_latex_().replace("$", "")
                      for player in self.game.players]) +
            r"\right]$"
        )

    @property
    def game(self) -> Game:
        """The game on which this mixed strategy profile is defined."""
        return self._game

    def __iter__(self) -> typing.Iterator[MixedStrategy, None, None]:
        """Iterate over the mixed strategies in the profile, one per player.

        .. versionchanged:: 17.0.0

            Previously yielded `(Strategy, probability)` pairs flattened across every
            player's strategies; now yields the `MixedStrategy` for each player, matching
            what `mixed_strategies()` returned.

        Yields
        ------
        strategy : MixedStrategy
            The player's mixed strategy specified in the profile
        """
        for player in self.game.players:
            yield self[player.label]

    def __getitem__(self, player: str) -> MixedStrategy:
        """Returns a snapshot of the mixed strategy for the player with label `player`,
        as of now; it will not reflect any later changes to this profile.

        Parameters
        ----------
        player : str
            The label of the player to look up.

        Raises
        ------
        KeyError
            If no player in the game has the label `player`.
        """
        self._check_validity()
        resolved_player = self.game._resolve_player(player, "__getitem__")
        values = {s.label: self._getprob_strategy(s) for s in resolved_player.strategies}
        return MixedStrategy.wrap(resolved_player, values)

    def _setprob_player(self, player: Player, distribution: collections.abc.Mapping) -> None:
        """Validates and sets the whole mixed strategy for player.

        `distribution` must specify a non-negative weight for each of the player's
        strategies, by label, and the weights must not all be zero. Weights need not sum
        to one; see `normalize`.
        """
        if not isinstance(distribution, collections.abc.Mapping):
            raise TypeError(
                f"a mixed strategy must be set from a Mapping from strategy label to "
                f"weight, not {distribution.__class__.__name__}"
            )
        labels = {s.label for s in player.strategies}
        if set(distribution.keys()) != labels:
            raise ValueError(
                "a distribution must specify exactly one weight for each of the "
                "player's strategies, by label"
            )
        values = {label: self._to_prob(weight) for label, weight in distribution.items()}
        if any(v < 0 for v in values.values()):
            raise ValueError("a mixed strategy's weights must be non-negative")
        if all(v == 0 for v in values.values()):
            raise ValueError("a mixed strategy's weights must not all be zero")
        for s in player.strategies:
            self._setprob_strategy(s, values[s.label])

    def __setitem__(self, player: str, distribution: collections.abc.Mapping) -> None:
        """Sets the mixed strategy for the player with label `player`.

        Parameters
        ----------
        player : str
            The label of the player whose mixed strategy is to be set.
        distribution : Mapping[str, Any]
            A non-negative, not-all-zero weight for each of the player's strategies, keyed
            by strategy label. A weight may be any value Gambit can interpret as a number
            (`int`, `float`, `str`, `Decimal`, or `Rational`); weights need not sum to one.

        Raises
        ------
        KeyError
            If no player in the game has the label `player`.
        ValueError
            If `distribution` does not specify exactly one weight for each of the
            player's strategies, if any weight is negative, or if the weights are all zero.
        """
        self._check_validity()
        resolved_player = self.game._resolve_player(player, "__setitem__")
        self._setprob_player(resolved_player, distribution)

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
        """Returns the Lyapunov value (see :cite:p:`McK91`) of the strategy profile.

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

        .. versionchanged:: 17.0.0

            No longer validates that entries are non-negative and not all zero for each
            player: assigning a mixed strategy (`__setitem__`) now enforces this at the
            point of assignment, so any `MixedStrategyProfile` already satisfies it.

        Returns
        -------
        MixedStrategyProfile
            The normalized mixed strategy profile.
        """
        self._check_validity()
        return self._normalize()

    def copy(self) -> MixedStrategyProfile:
        """Creates a copy of the mixed strategy profile.

        .. versionchanged:: 17.0.0

            The copy shares its underlying data with the original until one of them is
            next assigned into, at which point the one being assigned into transparently
            takes its own private copy first. Both profiles are fully independent from
            each other's perspective; this only affects when the underlying duplication
            happens, not whether it happens.
        """
        self._check_validity()
        return self._copy()

    # The public API above is implemented once here and dispatches to the hooks below,
    # each of which is implemented by a concrete dtype-specific subclass
    # (MixedStrategyProfileDouble/MixedStrategyProfileRational).

    def _check_validity(self) -> None:
        """Raises GameStructureChangedError if the game has structurally changed since
        this profile was created.
        """
        raise NotImplementedError

    @property
    def _game(self) -> Game:
        """The game on which this profile is defined."""
        raise NotImplementedError

    def _getprob_strategy(self, strategy: Strategy) -> ProfileDType:
        """Returns the probability with which strategy is played."""
        raise NotImplementedError

    def _setprob_strategy(self, strategy: Strategy, value: typing.Any) -> None:
        """Sets the probability with which strategy is played."""
        raise NotImplementedError

    def _to_prob(self, value: typing.Any) -> ProfileDType:
        """Coerces value (int, float, str, Decimal, or Rational) into this profile's
        native probability type.
        """
        raise NotImplementedError

    def _payoff(self, player: Player) -> ProfileDType:
        """Returns the expected payoff to player."""
        raise NotImplementedError

    def _strategy_value(self, strategy: Strategy) -> ProfileDType:
        """Returns the expected payoff to playing strategy."""
        raise NotImplementedError

    def _strategy_regret(self, strategy: Strategy) -> ProfileDType:
        """Returns the regret to playing strategy."""
        raise NotImplementedError

    def _player_regret(self, player: Player) -> ProfileDType:
        """Returns the regret of player for playing their mixed strategy."""
        raise NotImplementedError

    def _max_regret(self) -> ProfileDType:
        """Returns the maximum regret of any player."""
        raise NotImplementedError

    def _strategy_value_deriv(self, strategy: Strategy, other: Strategy) -> ProfileDType:
        """Returns the derivative of the payoff to strategy with respect to the
        probability that other is played.
        """
        raise NotImplementedError

    def _liap_value(self) -> ProfileDType:
        """Returns the Lyapunov value of the profile."""
        raise NotImplementedError

    def _copy(self) -> MixedStrategyProfile:
        """Creates a copy of the profile."""
        raise NotImplementedError

    def _as_behavior(self) -> MixedBehaviorProfile:
        """Creates the equivalent mixed behavior profile."""
        raise NotImplementedError

    def _normalize(self) -> MixedStrategyProfile:
        """Creates a copy of the profile, normalized so each player's strategy
        probabilities sum to one.
        """
        raise NotImplementedError


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
        if deref(self.profile).HasOutdatedGameVersion():
            raise GameStructureChangedError()

    def __len__(self) -> int:
        return deref(self.profile).MixedProfileLength()

    def _getprob_strategy(self, strategy: Strategy) -> float:
        return deref(self.profile).getitem_strategy(strategy.strategy)

    @cython.cfunc
    def _ensure_unshared(self) -> cython.void:
        """Clones the underlying profile if it is shared with another wrapper, so that
        the mutation about to happen is not observed by any other MixedStrategyProfile.
        """
        if self.profile.use_count() != 1:
            self.profile = make_shared[c_MixedStrategyProfile[double]](deref(self.profile))

    def _setprob_strategy(self, strategy: Strategy, value) -> None:
        self._ensure_unshared()
        setitem_mspd_strategy(deref(self.profile), strategy.strategy, value)

    def _to_prob(self, value: typing.Any) -> float:
        normalized = _to_number_string(value)
        try:
            return float(normalized)
        except ValueError:
            # normalized is a fraction-form string (e.g. "1/2"), which float() rejects
            return float(Rational(normalized))

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
        # Copy-on-write: share the underlying profile; _ensure_unshared() clones it
        # lazily, the first time either this copy or the original is next mutated.
        return MixedStrategyProfileDouble.wrap(self.profile)

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
        if deref(self.profile).HasOutdatedGameVersion():
            raise GameStructureChangedError()

    def __len__(self) -> int:
        return deref(self.profile).MixedProfileLength()

    def _getprob_strategy(self, strategy: Strategy) -> Rational:
        return rat_to_py(deref(self.profile).getitem_strategy(strategy.strategy))

    @cython.cfunc
    def _ensure_unshared(self) -> cython.void:
        """Clones the underlying profile if it is shared with another wrapper, so that
        the mutation about to happen is not observed by any other MixedStrategyProfile.
        """
        if self.profile.use_count() != 1:
            self.profile = make_shared[c_MixedStrategyProfile[c_Rational]](deref(self.profile))

    def _setprob_strategy(self, strategy: Strategy, value) -> None:
        if not isinstance(value, (int, fractions.Fraction)):
            raise TypeError("probability should be int or Fraction instance; received {}"
                            .format(value.__class__.__name__))
        self._ensure_unshared()
        setitem_mspr_strategy(deref(self.profile), strategy.strategy,
                              to_rational(str(value).encode("ascii")))

    def _to_prob(self, value: typing.Any) -> Rational:
        return Rational(_to_number_string(value))

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
        # Copy-on-write: share the underlying profile; _ensure_unshared() clones it
        # lazily, the first time either this copy or the original is next mutated.
        return MixedStrategyProfileRational.wrap(self.profile)

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
