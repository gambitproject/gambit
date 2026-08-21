#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/behavmixed.pxi
# Cython wrapper for mixed behavior profiles
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


@cython.cclass
class InfosetIndexedVector(_LabeledVector):
    """A read-only mapping from an information set to a computed value, one entry per
    information set.

    Since information sets don't reliably have unique persistent labels, this is indexed
    by any ``Node`` belonging to the information set (resolved to the information set
    itself before lookup) rather than by a label: any member node is an equally valid
    key, unlike ``NodeIndexedVector``.
    """
    _label_kind = "information set"

    def __getitem__(self, node: Node) -> typing.Any:
        infoset = cython.cast(NodeInfoset, node.infoset)._resolve()
        if infoset is None:
            raise ValueError("node is terminal, has no information set")
        try:
            return self._values[infoset]
        except KeyError:
            raise KeyError(f"no {self._label_kind} for this node") from None


@cython.cclass
class InfosetValueVector(InfosetIndexedVector):
    """The expected payoff to the player conditional on reaching each information set,
    one entry per (non-chance) information set.
    """


@cython.cclass
class InfosetRegretVector(InfosetIndexedVector):
    """The regret of playing the mixed action at each information set, one entry per
    information set.
    """


@cython.cclass
class InfosetProbVector(InfosetIndexedVector):
    """The probability with which each information set is reached, one entry per
    information set.
    """


@cython.cclass
class ActionValueVector(StrategyIndexedVector):
    """The expected payoff of playing each action, conditional on reaching it, for one
    information set's actions.
    """


@cython.cclass
class ActionRegretVector(StrategyIndexedVector):
    """The regret of playing each action, for one information set's actions."""


@cython.cclass
class ActionValuesVector(InfosetIndexedVector):
    """The expected payoff of playing each action, conditional on reaching it, grouped
    by information set; each value is an `ActionValueVector` for that information set's
    actions.
    """


@cython.cclass
class ActionRegretsVector(InfosetIndexedVector):
    """The regret of playing each action, grouped by information set; each value is an
    `ActionValueVector` for that information set's actions.
    """


@cython.cclass
class RealizProbVector(NodeIndexedVector):
    """The probability with which each node is reached, one entry per node."""


@cython.cclass
class BeliefVector(NodeIndexedVector):
    """The conditional probability that each node is reached, given that its
    information set is reached, one entry per node.
    """


@cython.cclass
class NodeValueVector(NodeIndexedVector):
    """The expected payoff to one player conditional on reaching each node, one entry
    per node.
    """


@cython.cclass
class NodeValuesVector(PlayerIndexedVector):
    """The expected payoff to each (non-chance) player conditional on reaching each
    node, grouped by player; each value is a `NodeValueVector` for that player.
    """


@cython.cclass
class MixedAction:
    """A probability distribution over a player's actions at an information set.

    An immutable snapshot taken from a ``MixedBehaviorProfile`` at retrieval time: it
    does not reflect later changes to the profile, and cannot itself be modified. The
    information set is accessible via `infoset`.

    .. versionchanged:: 17.0.0

        No longer a live view onto the profile: holds its own copy of the probabilities,
        and can no longer be assigned into. Set a distribution via
        ``MixedBehaviorProfile.__setitem__`` instead.
    """
    _infoset = cython.declare(Infoset)
    _values = cython.declare(dict)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a MixedAction outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(infoset: Infoset, values: dict) -> MixedAction:
        obj: MixedAction = MixedAction.__new__(MixedAction)
        obj._infoset = infoset
        obj._values = values
        return obj

    @property
    def infoset(self) -> Infoset:
        """The information set over which this mixed action is defined."""
        return self._infoset

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
        if not isinstance(other, MixedAction) or self.infoset != other.infoset:
            return False
        return self._values == cython.cast(MixedAction, other)._values

    def __len__(self) -> int:
        return len(self._values)

    def __iter__(self) -> typing.Iterator[tuple[str, ProfileDType], None, None]:
        """Iterate over the probabilities assigned to actions by the mixed action.

        .. versionchanged:: 17.0.0

            Yields the action's label instead of the ``Action`` object.

        Yields
        ------
        label : str
            The label of an action at the information set
        probability : float or Rational
            The probability the mixed action assigns to the action being played
        """
        yield from self._values.items()

    def __getitem__(self, action: str) -> ProfileDType:
        """Returns the probability that the action with label `action` is played.

        Parameters
        ----------
        action : str
            The label of the action to look up.

        Returns
        -------
        float or Rational
            The probability assigned to the action.

        Raises
        ------
        KeyError
            If no action at this information set has the label `action`.
        """
        try:
            return self._values[action]
        except KeyError:
            raise KeyError(f"no action with label '{action}' at infoset") from None


