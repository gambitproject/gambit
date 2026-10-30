#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/baggprofile.pxi
# Mixed behavior profiles of Bayesian action-graph games
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
class BAGGInfosetProbVector(PlayerIndexedVector):
    """The probability of each information set (type) of each player of a Bayesian game.

    Maps each player label to a dict from information set label to probability.

    .. versionadded:: 17.0.0
    """


@cython.cclass
class BAGGInfosetValueVector(PlayerIndexedVector):
    """The expected payoff to each player conditional on each of its types.

    Maps each player label to a dict from information set label to value.

    .. versionadded:: 17.0.0
    """


@cython.cclass
class BAGGInfosetRegretVector(PlayerIndexedVector):
    """The regret of each player at each of its types.

    Maps each player label to a dict from information set label to regret.

    .. versionadded:: 17.0.0
    """


@cython.cclass
class BAGGActionValuesVector(PlayerIndexedVector):
    """The expected payoff of playing each action conditional on each type of each player.

    Maps each player label to a dict from information set label to an
    `ActionValueVector` of the actions available at that information set.

    .. versionadded:: 17.0.0
    """


@cython.cclass
class BAGGActionRegretsVector(PlayerIndexedVector):
    """The regret of playing each action at each type of each player.

    Maps each player label to a dict from information set label to an
    `ActionRegretVector` of the actions available at that information set.

    .. versionadded:: 17.0.0
    """


@cython.cclass
class BAGGMixedBehavior:
    """A set of probability distributions describing a player's behavior at each of its
    types.

    An immutable snapshot taken from a `BAGGMixedBehaviorProfile` at retrieval time: it
    does not reflect later changes to the profile, and cannot itself be modified.

    .. versionadded:: 17.0.0
    """
    _player = cython.declare(str)
    _values = cython.declare(dict)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a BAGGMixedBehavior outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(player: str, values: dict) -> BAGGMixedBehavior:
        obj: BAGGMixedBehavior = BAGGMixedBehavior.__new__(BAGGMixedBehavior)
        obj._player = player
        obj._values = values
        return obj

    @property
    def player(self) -> str:
        """The label of the player for whom this mixed behavior is defined."""
        return self._player

    def __repr__(self) -> str:
        return str(list(self._values.values()))

    def __eq__(self, other: typing.Any) -> bool:
        values = list(self._values.values())
        if isinstance(other, list):
            return values == other
        if not isinstance(other, BAGGMixedBehavior) or self.player != other.player:
            return False
        return values == list(cython.cast(BAGGMixedBehavior, other)._values.values())

    def __len__(self) -> int:
        return len(self._values)

    def __iter__(self) -> typing.Iterator[tuple[str, MixedAction], None, None]:
        """Iterate over the mixed actions at the player's information sets.

        Yields
        ------
        infoset : str
            The label of an information set of the player
        mixed_action : MixedAction
            The distribution over actions at that information set
        """
        yield from self._values.items()

    def __getitem__(self, infoset: str) -> MixedAction:
        """Returns the mixed action at the information set with label `infoset`.

        Raises
        ------
        KeyError
            If the player has no information set with label `infoset`.
        """
        try:
            return self._values[infoset]
        except KeyError:
            raise KeyError(
                f"player '{self._player}' has no information set with label '{infoset}'"
            ) from None


