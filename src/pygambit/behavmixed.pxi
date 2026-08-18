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
class MixedAction:
    """A probability distribution over a player's actions at an information set.

    A ``MixedAction`` represents a component of a ``MixedBehaviorProfile``.  The
    full profile is accessible via the `profile` attribute, and the information set
    at which the ``MixedAction`` applies is accessible via `infoset`.
    """
    _profile = cython.declare(MixedBehaviorProfile)
    _infoset = cython.declare(Infoset)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a MixedAction outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(profile: MixedBehaviorProfile, infoset: Infoset) -> MixedAction:
        obj: MixedAction = MixedAction.__new__(MixedAction)
        obj._profile = profile
        obj._infoset = infoset
        return obj

    @property
    def profile(self) -> MixedBehaviorProfile:
        """The full profile of which this is a part."""
        return self._profile

    @property
    def infoset(self) -> Infoset:
        """The information set over which this mixed action is defined."""
        return self._infoset

    def __repr__(self) -> str:
        return str({action.label: self[action.label] for action in self.infoset.actions})

    def _repr_latex_(self) -> str:
        values = [self[action.label] for action in self.infoset.actions]
        if not values or not hasattr(values[0], "_repr_latex_"):
            return repr(self)
        return (
            r"$\left\{" +
            ",".join(
                r"\text{" + action.label + "}:" + value._repr_latex_().replace("$", "")
                for action, value in zip(self.infoset.actions, values, strict=True)
            ) +
            r"\right\}$"
        )

    def __eq__(self, other: typing.Any) -> bool:
        values = {action.label: self[action.label] for action in self.infoset.actions}
        if isinstance(other, collections.abc.Mapping):
            return values == dict(other)
        if not isinstance(other, MixedAction) or self.infoset != other.infoset:
            return False
        return values == {action.label: other[action.label] for action in other.infoset.actions}

    def __len__(self) -> len:
        return len(self.infoset.actions)

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
        for action in self.infoset.actions:
            yield action.label, self[action.label]

    def __getitem__(self, index: str) -> ProfileDType:
        """Returns the probability that the action with label `index` is played.

        Parameters
        ----------
        index : str
            The label of the action to look up.

        Returns
        -------
        float or Rational
            The probability assigned to the action.

        Raises
        ------
        KeyError
            If no action at this information set has the label `index`.
        """
        self.profile._check_validity()
        if not isinstance(index, str):
            raise TypeError(f"action index must be str, not {index.__class__.__name__}")
        try:
            return self.profile._getprob_action(self.infoset.actions[index])
        except KeyError:
            raise KeyError(f"no action with label '{index}' at infoset") from None

    def __setitem__(self, index: str, value: typing.Any) -> None:
        """Sets the probability that the action with label `index` is played.

        Parameters
        ----------
        index : str
            The label of the action to set.
        value
            Any value which can be converted to the data type of the ``MixedBehaviorProfile``.

        Raises
        ------
        KeyError
            If no action at this information set has the label `index`.
        """
        self.profile._check_validity()
        if not isinstance(index, str):
            raise TypeError(f"action index must be str, not {index.__class__.__name__}")
        try:
            self.profile._setprob_action(self.infoset.actions[index], value)
        except KeyError:
            raise KeyError(f"no action with label '{index}' at infoset") from None