@cython.cclass
class MixedBehavior:
    """A set of probability distributions describing a player's behavior.

    An immutable snapshot taken from a ``MixedBehaviorProfile`` at retrieval time: it
    does not reflect later changes to the profile, and cannot itself be modified. The
    player is accessible via `player`.

    .. versionchanged:: 17.0.0

        No longer a live view onto the profile: holds its own copy of the mixed actions,
        and can no longer be assigned into. Set a player's whole behavior via
        ``MixedBehaviorProfile.__setitem__`` instead.
    """
    _player = cython.declare(Player)
    _values = cython.declare(dict)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a MixedBehavior outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(player: Player, values: dict) -> MixedBehavior:
        obj: MixedBehavior = MixedBehavior.__new__(MixedBehavior)
        obj._player = player
        obj._values = values
        return obj

    @property
    def player(self) -> Player:
        """The player for whom this mixed behavior strategy is defined."""
        return self._player

    def __repr__(self) -> str:
        return str(list(self._values.values()))

    def _repr_latex_(self) -> str:
        values = list(self._values.values())
        if not values or not hasattr(values[0], "_repr_latex_"):
            return repr(self)
        return (
            r"$\left[" +
            ",".join(value._repr_latex_().replace("$", "") for value in values) +
            r"\right]$"
        )

    def __eq__(self, other: typing.Any) -> bool:
        values = list(self._values.values())
        if isinstance(other, list):
            return values == other
        if not isinstance(other, MixedBehavior) or self.player != other.player:
            return False
        return values == list(cython.cast(MixedBehavior, other)._values.values())

    def __len__(self) -> int:
        return len(self._values)

    def __iter__(self) -> typing.Iterator[tuple[Infoset, MixedAction], None, None]:
        """Iterate over the mixed actions specified by the mixed behavior.

        A ``MixedBehavior`` is a collection of ``MixedAction``\\ s, one per information
        set belonging to the player; this iterates over those, not over individual
        actions.

        .. versionchanged:: 17.0.0

            Previously iterated over individual actions and their probabilities; use
            ``MixedAction``'s own iteration for that at a specific information set.

        Yields
        ------
        infoset : Infoset
            An information set belonging to the player
        action : MixedAction
            The player's mixed action specified at the information set
        """
        yield from self._values.items()

    def __getitem__(self, index: Node) -> MixedAction:
        """Returns the mixed action at the information set containing `index`.

        Parameters
        ----------
        index : Node
            A node belonging to the information set to return.

        Raises
        ------
        MismatchError
            If `index` is a ``Node`` from a different game, or belongs to an
            information set that isn't this player's.
        ValueError
            If `index` is a terminal node, which belongs to no information set.
        """
        infoset = cython.cast(NodeInfoset, index.infoset)._resolve()
        if infoset is None:
            raise ValueError("node is terminal, has no information set")
        if infoset.player != self.player:
            raise MismatchError("node must belong to this player")
        return self._values[infoset]