@cython.cclass
class BAGGMixedBehaviorProfile:
    """A mixed behavior profile of a Bayesian action-graph game.

    The profile gives, for each player and each of its types (presented as information
    sets), a probability distribution over the actions available at that type.  Players,
    information sets and actions are identified by label.  Quantities reported for a
    player are ex-ante: they average over the player's own type, and over the types of
    the other players, using the type priors.

    Probabilities may be represented as either exact (rational) numbers, or floating-point
    numbers.  These may not be combined in the same profile.

    This is not a `MixedBehaviorProfile` and has no tree structure: there are no
    realization probabilities, beliefs or node values, and a `MixedAction` taken from it
    has no `history`.

    .. versionadded:: 17.0.0

    See Also
    --------
    BayesianActionGraphGame.mixed_behavior_profile
        Creates a new profile on a game.
    """
    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a BAGGMixedBehaviorProfile outside a Game.")

    def __repr__(self) -> str:
        return str([self[player] for player in self.game.players])

    @property
    def game(self) -> Game:
        """The game on which this profile is defined."""
        return self._game

    def __len__(self) -> int:
        return len(self.game.players)

    def __iter__(self) -> typing.Iterator[BAGGMixedBehavior, None, None]:
        """Iterate over the players' mixed behaviors, in the order of the game's players."""
        for player in self.game.players:
            yield self[player]

    @cython.cfunc
    def _bagg(self) -> BayesianActionGraphGame:
        return cython.cast(BayesianActionGraphGame, self._game)

    def _action_labels(self, player: str, infoset: str) -> list[str]:
        """The labels of the actions at `player`'s information set `infoset`."""
        agent: c_GamePlayer = self._bagg()._agent_player(player, infoset)
        return [
            strategy.deref().GetLabel().decode("utf-8")
            for strategy in agent.deref().GetStrategies()
        ]

    @cython.cfunc
    def _strategy(self, player: str, infoset: str, action: str) -> c_GameStrategy:
        agent: c_GamePlayer = self._bagg()._agent_player(player, infoset)
        for strategy in agent.deref().GetStrategies():
            if strategy.deref().GetLabel().decode("utf-8") == action:
                return strategy
        raise KeyError(
            f"player '{player}' has no action with label '{action}' "
            f"at information set '{infoset}'"
        )

    @staticmethod
    def _parse_index(index: typing.Any) -> tuple[str, str]:
        if not (isinstance(index, tuple) and len(index) == 2):
            raise TypeError(
                "profile index must be a player label, or a (player, information set) pair"
            )
        return index

    def _mixed_action(self, player: str, infoset: str) -> MixedAction:
        return MixedAction.wrap(None, {
            action: self._getprob(player, infoset, action)
            for action in self._action_labels(player, infoset)
        })

    def __getitem__(self, index: typing.Any) -> BAGGMixedBehavior | MixedAction:
        """Access a component of the mixed behavior profile specified by `index`.

        Parameters
        ----------
        index : str or tuple of str
            The part of the profile to return:

            * If `index` is a ``str``, returns a `BAGGMixedBehavior` over the player's
              information sets.
            * If `index` is a pair ``(player, infoset)``, returns the `MixedAction` over
              the actions at that information set.

        Raises
        ------
        KeyError
            If there is no player, information set, or action with the given label.
        TypeError
            If `index` is not a ``str`` or a pair of ``str``.
        """
        self._check_validity()
        if isinstance(index, str):
            return BAGGMixedBehavior.wrap(index, {
                infoset: self._mixed_action(index, infoset)
                for infoset in self.game.get_infosets(index)
            })
        player, infoset = self._parse_index(index)
        return self._mixed_action(player, infoset)

    def __setitem__(self, index: tuple[str, str], distribution: collections.abc.Mapping) -> None:
        """Sets the mixed action at information set `infoset` of player `player`.

        `distribution` need not specify a weight for every action: actions it omits are
        treated as having weight zero.

        Parameters
        ----------
        index : tuple of str
            The pair ``(player, infoset)`` of labels.
        distribution : Mapping[str, Any]
            A non-negative weight for some or all of the information set's actions,
            keyed by action label.  A weight may be any value Gambit can interpret as a
            number (`int`, `float`, `str`, `Decimal`, or `Rational`).  Weights need not
            sum to one, and at least one must be nonzero.

        Raises
        ------
        KeyError
            If there is no such player or information set.
        TypeError
            If `index` is not a pair of ``str``, or `distribution` is not a Mapping.
        ValueError
            If any key of `distribution` is not an action label at the information set; if
            any weight cannot be interpreted as a number or is negative; or if the weights
            are all zero.
        """
        self._check_validity()
        player, infoset = self._parse_index(index)
        if not isinstance(distribution, collections.abc.Mapping):
            raise TypeError(
                f"a mixed action must be set from a Mapping from action label to "
                f"weight, not {distribution.__class__.__name__}"
            )
        labels = self._action_labels(player, infoset)
        unknown = set(distribution.keys()) - set(labels)
        if unknown:
            raise ValueError(
                f"not an action label at this information set: {', '.join(sorted(unknown))}"
            )
        zero = self._to_prob(0)
        values = {label: zero for label in labels}
        values.update({label: self._to_prob(weight) for label, weight in distribution.items()})
        if any(v < 0 for v in values.values()):
            raise ValueError("a mixed action's weights must be non-negative")
        if all(v == 0 for v in values.values()):
            raise ValueError("a mixed action's weights must not all be zero")
        for label, value in values.items():
            self._setprob(player, infoset, label, value)

    def _prior(self, player: str, infoset: str) -> ProfileDType:
        raise NotImplementedError

    def _coerce(self, value: typing.Any) -> ProfileDType:
        """Converts a computed number into this profile's native type."""
        raise NotImplementedError

    def _types(self, player: str) -> list[tuple[str, ProfileDType]]:
        return [
            (infoset, self._prior(player, infoset))
            for infoset in self.game.get_infosets(player)
        ]

    def _player_payoff(self, player: str) -> ProfileDType:
        return self._coerce(sum(
            (prior * self._agent_payoff(player, infoset)
             for infoset, prior in self._types(player)),
            self._to_prob(0)
        ))

    def _player_regret(self, player: str) -> ProfileDType:
        return self._coerce(sum(
            (prior * self._agent_regret(player, infoset)
             for infoset, prior in self._types(player)),
            self._to_prob(0)
        ))

    @property
    def payoffs(self) -> PayoffVector:
        """Returns the ex-ante expected payoff to each player, if all players play
        according to the profile.

        This is the average over the player's own type, using the type priors, of its
        payoff conditional on that type.
        """
        self._check_validity()
        return PayoffVector({p: self._player_payoff(p) for p in self.game.players})

    @property
    def infoset_probs(self) -> BAGGInfosetProbVector:
        """Returns the probability of each information set (type) of each player.

        These are the type priors.
        """
        self._check_validity()
        return BAGGInfosetProbVector({
            player: dict(self._types(player)) for player in self.game.players
        })

    @property
    def infoset_values(self) -> BAGGInfosetValueVector:
        """Returns the expected payoff to each player conditional on being of each of
        its types, if all players play according to the profile.

        See Also
        --------
        infoset_regrets
        payoffs
        """
        self._check_validity()
        return BAGGInfosetValueVector({
            player: {
                infoset: self._agent_payoff(player, infoset)
                for infoset in self.game.get_infosets(player)
            }
            for player in self.game.players
        })

    @property
    def infoset_regrets(self) -> BAGGInfosetRegretVector:
        """Returns the regret to each player of playing its mixed action at each of its
        types, if all other players play according to the profile.

        The regret is the difference between the payoff of the best-response action and
        the payoff of the mixed action, conditional on the player being of that type.
        By convention, the regret is always non-negative.

        See Also
        --------
        action_regrets
        agent_max_regret
        """
        self._check_validity()
        return BAGGInfosetRegretVector({
            player: {
                infoset: self._agent_regret(player, infoset)
                for infoset in self.game.get_infosets(player)
            }
            for player in self.game.players
        })

    @property
    def action_values(self) -> BAGGActionValuesVector:
        """Returns the expected payoff to each player of playing each action,
        conditional on being of each of its types, if all players play according to the
        profile.

        The result is indexed by player, then information set, and gives a mapping from
        action label to value.

        See Also
        --------
        infoset_values
        action_regrets
        """
        self._check_validity()
        return BAGGActionValuesVector({
            player: {
                infoset: ActionValueVector({
                    action: self._agent_action_value(player, infoset, action)
                    for action in self._action_labels(player, infoset)
                })
                for infoset in self.game.get_infosets(player)
            }
            for player in self.game.players
        })

    @property
    def action_regrets(self) -> BAGGActionRegretsVector:
        """Returns the regret to each player of playing each action, conditional on being
        of each of its types, if all other players play according to the profile.

        The regret of an action is the difference between the payoff of the best-response
        action and the payoff of the action.  By convention, the regret is always
        non-negative.  The result is indexed by player, then information set, and gives
        a mapping from action label to regret.

        See Also
        --------
        infoset_regrets
        action_values
        """
        self._check_validity()
        return BAGGActionRegretsVector({
            player: {
                infoset: ActionRegretVector({
                    action: self._agent_action_regret(player, infoset, action)
                    for action in self._action_labels(player, infoset)
                })
                for infoset in self.game.get_infosets(player)
            }
            for player in self.game.players
        })

    def agent_liap_value(self) -> ProfileDType:
        """Returns the Lyapunov value (see :cite:p:`McK91`) of the profile, computed
        from the regrets conditional on each type.

        The agent Lyapunov value is a non-negative number which is zero exactly at
        Nash equilibria of the Bayesian game (for types with positive prior).

        See Also
        --------
        agent_max_regret
        liap_value
        """
        self._check_validity()
        return self._agent_liap_value()

    def liap_value(self) -> ProfileDType:
        """Returns the Lyapunov value (see :cite:p:`McK91`) of the profile, computed
        from the ex-ante regrets.

        This is the sum, over each player, type and action, of the squared positive part
        of the action's value in excess of the type's value, with each excess weighted
        by the prior probability of the type.  It is a non-negative number which is
        zero exactly at Nash equilibria.

        See Also
        --------
        max_regret
        agent_liap_value
        """
        self._check_validity()
        zero = self._to_prob(0)
        total = zero
        for player in self.game.players:
            for infoset, prior in self._types(player):
                value = self._agent_payoff(player, infoset)
                for action in self._action_labels(player, infoset):
                    excess = self._agent_action_value(player, infoset, action) - value
                    if excess > zero:
                        total += (prior * excess) ** 2
        return self._coerce(total)

    def agent_max_regret(self) -> ProfileDType:
        """Returns the maximum regret at any information set.

        The regret at an information set is conditional on the player being of that type.
        A profile is a Nash equilibrium of the Bayesian game if and only if
        `agent_max_regret()` is 0, for types with positive prior.

        See Also
        --------
        max_regret
        """
        self._check_validity()
        return max(
            (self._agent_regret(player, infoset)
             for player in self.game.players for infoset in self.game.get_infosets(player)),
            default=self._to_prob(0)
        )

    def max_regret(self) -> ProfileDType:
        """Returns the maximum regret of any player.

        The regret of a player is the ex-ante gain, weighted by the type priors, from the
        player's best response.  A profile is a Nash equilibrium if and only if
        `max_regret()` is 0.

        See Also
        --------
        agent_max_regret
        """
        self._check_validity()
        return max(
            (self._player_regret(player) for player in self.game.players),
            default=self._to_prob(0)
        )

    def as_float(self) -> BAGGMixedBehaviorProfileDouble:
        """Creates a floating-point copy of this profile.

        If this profile is already a `BAGGMixedBehaviorProfileDouble`, returns a copy of it.
        """
        self._check_validity()
        return self._as_float()

    def normalize(self) -> BAGGMixedBehaviorProfile:
        """Create a profile with the same action proportions as this
        one, but normalised so probabilities for each information set sum to one.
        """
        self._check_validity()
        return self._normalize()

    def copy(self) -> BAGGMixedBehaviorProfile:
        """Creates a copy of the profile.

        The copy shares its underlying data with the original until one of them is
        next assigned into, at which point the one being assigned into transparently
        takes its own private copy first.
        """
        self._check_validity()
        return self._copy()

    # Implemented by the dtype-specific subclasses below.

    def _check_validity(self) -> None:
        raise NotImplementedError

    @property
    def _game(self) -> Game:
        raise NotImplementedError

    def _to_prob(self, value: typing.Any) -> ProfileDType:
        raise NotImplementedError

    def _getprob(self, player: str, infoset: str, action: str) -> ProfileDType:
        raise NotImplementedError

    def _setprob(self, player: str, infoset: str, action: str, value: ProfileDType) -> None:
        raise NotImplementedError

    def _agent_payoff(self, player: str, infoset: str) -> ProfileDType:
        """The expected payoff to `player` conditional on being of type `infoset`."""
        raise NotImplementedError

    def _agent_regret(self, player: str, infoset: str) -> ProfileDType:
        """The regret of `player` conditional on being of type `infoset`."""
        raise NotImplementedError

    def _agent_action_value(self, player: str, infoset: str, action: str) -> ProfileDType:
        """The expected payoff to `player` of `action`, conditional on being of type
        `infoset`.
        """
        raise NotImplementedError

    def _agent_action_regret(self, player: str, infoset: str, action: str) -> ProfileDType:
        """The regret of `action` for `player`, conditional on being of type `infoset`."""
        raise NotImplementedError

    def _agent_liap_value(self) -> ProfileDType:
        raise NotImplementedError

    def _copy(self) -> BAGGMixedBehaviorProfile:
        raise NotImplementedError

    def _as_float(self) -> BAGGMixedBehaviorProfileDouble:
        raise NotImplementedError

    def _normalize(self) -> BAGGMixedBehaviorProfile:
        raise NotImplementedError


