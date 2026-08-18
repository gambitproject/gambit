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
class PayoffVector(PlayerIndexedVector):
    """The expected payoff to each player, one entry per player in a game."""


@cython.cclass
class PlayerRegretVector(PlayerIndexedVector):
    """The regret of each player for playing their mixed strategy, one entry per player."""


@cython.cclass
class StrategyValueVector(StrategyIndexedVector):
    """The expected payoff of playing each strategy, conditional on reaching it, for one
    player's strategies.
    """


@cython.cclass
class StrategyRegretVector(StrategyIndexedVector):
    """The regret of playing each strategy, for one player's strategies."""


@cython.cclass
class StrategyValuesVector(PlayerIndexedVector):
    """The expected payoff of playing each strategy, conditional on reaching it, grouped
    by player; each value is a `StrategyValueVector` for that player's strategies.
    """


@cython.cclass
class StrategyRegretsVector(PlayerIndexedVector):
    """The regret of playing each strategy, grouped by player; each value is a
    `StrategyRegretVector` for that player's strategies.
    """


@cython.cclass
class MixedStrategy:
    """A probability distribution over a player's strategies.

    An immutable snapshot taken from a ``MixedStrategyProfile`` at retrieval time: it
    does not reflect later changes to the profile, and cannot itself be modified. The
    player is accessible via `player`.

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
        return self._values == cython.cast(MixedStrategy, other)._values

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
        """Returns the probability that the strategy with label `strategy` is played.

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
            If no strategy for this player has the label `strategy`.
        """
        try:
            return self._values[strategy]
        except KeyError:
            raise KeyError(f"no strategy with label '{strategy}' for player") from None


@cython.cclass
class MixedStrategyProfile:
    """Represents a mixed strategy profile over the strategies in a ``Game``.

    A mixed strategy profile is a dict-like object, mapping each player in a game to
    a `MixedStrategy` giving the probability distribution over that player's strategies.

    Mixed strategy profiles may represent probabilities as either exact (rational)
    numbers, or floating-point numbers.  These may not be combined in the same mixed
    strategy profile.

    .. versionchanged:: 17.0.0
        Indexing (`__getitem__`/`__setitem__`) is by player label only, operating on a
        player's whole distribution at once; indexing directly by an individual strategy
        is no longer supported.

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
        return str({player.label: self[player.label] for player in self.game.players})

    def _repr_latex_(self) -> str:
        return (
            r"$\left\{" +
            ",".join(
                r"\text{" + player.label + "}:" +
                self[player.label]._repr_latex_().replace("$", "")
                for player in self.game.players
            ) +
            r"\right\}$"
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
        TypeError
            If `player` is not a str.
        """
        self._check_validity()
        resolved_player = self.game._resolve_player(player, "__getitem__")
        values = {s.label: self._getprob_strategy(s) for s in resolved_player.strategies}
        return MixedStrategy.wrap(resolved_player, values)

    def _setprob_player(
        self, player: Player, distribution: collections.abc.Mapping, sparse: bool
    ) -> None:
        """Validates and sets the whole mixed strategy for player.

        Every key of `distribution` must be one of the player's strategy labels. If
        `sparse` is True, strategies `distribution` omits are treated as having weight
        zero; if False, `distribution` must specify a weight for every strategy. Weights
        must be non-negative and not all zero. Weights need not sum to one; see
        `normalize`.
        """
        if not isinstance(distribution, collections.abc.Mapping):
            raise TypeError(
                f"a mixed strategy must be set from a Mapping from strategy label to "
                f"weight, not {distribution.__class__.__name__}"
            )
        labels = {s.label for s in player.strategies}
        given = set(distribution.keys())
        unknown = given - labels
        if unknown:
            raise ValueError(
                f"not a strategy label for this player: {', '.join(sorted(unknown))}"
            )
        if not sparse and given != labels:
            raise ValueError(
                "a distribution must specify exactly one weight for each of the "
                "player's strategies, by label, unless sparse=True"
            )
        zero = self._to_prob(0)
        values = {label: zero for label in labels}
        values.update({label: self._to_prob(weight) for label, weight in distribution.items()})
        if any(v < 0 for v in values.values()):
            raise ValueError("a mixed strategy's weights must be non-negative")
        if all(v == 0 for v in values.values()):
            raise ValueError("a mixed strategy's weights must not all be zero")
        for s in player.strategies:
            self._setprob_strategy(s, values[s.label])

    def __setitem__(self, player: str, distribution: collections.abc.Mapping) -> None:
        """Sets the mixed strategy for the player with label `player`.

        `distribution` need not specify a weight for every one of the player's
        strategies: strategies it omits are treated as having weight zero. Use
        `set_strategy` if you want that to be an error instead.

        Parameters
        ----------
        player : str
            The label of the player whose mixed strategy is to be set.
        distribution : Mapping[str, Any]
            A non-negative weight for some or all of the player's strategies, keyed by
            strategy label; strategies it omits are treated as having weight zero. A
            weight may be any value Gambit can interpret as a number (`int`, `float`,
            `str`, `Decimal`, or `Rational`). Weights need not sum to one, and at least
            one must be nonzero.

        Raises
        ------
        KeyError
            If no player in the game has the label `player`.
        TypeError
            If `player` is not a str, or `distribution` is not a Mapping.
        ValueError
            If any key of `distribution` is not one of the player's strategy labels, if
            any weight cannot be interpreted as a number, if any weight is negative, or
            if the weights are all zero.

        See Also
        --------
        set_strategy
            Equivalent, but can require a weight for every strategy instead of silently
            defaulting omitted ones to zero.
        """
        self._check_validity()
        resolved_player = self.game._resolve_player(player, "__setitem__")
        self._setprob_player(resolved_player, distribution, sparse=True)

    def set_strategy(
        self, player: str, distribution: collections.abc.Mapping, sparse: bool = False
    ) -> None:
        """Sets the mixed strategy for the player with label `player`.

        Equivalent to ``profile[player] = distribution``, except that by default every
        one of the player's strategies must be given an explicit weight in
        `distribution`. Use this instead of `__setitem__` when omitting a strategy
        should be an error rather than silently defaulting its weight to zero.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        player : str
            The label of the player whose mixed strategy is to be set.
        distribution : Mapping[str, Any]
            A non-negative weight for the player's strategies, keyed by strategy label.
            A weight may be any value Gambit can interpret as a number (`int`, `float`,
            `str`, `Decimal`, or `Rational`). Weights need not sum to one, and at least
            one must be nonzero.
        sparse : bool, default False
            If False (the default), `distribution` must specify a weight for every one
            of the player's strategies. If True, strategies it omits are treated as
            having weight zero, the same as ``profile[player] = distribution``.

        Raises
        ------
        KeyError
            If no player in the game has the label `player`.
        TypeError
            If `player` is not a str, or `distribution` is not a Mapping.
        ValueError
            If any key of `distribution` is not one of the player's strategy labels, if
            `sparse` is False and `distribution` omits a strategy, if any weight cannot
            be interpreted as a number, if any weight is negative, or if the weights are
            all zero.

        See Also
        --------
        __setitem__
        """
        self._check_validity()
        resolved_player = self.game._resolve_player(player, "set_strategy")
        self._setprob_player(resolved_player, distribution, sparse=sparse)

    @property
    def payoffs(self) -> PayoffVector:
        """Returns the expected payoff to each player, if all players play according to
        the profile.
        """
        self._check_validity()
        return PayoffVector({p.label: self._payoff(p) for p in self.game.players})

    @property
    def strategy_values(self) -> StrategyValuesVector:
        """Returns the expected payoff of playing each strategy, conditional on reaching
        it, if all other players play according to the profile, grouped by player.
        """
        self._check_validity()
        return StrategyValuesVector({
            p.label: StrategyValueVector({s.label: self._strategy_value(s) for s in p.strategies})
            for p in self.game.players
        })

    @property
    def strategy_regrets(self) -> StrategyRegretsVector:
        """Returns the regret to playing each strategy, if all other players play
        according to the profile, grouped by player.

        .. versionchanged:: 16.2.0

            Changed from `regret()` to disambiguate from other regret concepts.

        See Also
        --------
        player_regrets
        max_regret
        """
        self._check_validity()
        return StrategyRegretsVector({
            p.label: StrategyRegretVector(
                {s.label: self._strategy_regret(s) for s in p.strategies}
            )
            for p in self.game.players
        })

    @property
    def player_regrets(self) -> PlayerRegretVector:
        """Returns the regret of each player for playing their mixed strategy, if all
        other players play according to the profile.

        See Also
        --------
        strategy_regrets
        max_regret
        """
        self._check_validity()
        return PlayerRegretVector({p.label: self._player_regret(p) for p in self.game.players})

    def max_regret(self) -> ProfileDType:
        """Returns the maximum regret of any player.

        A profile is a Nash equilibrium if and only if `max_regret()` is 0.

        .. versionadded:: 16.2.0

        See Also
        --------
        strategy_regrets
        player_regrets
        liap_value
        """
        self._check_validity()
        return self._max_regret()

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
        return len(self.game.players)

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
        return len(self.game.players)

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