@cython.cclass
class MixedBehaviorProfile:
    """Represents a mixed behavior profile over the actions in a ``Game``.

    A mixed behavior profile is a dict-like object, mapping each action at each information
    set in a game to the corresponding probability with which the action is played, conditional
    on that information set being reached.

    Mixed behavior profiles may represent probabilities as either exact (rational)
    numbers, or floating-point numbers.  These may not be combined in the same mixed
    behavior profile.

    .. versionchanged:: 16.1.0
        Profiles are accessed as dict-like objects; indexing by integer player, infoset, or
        action indices is no longer supported.

    See Also
    --------
    Game.mixed_behavior_profile
        Creates a new mixed behavior profile on a game.
    MixedStrategyProfile
        Represents a mixed strategy profile over a ``Game``.
    """
    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a MixedBehaviorProfile outside a Game.")

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
        """The game on which this mixed behavior profile is defined."""
        return self._game

    def __iter__(self) -> typing.Iterator[MixedBehavior, None, None]:
        """Iterate over the mixed behaviors in the profile, one per player.

        .. versionchanged:: 17.0.0

            Previously yielded `(Action, probability)` pairs flattened across every
            player's actions; now yields the `MixedBehavior` for each player, matching
            what `mixed_behaviors()` returned.

        Yields
        ------
        behavior : MixedBehavior
            The player's mixed behavior specified in the profile
        """
        for player in self.game.players:
            yield self[player.label]

    def __getitem__(self, index: typing.Any) -> MixedBehavior | MixedAction:
        """Access a component of the mixed behavior profile specified by `index`.

        Parameters
        ----------
        index : str or Node
            The part of the profile to return:

            * If `index` is a ``str``, returns a ``MixedBehavior`` over the player's
              information sets. The player is determined by finding the player with
              that label, if any.
            * If `index` is a ``Node``, returns a ``MixedAction`` over the actions at
              the node's information set.

        Raises
        ------
        TypeError
            If `index` is not a ``str`` or a ``Node``.
        MismatchError
            If `index` is a ``Node`` from a different game.
        ValueError
            If `index` is a terminal ``Node``, which belongs to no information set.
        KeyError
            If `index` is a ``str`` and no player in the game has that label.
        """
        self._check_validity()
        if isinstance(index, Node):
            return self._mixed_action_at(self._resolve_infoset_for_node(index))
        if isinstance(index, str):
            resolved_player = self.game._resolve_player(index, "__getitem__")
            values = {
                infoset: self._mixed_action_at(infoset) for infoset in resolved_player.infosets
            }
            return MixedBehavior.wrap(resolved_player, values)
        raise TypeError(
            f"profile index must be str or Node, not {index.__class__.__name__}"
        )

    def _resolve_infoset_for_node(self, node: Node) -> Infoset:
        """Resolves the information set containing node.

        Raises
        ------
        MismatchError
            If `node` belongs to a different game.
        ValueError
            If `node` is terminal, and so belongs to no information set.
        """
        if node.game != self.game:
            raise MismatchError("node must belong to this game")
        infoset = cython.cast(NodeInfoset, node.infoset)._resolve()
        if infoset is None:
            raise ValueError("node is terminal, has no information set")
        return infoset

    def _all_infosets(self) -> typing.Iterator[Infoset]:
        """Iterates over every information set in the game, including the chance
        player's, which ``self.game.infosets`` excludes.
        """
        yield from self.game.infosets
        yield from self.game.players.chance.infosets

    def _mixed_action_at(self, infoset: Infoset) -> MixedAction:
        """Returns a snapshot of the mixed action at infoset, as of now."""
        return MixedAction.wrap(
            infoset, {a.label: self._getprob_action(a) for a in infoset.actions}
        )

    def _setprob_infoset(
        self, infoset: Infoset, distribution: collections.abc.Mapping, sparse: bool
    ) -> None:
        """Validates and sets the whole mixed action for infoset.

        Every key of `distribution` must be one of the information set's action
        labels. If `sparse` is True, actions `distribution` omits are treated as
        having weight zero; if False, `distribution` must specify a weight for every
        action. Weights must be non-negative and not all zero.
        """
        if not isinstance(distribution, collections.abc.Mapping):
            raise TypeError(
                f"a mixed action must be set from a Mapping from action label to "
                f"weight, not {distribution.__class__.__name__}"
            )
        labels = {a.label for a in infoset.actions}
        given = set(distribution.keys())
        unknown = given - labels
        if unknown:
            raise ValueError(
                f"not an action label at this information set: {', '.join(sorted(unknown))}"
            )
        if not sparse and given != labels:
            raise ValueError(
                "a distribution must specify exactly one weight for each action at the "
                "information set, unless sparse=True"
            )
        zero = self._to_prob(0)
        values = {label: zero for label in labels}
        values.update({label: self._to_prob(weight) for label, weight in distribution.items()})
        if any(v < 0 for v in values.values()):
            raise ValueError("a mixed action's weights must be non-negative")
        if all(v == 0 for v in values.values()):
            raise ValueError("a mixed action's weights must not all be zero")
        for a in infoset.actions:
            self._setprob_action(a, values[a.label])

    def __setitem__(self, index: Node, distribution: collections.abc.Mapping) -> None:
        """Sets the mixed action at the information set containing `index`.

        `distribution` need not specify a weight for every one of the information
        set's actions: actions it omits are treated as having weight zero. Use
        `set_mixed_action` if you want that to be an error instead.

        Parameters
        ----------
        index : Node
            A node belonging to the information set to set.
        distribution : Mapping[str, Any]
            A non-negative weight for some or all of the information set's actions,
            keyed by action label; actions it omits are treated as having weight
            zero. A weight may be any value Gambit can interpret as a number
            (`int`, `float`, `str`, `Decimal`, or `Rational`). Weights need not sum
            to one, and at least one must be nonzero.

        Raises
        ------
        TypeError
            If `index` is not a ``Node``, or `distribution` is not a Mapping.
        MismatchError
            If `index` is a ``Node`` from a different game.
        ValueError
            If `index` is a terminal ``Node``, which belongs to no information set;
            if any key of `distribution` is not one of the information set's action
            labels; if any weight cannot be interpreted as a number; if any weight
            is negative; or if the weights are all zero.

        See Also
        --------
        set_mixed_action
            Equivalent, but can require a weight for every action instead of
            silently defaulting omitted ones to zero.
        """
        self._check_validity()
        if not isinstance(index, Node):
            raise TypeError(f"profile index must be Node, not {index.__class__.__name__}")
        infoset = self._resolve_infoset_for_node(index)
        self._setprob_infoset(infoset, distribution, sparse=True)

    def set_mixed_action(
        self, index: Node, distribution: collections.abc.Mapping, sparse: bool = False
    ) -> None:
        """Sets the mixed action at the information set containing `index`.

        Equivalent to ``profile[index] = distribution``, except that by default
        every one of the information set's actions must be given an explicit
        weight in `distribution`. Use this instead of `__setitem__` when omitting
        an action should be an error rather than silently defaulting its weight to
        zero.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        index : Node
            A node belonging to the information set to set.
        distribution : Mapping[str, Any]
            A non-negative weight for the information set's actions, keyed by
            action label. A weight may be any value Gambit can interpret as a
            number (`int`, `float`, `str`, `Decimal`, or `Rational`). Weights need
            not sum to one, and at least one must be nonzero.
        sparse : bool, default False
            If False (the default), `distribution` must specify a weight for every
            one of the information set's actions. If True, actions it omits are
            treated as having weight zero, the same as
            ``profile[index] = distribution``.

        Raises
        ------
        TypeError
            If `index` is not a ``Node``, or `distribution` is not a Mapping.
        MismatchError
            If `index` is a ``Node`` from a different game.
        ValueError
            If `index` is a terminal ``Node``, which belongs to no information set;
            if any key of `distribution` is not one of the information set's action
            labels; if `sparse` is False and `distribution` omits an action; if any
            weight cannot be interpreted as a number; if any weight is negative; or
            if the weights are all zero.

        See Also
        --------
        __setitem__
        """
        self._check_validity()
        if not isinstance(index, Node):
            raise TypeError(f"profile index must be Node, not {index.__class__.__name__}")
        infoset = self._resolve_infoset_for_node(index)
        self._setprob_infoset(infoset, distribution, sparse=sparse)

    def is_defined_at(self, infoset: InfosetReference) -> bool:
        """Returns whether the profile has probabilities defined at the information set.
        A profile can be well-defined if probabilities are not specified at some information sets,
        as long as those information sets are reached with zero probability.

        Parameters
        ----------
        infoset : Infoset or str
            The information set to check.  If a string is passed, the
            information set is determined by finding the information set with that label, if any.

        Raises
        ------
        MismatchError
            If `infoset` is an ``Infoset`` from a different game.
        KeyError
            If `infoset` is a string and no information set in the game has that label.
        """
        self._check_validity()
        return self._is_defined_at(self.game._resolve_infoset(infoset, "is_defined_at"))

    @property
    def payoffs(self) -> PayoffVector:
        """Returns the expected payoff to each player, if all players play according to
        the profile.

        The chance player is excluded, since it takes no decisions and so has no
        well-defined payoff; ``self.game.players`` already excludes it.
        """
        self._check_validity()
        return PayoffVector({p.label: self._payoff(p) for p in self.game.players})

    @property
    def node_values(self) -> NodeValuesVector:
        """Returns the expected payoff to each player conditional on play reaching each
        node, if all players play according to the profile, grouped by player.
        """
        self._check_validity()
        return NodeValuesVector({
            p.label: NodeValueVector({n: self._node_value(p, n) for n in self.game.nodes})
            for p in self.game.players
        })

    @property
    def infoset_values(self) -> InfosetValueVector:
        """Returns the expected payoff to the player conditional on reaching each
        information set, if all players play according to the profile.

        If an information set is not reachable, its expected payoff is not
        well-defined, and the corresponding entry is `None`.

        See Also
        --------
        MixedBehaviorProfile.infoset_probs
        """
        self._check_validity()
        return InfosetValueVector({
            infoset: self._infoset_value(infoset) for infoset in self.game.infosets
        })

    @property
    def action_values(self) -> ActionValuesVector:
        """Returns the expected payoff to the player of playing each action,
        conditional on reaching its information set, if all players play according to
        the profile, grouped by information set.

        If an information set is not reachable, the expected payoffs of its actions
        are not well-defined, and the corresponding entries are `None`.

        See Also
        --------
        MixedBehaviorProfile.infoset_probs
        """
        self._check_validity()
        return ActionValuesVector({
            infoset: ActionValueVector({a.label: self._action_value(a) for a in infoset.actions})
            for infoset in self.game.infosets
        })

    @property
    def realiz_probs(self) -> RealizProbVector:
        """Returns the probability with which each node is reached, if all players
        play according to the profile.
        """
        self._check_validity()
        return RealizProbVector({n: self._realiz_prob(n) for n in self.game.nodes})

    @property
    def infoset_probs(self) -> InfosetProbVector:
        """Returns the probability with which each information set is reached, if all
        players play according to the profile.

        This is the probability that the information set is reached *at least once*
        under the profile: the realization probability of its upper frontier, i.e. the
        members not preceded by another member of the same information set.
        For a non-absent-minded information set, its upper frontier coincides with it.
        For an absent-minded information set, with a play passing through it more than once,
        the members below its frontier are excluded, so this is generally less than
        the sum of the members' realization probabilities; see :cite:p:`HalPas21`.

        See Also
        --------
        MixedBehaviorProfile.beliefs
        """
        self._check_validity()
        return InfosetProbVector({
            infoset: self._infoset_prob(infoset) for infoset in self._all_infosets()
        })

    @property
    def beliefs(self) -> BeliefVector:
        """Returns, for each node, the conditional probability that the node is
        reached, given that its information set is reached, if all players play
        according to the profile.

        The conditioning event is that the information set is reached at least once,
        so beliefs are normalized by the upper-frontier probability returned by
        `infoset_probs` (following :cite:p:`HalPas21`), rather than by the sum of the
        members' realization probabilities.  For a non-absent-minded information set
        the two approaches agree.  For an absent-minded information set they need not:
        the beliefs over its members may sum to more than one.

        If a node's information set is reached with zero probability under the
        profile, the belief is not well-defined and the corresponding entry is `None`.
        This is the same reach probability returned by `infoset_probs`, so a `None`
        belief corresponds exactly to `infoset_probs` being zero there.

        See Also
        --------
        MixedBehaviorProfile.infoset_probs
        """
        self._check_validity()
        return BeliefVector({n: self._belief(n) for n in self.game.nodes})

    @property
    def action_regrets(self) -> ActionRegretsVector:
        """Returns the regret to playing each action, if all other players play
        according to the profile, grouped by information set.

        The regret is defined as the difference between the payoff of the
        best-response action and the payoff of the action.  Payoffs are computed
        conditional on reaching the information set.  By convention, the regret is
        always non-negative.

        Regret is not defined for the chance player, which takes no decisions; its
        information sets are excluded (``self.game.infosets`` already excludes them).

        See Also
        --------
        infoset_regrets
        agent_max_regret
        """
        self._check_validity()
        return ActionRegretsVector({
            infoset: ActionRegretVector({a.label: self._action_regret(a) for a in infoset.actions})
            for infoset in self.game.infosets
        })

    @property
    def infoset_regrets(self) -> InfosetRegretVector:
        """Returns the regret to the player for playing their mixed action at each
        information set, if all other players play according to the profile.

        The regret is defined as the difference between the payoff of the
        best-response action and the payoff of the player's mixed action.
        Payoffs are computed conditional on reaching the information set.
        By convention, the regret is always non-negative.

        Regret is not defined for the chance player, which takes no decisions; its
        information sets are excluded (``self.game.infosets`` already excludes them).

        See Also
        --------
        action_regrets
        agent_max_regret
        """
        self._check_validity()
        return InfosetRegretVector({
            infoset: self._infoset_regret(infoset) for infoset in self.game.infosets
        })

    def agent_max_regret(self) -> ProfileDType:
        """Returns the maximum regret at any information set.

        A profile is an agent Nash equilibrium if and only if `agent_max_regret()` is 0.

        .. versionchanged:: 16.5.0

           Renamed from `max_regret` to `agent_max_regret` to clarify the distinction between
           per-player and per-agent concepts.

        See Also
        --------
        action_regrets
        infoset_regrets
        max_regret
        agent_liap_value
        """
        self._check_validity()
        return self._agent_max_regret()

    def agent_liap_value(self) -> ProfileDType:
        """Returns the Lyapunov value (see :cite:p:`McK91`) of the strategy profile.

        The agent Lyapunov value is a non-negative number which is zero exactly at
        agent Nash equilibria.

        .. versionchanged:: 16.5.0

           Renamed from `liap_value` to `agent_liap_value` to clarify the distinction between
           per-player and per-agent concepts.

        See Also
        --------
        agent_max_regret
        liap_value
        """
        self._check_validity()
        return self._agent_liap_value()

    def max_regret(self) -> ProfileDType:
        """Returns the maximum regret at any information set.

        A profile is a Nash equilibrium if and only if `max_regret()` is 0.

        .. versionchanged:: 16.5.0

           New implementation of `max_regret` to clarify the distinction between
           per-player and per-agent concepts.

        See Also
        --------
        liap_value
        agent_max_regret
        """
        self._check_validity()
        return self._max_regret()

    def liap_value(self) -> ProfileDType:
        """Returns the Lyapunov value (see :cite:p:`McK91`) of the strategy profile.

        The Lyapunov value is a non-negative number which is zero exactly at
        Nash equilibria.

        .. versionchanged:: 16.5.0

           New implementation of `liap_value` to clarify the distinction between
           per-player and per-agent concepts.

        See Also
        --------
        max_regret
        agent_liap_value
        """
        self._check_validity()
        return self._liap_value()

    def as_strategy(self) -> MixedStrategyProfile:
        """Returns a `MixedStrategyProfile` which is equivalent
        to the profile.
        """
        self._check_validity()
        return self._as_strategy()

    def normalize(self) -> MixedBehaviorProfile:
        """Create a profile with the same action proportions as this
        one, but normalised so probabilities for each infoset sum to one.
        """
        self._check_validity()
        return self._normalize()

    def copy(self) -> MixedBehaviorProfile:
        """Creates a copy of the behavior strategy profile.

        .. versionchanged:: 17.0.0

            The copy shares its underlying data with the original until one of them is
            next assigned into, at which point the one being assigned into transparently
            takes its own private copy first. Both profiles are fully independent from
            each other's perspective; this only affects when the underlying duplication
            happens, not whether it happens.
        """
        self._check_validity()
        return self._copy()