@cython.cclass
class MixedBehavior:
    """A set of probability distributions describing a player's behavior.

    A ``MixedBehavior`` represents the component of a ``MixedBehaviorProfile``
    associated with a given ``Player``.  The  full profile is accessible via the `profile`
    attribute, and the player for whom the  ``MixedBehavior`` applies is accessible
    via `player`.
    """
    _profile = cython.declare(MixedBehaviorProfile)
    _player = cython.declare(Player)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a MixedBehavior outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(profile: MixedBehaviorProfile, player: Player) -> MixedBehavior:
        obj: MixedBehavior = MixedBehavior.__new__(MixedBehavior)
        obj._profile = profile
        obj._player = player
        return obj

    @property
    def profile(self) -> MixedBehaviorProfile:
        """The full profile of which this is a part."""
        return self._profile

    @property
    def player(self) -> Player:
        """The player for whom this mixed behavior strategy is defined."""
        return self._player

    def __repr__(self) -> str:
        return str([self[next(iter(infoset.members))] for infoset in self.player.infosets])

    def _repr_latex_(self) -> str:
        if isinstance(self.profile, MixedBehaviorProfileRational):
            return (
                r"$\left[" +
                ",".join(self[next(iter(infoset.members))]._repr_latex_().replace("$", "")
                         for infoset in self.player.infosets) +
                r"\right]$"
            )
        return repr(self)

    def __eq__(self, other: typing.Any) -> bool:
        values = [self[next(iter(infoset.members))] for infoset in self.player.infosets]
        if isinstance(other, list):
            return values == other
        if not isinstance(other, MixedBehavior) or self.player != other.player:
            return False
        return values == [
            other[next(iter(infoset.members))] for infoset in other.player.infosets
        ]

    def __len__(self) -> int:
        return len(self.player.infosets)

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
        for infoset in self.player.infosets:
            yield infoset, self[infoset]

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
        if infoset is not None and infoset.player != self.player:
            raise MismatchError("node must belong to this player")
        return self.profile[index]

    def __setitem__(self, index: Node, value: typing.Any) -> None:
        """Sets the mixed action at the information set containing `index`.

        Parameters
        ----------
        index : Node
            A node belonging to the information set to set.
        value
            The distribution to assign over the information set's actions.

        Raises
        ------
        MismatchError
            If `index` is a ``Node`` from a different game, or belongs to an
            information set that isn't this player's.
        ValueError
            If `index` is a terminal node, which belongs to no information set.
        """
        infoset = cython.cast(NodeInfoset, index.infoset)._resolve()
        if infoset is not None and infoset.player != self.player:
            raise MismatchError("node must belong to this player")
        self.profile[index] = value


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
            if index.game != self.game:
                raise MismatchError("node must belong to this game")
            infoset = cython.cast(NodeInfoset, index.infoset)._resolve()
            if infoset is None:
                raise ValueError("node is terminal, has no information set")
            return MixedAction.wrap(self, infoset)
        if isinstance(index, str):
            return MixedBehavior.wrap(self, self.game._resolve_player(index, "__getitem__"))
        raise TypeError(
            f"profile index must be str or Node, not {index.__class__.__name__}"
        )

    def _setprob_infoset(self, infoset: Infoset, value: typing.Any) -> None:
        if len(infoset.actions) != len(value):
            raise ValueError(
                "when setting an agent strategy, must specify exactly one value per action"
            )
        for a, v in zip(infoset.actions, value):
            self._setprob_action(a, v)

    def _setprob_player(self, player: Player, value: typing.Any) -> None:
        if len(player.infosets) != len(value):
            raise ValueError(
                "when setting a behavior strategy, must specify exactly one distribution "
                "per infoset"
            )
        for s, v in zip(player.infosets, value):
            self._setprob_infoset(s, v)

    def __setitem__(self, index: typing.Any, value: typing.Any) -> None:
        """Sets a component of the mixed behavior profile specified by `index` to `value`.

        Parameters
        ----------
        index : str or Node
            The part of the profile to set:

            * If `index` is a ``str``, sets the ``MixedBehavior`` over the player's
              information sets. The player is determined by finding the player with
              that label, if any.
            * If `index` is a ``Node``, sets the ``MixedAction`` over the actions at
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
            if index.game != self.game:
                raise MismatchError("node must belong to this game")
            infoset = cython.cast(NodeInfoset, index.infoset)._resolve()
            if infoset is None:
                raise ValueError("node is terminal, has no information set")
            self._setprob_infoset(infoset, value)
            return
        if isinstance(index, str):
            self._setprob_player(self.game._resolve_player(index, "__setitem__"), value)
            return
        raise TypeError(
            f"profile index must be str or Node, not {index.__class__.__name__}"
        )

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

    def belief(self, node: NodeReference) -> ProfileDType | None:
        """Returns the conditional probability that a node is reached, given that
        its information set is reached.

        The conditioning event is that the information set is reached at least once,
        so beliefs are normalized by the upper-frontier probability returned by
        `infoset_prob` (following :cite:p:`HalPas21`), rather than by the sum of the
        members' realization probabilities.  For a non-absent-minded information set
        the two approaches agree.  For an absent-minded information set they need not:
        the beliefs over its members may sum to more than one.

        If the information set is reached with zero probability under the profile, the
        belief is not well-defined and the function returns `None`.  This is the same
        reach probability returned by `infoset_prob`, so a `None` belief corresponds
        exactly to `infoset_prob` being zero there.

        Parameters
        ----------
        node
            The node of the game tree

        Raises
        ------
        MismatchError
            If `node` is not in the same game as the profile

        See Also
        --------
        MixedBehaviorProfile.infoset_prob
        """
        self._check_validity()
        return self._belief(self.game._resolve_node(node, "belief"))

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
            If `player` is a ``Player`` from a different game.
        KeyError
            If `player` is a string and no player in the game has that label.
        ValueError
            If `player` resolves to the chance player
        """
        self._check_validity()
        resolved_player = self.game._resolve_player(player, "payoff")
        if resolved_player.is_chance:
            raise ValueError("payoff() is not defined for the chance player")
        return self._payoff(resolved_player)

    def node_value(self, player: PlayerReference,
                   node: NodeReference) -> ProfileDType:
        """Returns the expected payoff to `player` conditional on play reaching `node`,
        if all players play according to the profile.

        Parameters
        ----------
        player : Player or str
            The player to get the payoff for.  If a string is passed, the
            player is determined by finding the player with that label, if any.
        node : Node or str
            The node to get the payoff at.  If a string is passed, the
            node is determined by finding the node with that label, if any.

        Raises
        ------
        MismatchError
            If `player` is a ``Player`` from a different game or `node` is a ``Node``
            from a different game.
        KeyError
            If `player` is a string and no player in the game has that label, or
            `node` is a string and no node in the game has that label.
        ValueError
            If `player` resolves to the chance player
        """
        self._check_validity()
        resolved_player = self.game._resolve_player(player, "node_value")
        resolved_node = self.game._resolve_node(node, "node_value")
        if resolved_player.is_chance:
            raise ValueError("node_value() is not defined for the chance player")
        return self._node_value(resolved_player, resolved_node)

    def infoset_value(self, infoset: InfosetReference) -> ProfileDType | None:
        """Returns the expected payoff to the player conditional on reaching an information set,
        if all players play according to the profile.

        If the information set is not reachable, the expected payoff is not well-defined.
        In this case, the function returns `None`.

        Parameters
        ----------
        infoset : Infoset or str
            The information set to get the payoff for.  If a string is passed, the
            information set is determined by finding the information set with that label, if any.

        Raises
        ------
        MismatchError
            If `infoset` is an ``Infoset`` from a different game.
        KeyError
            If `infoset` is a string and no information set in the game has that label.
        ValueError
            If `infoset` resolves to an infoset that belongs to the chance player

        See Also
        --------
        MixedBehaviorProfile.infoset_prob
        """
        self._check_validity()
        resolved_infoset = self.game._resolve_infoset(infoset, "infoset_value")
        if resolved_infoset.player.is_chance:
            raise ValueError("infoset_value() is not defined for the chance player")
        return self._infoset_value(resolved_infoset)

    def action_value(self, action: ActionReference) -> ProfileDType | None:
        """Returns the expected payoff to the player of playing an action conditional on reaching
        its information set, if all players play according to the profile.

        If the information set is not reachable, the expected payoff is not well-defined.
        In this case, the function returns `None`.

        Parameters
        ----------
        action : Action or str
            The action to get the payoff for.  If a string is passed, the
            action is determined by finding the action with that label, if any.

        Raises
        ------
        MismatchError
            If `action` is an ``Action`` from a different game.
        KeyError
            If `action` is a string and no action in the game has that label.
        ValueError
            If `action` resolves to an action that belongs to the chance player

        See Also
        --------
        MixedBehaviorProfile.infoset_prob
        """
        self._check_validity()
        resolved_action = self.game._resolve_action(action, "action_value")
        if resolved_action.infoset.player.is_chance:
            raise ValueError("action_value() is not defined for the chance player")
        return self._action_value(resolved_action)

    def realiz_prob(self, node: NodeReference) -> ProfileDType:
        """Returns the probability with which a node is reached.

        Parameters
        ----------
        node : Node or str
            The node to get the payoff for.  If a string is passed, the
            node is determined by finding the node with that label, if any.

        Raises
        ------
        MismatchError
            If `node` is a ``Node`` from a different game.
        KeyError
            If `node` is a string and no node in the game has that label.
        """
        self._check_validity()
        return self._realiz_prob(self.game._resolve_node(node, "realiz_prob"))

    def infoset_prob(self, infoset: InfosetReference) -> ProfileDType:
        """Returns the probability with which an information set is reached.

        This is the probability that the information set is reached *at least once*
        under the profile: the realization probability of its upper frontier, i.e. the
        members not preceded by another member of the same information set.
        For a non-absent-minded information set, its upper frontier coincides with it.
        For an absent-minded information set, with a play passing through it more than once,
        the members below its frontier are excluded, so this is generally less than
        the sum of the members' realization probabilities; see :cite:p:`HalPas21`.

        Parameters
        ----------
        infoset : Infoset or str
            The information set to get the probability for.  If a string is passed, the
            information set is determined by finding the information set with that label, if any.

        Raises
        ------
        MismatchError
            If `infoset` is an ``Infoset`` from a different game.
        KeyError
            If `infoset` is a string and no information set in the game has that label.

        See Also
        --------
        MixedBehaviorProfile.belief
        """
        self._check_validity()
        return self._infoset_prob(self.game._resolve_infoset(infoset, "infoset_prob"))

    def action_regret(self, action: ActionReference) -> ProfileDType:
        """Returns the regret to playing `action`, if all other
        players play according to the profile.

        The regret is defined as the difference between the payoff of the
        best-response action and the payoff of `action`.  Payoffs are computed
        conditional on reaching the information set.  By convention, the
        regret is always non-negative.

        .. versionchanged:: 16.2.0

            Changed from `regret()` to disambiguate from other regret concepts.

        Parameters
        ----------
        action : Action or str
            The action to get the regret for.  If a string is passed, the
            action is determined by finding the action with that label, if any.

        Raises
        ------
        MismatchError
            If `action` is an ``Action`` from a different game.
        KeyError
            If `action` is a string and no action in the game has that label.

        See Also
        --------
        infoset_regret
        max_regret
        """
        self._check_validity()
        return self._action_regret(self.game._resolve_action(action, "action_regret"))

    def infoset_regret(self, infoset: InfosetReference) -> ProfileDType:
        """Returns the regret to the player for playing their mixed action at
        `infoset`, if all other players play according to the profile.

        The regret is defined as the difference between the payoff of the
        best-response action and the payoff of the player's mixed action.
        Payoffs are computed conditional on reaching the information set.
        By convention, the regret is always non-negative.

        .. versionadded:: 16.2.0

        Parameters
        ----------
        infoset : Infoset or str
            The information set to get the regret at.  If a string is passed, the
            information set is determined by finding the information set with that
            label, if any.

        Raises
        ------
        MismatchError
            If `infoset` is an ``Infoset`` from a different game.
        KeyError
            If `infoset` is a string and no information set in the game has that label.

        See Also
        --------
        action_regret
        agent_max_regret
        """
        self._check_validity()
        return self._infoset_regret(self.game._resolve_infoset(infoset, "infoset_regret"))

    def agent_max_regret(self) -> ProfileDType:
        """Returns the maximum regret at any information set.

        A profile is an agent Nash equilibrium if and only if `agent_max_regret()` is 0.

        .. versionchanged:: 16.5.0

           Renamed from `max_regret` to `agent_max_regret` to clarify the distinction between
           per-player and per-agent concepts.

        See Also
        --------
        action_regret
        infoset_regret
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
        """Creates a copy of the behavior strategy profile."""
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

    def _setprob_action(self, index: Action, value) -> None:
        setitem_mbpd_action(deref(self.profile), index.action, value)

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
        return MixedBehaviorProfileDouble.wrap(
            make_shared[c_MixedBehaviorProfile[double]](deref(self.profile))
        )

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

    def _setprob_action(self, index: Action, value: typing.Any) -> None:
        if not isinstance(value, (int, fractions.Fraction)):
            raise TypeError(
                f"rational precision profile requires int or Fraction probability, "
                f"not {value.__class__.__name__}"
            )
        setitem_mbpr_action(deref(self.profile), index.action,
                            to_rational(str(value).encode("ascii")))

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
        return MixedBehaviorProfileRational.wrap(
            make_shared[c_MixedBehaviorProfile[c_Rational]](deref(self.profile))
        )

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