@cython.cclass
class BAGGMixedBehaviorProfileDouble(BAGGMixedBehaviorProfile):
    """A `BAGGMixedBehaviorProfile` with floating-point probabilities.

    .. versionadded:: 17.0.0
    """
    profile = cython.declare(shared_ptr[c_MixedStrategyProfile[double]])

    def _payoff_deriv_block_discrepancy(self, removed: list[tuple[str, str, str]]) -> float:
        # Private, for testing: the largest discrepancy between the agent-form payoff
        # derivative blocks and the same quantities computed one at a time, on the support
        # without the actions `removed`, given as (player, information set, action) labels.
        cdef stdvector[c_GameStrategy] handles
        for player, infoset, action in removed:
            handles.push_back(self._strategy(player, infoset, action))
        return PayoffDerivBlockDiscrepancy(deref(self.profile), handles)

    def _payoff_deriv_finite_difference_discrepancy(self, step: float) -> float:
        # Private, for testing: the largest discrepancy between the agent-form partial
        # derivatives of payoffs and central differences of the payoffs with the given step.
        return PayoffDerivFiniteDifferenceDiscrepancy(deref(self.profile), step)

    @staticmethod
    @cython.cfunc
    def wrap(
        profile: shared_ptr[c_MixedStrategyProfile[float]]
    ) -> BAGGMixedBehaviorProfileDouble:
        obj: BAGGMixedBehaviorProfileDouble = (
            BAGGMixedBehaviorProfileDouble.__new__(BAGGMixedBehaviorProfileDouble)
        )
        obj.profile = profile
        return obj

    def _check_validity(self) -> None:
        if deref(self.profile).HasOutdatedGameVersion():
            raise GameStructureChangedError()

    @property
    def _game(self) -> Game:
        return Game._wrap(deref(self.profile).GetGame())

    def _to_prob(self, value: typing.Any) -> float:
        normalized = _to_number_string(value)
        try:
            return float(normalized)
        except ValueError:
            return float(Rational(normalized))

    def _coerce(self, value: typing.Any) -> float:
        return float(value)

    def _prior(self, player: str, infoset: str) -> float:
        return float(rat_to_py(self._bagg()._type_prior(player, infoset)))

    def _getprob(self, player: str, infoset: str, action: str) -> float:
        return deref(self.profile).getitem_strategy(self._strategy(player, infoset, action))

    def _setprob(self, player: str, infoset: str, action: str, value: float) -> None:
        handle: c_GameStrategy = self._strategy(player, infoset, action)
        if self.profile.use_count() != 1:
            self.profile = make_shared[c_MixedStrategyProfile[double]](deref(self.profile))
        setitem_mspd_strategy(deref(self.profile), handle, value)

    def _agent_payoff(self, player: str, infoset: str) -> float:
        return deref(self.profile).GetPayoff(self._bagg()._agent_player(player, infoset))

    def _agent_regret(self, player: str, infoset: str) -> float:
        return deref(self.profile).GetRegret(self._bagg()._agent_player(player, infoset))

    def _agent_action_value(self, player: str, infoset: str, action: str) -> float:
        return deref(self.profile).GetPayoff(self._strategy(player, infoset, action))

    def _agent_action_regret(self, player: str, infoset: str, action: str) -> float:
        return deref(self.profile).GetRegret(self._strategy(player, infoset, action))

    def _agent_liap_value(self) -> float:
        return deref(self.profile).GetLiapValue()

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, BAGGMixedBehaviorProfileDouble) and
            deref(self.profile) ==
            deref(cython.cast(BAGGMixedBehaviorProfileDouble, other).profile)
        )

    def _copy(self) -> BAGGMixedBehaviorProfileDouble:
        return BAGGMixedBehaviorProfileDouble.wrap(self.profile)

    def _as_float(self) -> BAGGMixedBehaviorProfileDouble:
        return self._copy()

    def _normalize(self) -> BAGGMixedBehaviorProfileDouble:
        return BAGGMixedBehaviorProfileDouble.wrap(
            make_shared[c_MixedStrategyProfile[double]](deref(self.profile).Normalize())
        )