@cython.cclass
class MixedBehaviorProfileDouble(MixedBehaviorProfile):
    profile = cython.declare(shared_ptr[c_MixedBehaviorProfile[double]])

    @staticmethod
    @cython.cfunc
    def wrap(profile: shared_ptr[c_MixedBehaviorProfile[float]]) -> MixedBehaviorProfileDouble:
        obj: MixedBehaviorProfileDouble = (
            MixedBehaviorProfileDouble.__new__(MixedBehaviorProfileDouble)
        )
        obj.profile = profile
        return obj

    def _check_validity(self) -> None:
        if deref(self.profile).IsInvalidated():
            raise GameStructureChangedError()

    def __len__(self) -> int:
        return deref(self.profile).BehaviorProfileLength()

    def _is_defined_at(self, infoset: Infoset) -> bool:
        return deref(self.profile).IsDefinedAt(infoset.infoset)

    def _getprob_action(self, index: Action) -> float:
        return deref(self.profile).getaction(index.action)

    @cython.cfunc
    def _ensure_unshared(self) -> cython.void:
        """Clones the underlying profile if it is shared with another wrapper, so that
        the mutation about to happen is not observed by any other MixedBehaviorProfile.
        """
        if self.profile.use_count() != 1:
            self.profile = make_shared[c_MixedBehaviorProfile[double]](deref(self.profile))

    def _setprob_action(self, index: Action, value) -> None:
        self._ensure_unshared()
        setitem_mbpd_action(deref(self.profile), index.action, value)

    def _to_prob(self, value: typing.Any) -> float:
        normalized = _to_number_string(value)
        try:
            return float(normalized)
        except ValueError:
            # normalized is a fraction-form string (e.g. "1/2"), which float() rejects
            return float(Rational(normalized))

    def _payoff(self, player: Player) -> float:
        return deref(self.profile).GetPayoff(player.player)

    def _belief(self, node: Node) -> float:
        cdef optional[double] value = deref(self.profile).GetBeliefProb(node.node)
        if value.has_value():
            return value.value()
        return None

    def _realiz_prob(self, node: Node) -> float:
        return deref(self.profile).GetRealizProb(node.node)

    def _infoset_prob(self, infoset: Infoset) -> float:
        return deref(self.profile).GetInfosetProb(infoset.infoset)

    def _infoset_value(self, infoset: Infoset) -> float | None:
        cdef optional[double] value = deref(self.profile).GetPayoff(infoset.infoset)
        if value.has_value():
            return value.value()
        return None

    def _node_value(self, player: Player, node: Node) -> float:
        return deref(self.profile).GetPayoff(player.player, node.node)

    def _action_value(self, action: Action) -> float | None:
        cdef optional[double] value = deref(self.profile).GetPayoff(action.action)
        if value.has_value():
            return value.value()
        return None

    def _action_regret(self, action: Action) -> float:
        return deref(self.profile).GetRegret(action.action)

    def _infoset_regret(self, infoset: Infoset) -> float:
        return deref(self.profile).GetRegret(infoset.infoset)

    def _agent_max_regret(self) -> float:
        return deref(self.profile).GetAgentMaxRegret()

    def _max_regret(self) -> float:
        return deref(self.profile).GetMaxRegret()

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, MixedBehaviorProfileDouble) and
            deref(self.profile) == deref(cython.cast(MixedBehaviorProfileDouble, other).profile)
        )

    def _copy(self) -> MixedBehaviorProfileDouble:
        # Copy-on-write: share the underlying profile; _ensure_unshared() clones it
        # lazily, the first time either this copy or the original is next mutated.
        return MixedBehaviorProfileDouble.wrap(self.profile)

    def _as_strategy(self) -> MixedStrategyProfileDouble:
        return MixedStrategyProfileDouble.wrap(make_shared[c_MixedStrategyProfile[double]](
            deref(self.profile).ToMixedProfile()
        ))

    def _agent_liap_value(self) -> float:
        return deref(self.profile).GetAgentLiapValue()

    def _liap_value(self) -> float:
        return deref(self.profile).GetLiapValue()

    def _normalize(self) -> MixedBehaviorProfileDouble:
        return MixedBehaviorProfileDouble.wrap(
            make_shared[c_MixedBehaviorProfile[double]](deref(self.profile).Normalize())
        )

    @property
    def _game(self) -> Game:
        return Game.wrap(deref(self.profile).GetGame())


@cython.cclass
class MixedBehaviorProfileRational(MixedBehaviorProfile):
    profile = cython.declare(shared_ptr[c_MixedBehaviorProfile[c_Rational]])

    @staticmethod
    @cython.cfunc
    def wrap(
        profile: shared_ptr[c_MixedBehaviorProfile[c_Rational]]
    ) -> MixedBehaviorProfileRational:
        obj: MixedBehaviorProfileRational = (
            MixedBehaviorProfileRational.__new__(MixedBehaviorProfileRational)
        )
        obj.profile = profile
        return obj

    def _check_validity(self) -> None:
        if deref(self.profile).IsInvalidated():
            raise GameStructureChangedError()

    def __len__(self) -> int:
        return deref(self.profile).BehaviorProfileLength()

    def _is_defined_at(self, infoset: Infoset) -> bool:
        return deref(self.profile).IsDefinedAt(infoset.infoset)

    def _getprob_action(self, index: Action) -> Rational:
        return rat_to_py(deref(self.profile).getaction(index.action))

    @cython.cfunc
    def _ensure_unshared(self) -> cython.void:
        """Clones the underlying profile if it is shared with another wrapper, so that
        the mutation about to happen is not observed by any other MixedBehaviorProfile.
        """
        if self.profile.use_count() != 1:
            self.profile = make_shared[c_MixedBehaviorProfile[c_Rational]](deref(self.profile))

    def _setprob_action(self, index: Action, value: typing.Any) -> None:
        if not isinstance(value, (int, fractions.Fraction)):
            raise TypeError(
                f"rational precision profile requires int or Fraction probability, "
                f"not {value.__class__.__name__}"
            )
        self._ensure_unshared()
        setitem_mbpr_action(deref(self.profile), index.action,
                            to_rational(str(value).encode("ascii")))

    def _to_prob(self, value: typing.Any) -> Rational:
        return Rational(_to_number_string(value))

    def _payoff(self, player: Player) -> Rational:
        return rat_to_py(deref(self.profile).GetPayoff(player.player))

    def _belief(self, node: Node) -> Rational:
        cdef optional[c_Rational] value = deref(self.profile).GetBeliefProb(node.node)
        if value.has_value():
            return rat_to_py(value.value())
        return None

    def _realiz_prob(self, node: Node) -> Rational:
        return rat_to_py(deref(self.profile).GetRealizProb(node.node))

    def _infoset_prob(self, infoset: Infoset) -> Rational:
        return rat_to_py(deref(self.profile).GetInfosetProb(infoset.infoset))

    def _infoset_value(self, infoset: Infoset) -> Rational | None:
        cdef optional[c_Rational] value = deref(self.profile).GetPayoff(infoset.infoset)
        if value.has_value():
            return rat_to_py(value.value())
        return None

    def _node_value(self, player: Player, node: Node) -> Rational:
        return rat_to_py(deref(self.profile).GetPayoff(player.player, node.node))

    def _action_value(self, action: Action) -> Rational | None:
        cdef optional[c_Rational] value = deref(self.profile).GetPayoff(action.action)
        if value.has_value():
            return rat_to_py(value.value())
        return None

    def _action_regret(self, action: Action) -> Rational:
        return rat_to_py(deref(self.profile).GetRegret(action.action))

    def _infoset_regret(self, infoset: Infoset) -> Rational:
        return rat_to_py(deref(self.profile).GetRegret(infoset.infoset))

    def _agent_max_regret(self) -> Rational:
        return rat_to_py(deref(self.profile).GetAgentMaxRegret())

    def _max_regret(self) -> Rational:
        return rat_to_py(deref(self.profile).GetMaxRegret())

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, MixedBehaviorProfileRational) and
            deref(self.profile) == deref(cython.cast(MixedBehaviorProfileRational, other).profile)
        )

    def _copy(self) -> MixedBehaviorProfileRational:
        # Copy-on-write: share the underlying profile; _ensure_unshared() clones it
        # lazily, the first time either this copy or the original is next mutated.
        return MixedBehaviorProfileRational.wrap(self.profile)

    def _as_strategy(self) -> MixedStrategyProfileRational:
        return MixedStrategyProfileRational.wrap(make_shared[c_MixedStrategyProfile[c_Rational]](
            deref(self.profile).ToMixedProfile()
        ))

    def _agent_liap_value(self) -> Rational:
        return rat_to_py(deref(self.profile).GetAgentLiapValue())

    def _liap_value(self) -> Rational:
        return rat_to_py(deref(self.profile).GetLiapValue())

    def _normalize(self) -> MixedBehaviorProfileRational:
        return MixedBehaviorProfileRational.wrap(
            make_shared[c_MixedBehaviorProfile[c_Rational]](deref(self.profile).Normalize())
        )

    @property
    def _game(self) -> Game:
        return Game.wrap(deref(self.profile).GetGame())