@cython.cclass
class BAGGMixedBehaviorProfileRational(BAGGMixedBehaviorProfile):
    """A `BAGGMixedBehaviorProfile` with exact (rational) probabilities.

    .. versionadded:: 17.0.0
    """
    profile = cython.declare(shared_ptr[c_MixedStrategyProfile[c_Rational]])

    @staticmethod
    @cython.cfunc
    def wrap(
        profile: shared_ptr[c_MixedStrategyProfile[c_Rational]]
    ) -> BAGGMixedBehaviorProfileRational:
        obj: BAGGMixedBehaviorProfileRational = (
            BAGGMixedBehaviorProfileRational.__new__(BAGGMixedBehaviorProfileRational)
        )
        obj.profile = profile
        return obj

    def _check_validity(self) -> None:
        if deref(self.profile).HasOutdatedGameVersion():
            raise GameStructureChangedError()

    @property
    def _game(self) -> Game:
        return Game._wrap(deref(self.profile).GetGame())

    def _to_prob(self, value: typing.Any) -> Rational:
        return Rational(_to_number_string(value))

    def _coerce(self, value: typing.Any) -> Rational:
        return Rational(value)

    def _prior(self, player: str, infoset: str) -> Rational:
        return rat_to_py(self._bagg()._type_prior(player, infoset))

    def _getprob(self, player: str, infoset: str, action: str) -> Rational:
        return rat_to_py(
            deref(self.profile).getitem_strategy(self._strategy(player, infoset, action))
        )

    def _setprob(self, player: str, infoset: str, action: str, value: Rational) -> None:
        handle: c_GameStrategy = self._strategy(player, infoset, action)
        if self.profile.use_count() != 1:
            self.profile = make_shared[c_MixedStrategyProfile[c_Rational]](deref(self.profile))
        setitem_mspr_strategy(deref(self.profile), handle,
                              to_rational(str(value).encode("ascii")))

    def _agent_payoff(self, player: str, infoset: str) -> Rational:
        return rat_to_py(
            deref(self.profile).GetPayoff(self._bagg()._agent_player(player, infoset))
        )

    def _agent_regret(self, player: str, infoset: str) -> Rational:
        return rat_to_py(
            deref(self.profile).GetRegret(self._bagg()._agent_player(player, infoset))
        )

    def _agent_action_value(self, player: str, infoset: str, action: str) -> Rational:
        return rat_to_py(
            deref(self.profile).GetPayoff(self._strategy(player, infoset, action))
        )

    def _agent_action_regret(self, player: str, infoset: str, action: str) -> Rational:
        return rat_to_py(
            deref(self.profile).GetRegret(self._strategy(player, infoset, action))
        )

    def _agent_liap_value(self) -> Rational:
        return rat_to_py(deref(self.profile).GetLiapValue())

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, BAGGMixedBehaviorProfileRational) and
            deref(self.profile) ==
            deref(cython.cast(BAGGMixedBehaviorProfileRational, other).profile)
        )

    def _copy(self) -> BAGGMixedBehaviorProfileRational:
        return BAGGMixedBehaviorProfileRational.wrap(self.profile)

    def _as_float(self) -> BAGGMixedBehaviorProfileDouble:
        result = self._bagg()._new_profile(False)
        for player in self.game.players:
            for infoset in self.game.get_infosets(player):
                result[player, infoset] = {
                    action: float(self._getprob(player, infoset, action))
                    for action in self._action_labels(player, infoset)
                }
        return result

    def _normalize(self) -> BAGGMixedBehaviorProfileRational:
        return BAGGMixedBehaviorProfileRational.wrap(
            make_shared[c_MixedStrategyProfile[c_Rational]](deref(self.profile).Normalize())
        )


@cython.cclass
class BAGGLogitQREMixedBehaviorProfile:
    """A logit quantal response equilibrium of a Bayesian action-graph game.

    This is the type-level (Bayesian) logit QRE: every type of every player responds to
    the expected payoffs of its actions with the same precision `lam`, the same notion as
    the agent QRE of an extensive game.  It is not the QRE of the strategic form of the
    Bayesian game, which would give a type with prior probability ``P(t)`` the precision
    ``lam * P(t)``.  Both converge to Nash equilibria of the game as `lam` tends to
    infinity.

    .. versionadded:: 17.0.0

    See Also
    --------
    pygambit.nash.logit_solve : Compute a Nash equilibrium by tracing the QRE correspondence.
    """
    thisptr = cython.declare(shared_ptr[c_LogitQREMixedStrategyProfile])

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a BAGGLogitQREMixedBehaviorProfile outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(
        profile: shared_ptr[c_LogitQREMixedStrategyProfile]
    ) -> BAGGLogitQREMixedBehaviorProfile:
        obj: BAGGLogitQREMixedBehaviorProfile = (
            BAGGLogitQREMixedBehaviorProfile.__new__(BAGGLogitQREMixedBehaviorProfile)
        )
        obj.thisptr = profile
        return obj

    def __repr__(self) -> str:
        return f"BAGGLogitQREMixedBehaviorProfile(lam={self.lam},profile={self.profile})"

    @property
    def game(self) -> Game:
        """The game on which this profile is defined."""
        return Game._wrap(deref(self.thisptr).GetGame())

    @property
    def lam(self) -> float:
        """The value of the precision parameter."""
        return deref(self.thisptr).GetLambda()

    @property
    def log_like(self) -> float:
        """The log-likelihood of the data."""
        return deref(self.thisptr).GetLogLike()

    @property
    def profile(self) -> BAGGMixedBehaviorProfileDouble:
        """The mixed behavior profile."""
        return BAGGMixedBehaviorProfileDouble.wrap(
            make_shared[c_MixedStrategyProfile[double]](deref(self.thisptr).GetProfile())
        )
