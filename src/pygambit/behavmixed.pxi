#
# This file is part of Gambit
# Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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
from cython.operator cimport dereference as deref


class MixedAction:
    """A probability distribution over a player's actions at an information set.

    A ``MixedAction`` represents a component of a ``MixedBehaviorProfile``.  The
    full profile is accessible via the `profile` attribute, and the information set
    at which the ``MixedAction`` applies is accessible via `infoset`.
    """
    def __init__(self, profile: MixedBehaviorProfile, infoset: Infoset) -> None:
        self._profile = profile
        self._infoset = infoset

    @property
    def profile(self) -> MixedBehaviorProfile:
        """The full profile of which this is a part."""
        return self._profile

    @property
    def infoset(self) -> Infoset:
        """The information set over which this mixed action is defined."""
        return self._infoset

    def __repr__(self) -> str:
        return str([self.profile[action] for action in self.infoset.actions])

    def _repr_latex_(self) -> str:
        if isinstance(self.profile, MixedBehaviorProfileRational):
            return (
                r"$\left[" +
                ",".join(self._profile[act]._repr_latex_().replace("$", "")
                         for act in self.infoset.actions) +
                r"\right]$"
            )
        return repr(self)

    def __eq__(self, other: typing.Any) -> bool:
        if isinstance(other, list):
            return [self[action] for action in self.infoset.actions] == other
        if not isinstance(other, MixedAction) or self.infoset != other.infoset:
            return False
        return (
            [self[action] for action in self.infoset.actions] ==
            [other[action] for action in other.infoset.actions]
        )

    def __len__(self) -> len:
        return len(self.infoset.actions)

    def __iter__(self) -> typing.Iterator[typing.Tuple[Action, ProfileDType], None, None]:
        """Iterate over the probabilities assigned to actions by the mixed action.

        .. versionadded:: 16.2.0

        Yields
        ------
        action : Action
            An action at the information set
        probability : float or Rational
            The probability the mixed action assigns to the action being played
        """
        for action in self.infoset.actions:
            yield action, self[action]

    def __getitem__(self, index: ActionReference) -> ProfileDType:
        """Returns the probability that the action referred to by `index` is played.

        Parameters
        ----------
        index : Action or str

            * If `index` is an ``Action``, returns the probability the action is played.
            * If `index` is a ``str``, attempts to resolve the referenced object by searching
              for an action with that label.

        Returns
        -------
        float or Rational
            The probability assigned to the action.

        Raises
        ------
        MismatchError
            If `index` is an ``Action`` that does not belong to this ``MixedAction``'s
            information set.
        """
        self.profile._check_validity()
        if isinstance(index, Action):
            if index.infoset != self.infoset:
                raise MismatchError("action must belong to this infoset")
            return self.profile._getprob_action(index)
        if isinstance(index, str):
            try:
                return self.profile._getprob_action(self.infoset.actions[index])
            except KeyError:
                raise KeyError(f"no action with label '{index}' at infoset") from None
        raise TypeError(f"strategy index must be Action or str, not {index.__class__.__name__}")

    def __setitem__(self, index: ActionReference, value: typing.Any) -> None:
        """Sets the probability an action is played.

        Parameters
        ----------
        index : Action or str
            The part of the profile to set:

            * If `index` is an ``Action``, sets the probability the action is played.
            * If `index` is a ``str``, attempts to resolve the referenced object by searching
              for an action with that label, and sets the probability for that action.

        value
            Any value which can be converted to the data type of the ``MixedBehaviorProfile``.

        Raises
        ------
        MismatchError
            If `action` is an ``Action`` that does not belong to this ``MixedAction``'s
            information set.
        """
        self.profile._check_validity()
        if isinstance(index, Action):
            if index.infoset != self.infoset:
                raise MismatchError("action must belong to this infoset")
            self.profile._setprob_action(index, value)
            return
        if isinstance(index, str):
            try:
                self.profile._setprob_action(self.infoset.actions[index], value)
                return
            except KeyError:
                raise KeyError(f"no action with label '{index}' at infoset") from None
        raise TypeError(f"strategy index must be Action or str, not {index.__class__.__name__}")


class MixedBehavior:
    """A set of probability distributions describing a player's behavior.

    A ``MixedBehavior`` represents the component of a ``MixedBehaviorProfile``
    associated with a given ``Player``.  The  full profile is accessible via the `profile`
    attribute, and the player for whom the  ``MixedBehavior`` applies is accessible
    via `player`.
    """
    def __init__(self, profile: MixedBehaviorProfile, player: Player) -> None:
        self._profile = profile
        self._player = player

    @property
    def profile(self) -> MixedBehaviorProfile:
        """The full profile of which this is a part."""
        return self._profile

    @property
    def player(self) -> Player:
        """The player for whom this mixed behavior strategy is defined."""
        return self._player

    def __repr__(self) -> str:
        return str([self.profile[infoset] for infoset in self.player.infosets])

    def _repr_latex_(self) -> str:
        if isinstance(self.profile, MixedBehaviorProfileRational):
            return (
                r"$\left[" +
                ",".join(self.profile[infoset]._repr_latex_().replace("$", "")
                         for infoset in self.player.infosets) +
                r"\right]$"
            )
        return repr(self)

    def __eq__(self, other: typing.Any) -> bool:
        if isinstance(other, list):
            return [self[infoset] for infoset in self.player.infosets] == other
        if not isinstance(other, MixedBehavior) or self.player != other.player:
            return False
        return (
            [self[infoset] for infoset in self.player.infosets] ==
            [other[infoset] for infoset in other.player.infosets]
        )

    def __len__(self) -> int:
        return len(self.player.actions)

    def mixed_actions(self) -> typing.Iterator[typing.Tuple[Infoset, MixedAction], None, None]:
        """Iterate over the mixed actions specified by the mixed behavior.

        .. versionadded:: 16.2.0

        Yields
        ------
        infoset : Infoset
            An information set belonging to the player
        action : MixedAction
            The player's mixed action specified in the mixed behavior
        """
        for infoset in self.player.infosets:
            yield infoset, self[infoset]

    def __iter__(self) -> typing.Iterator[typing.Tuple[Action, ProfileDType], None, None]:
        """Iterate over the probabilities assigned to actions by the mixed behavior.

        .. versionadded:: 16.2.0

        Yields
        ------
        action : Action
            An action for the player
        probability : float or Rational
            The probability the behavior assigns to the action being played
        """
        for action in self.player.actions:
            yield action, self[action]

    def __getitem__(
            self,
            index: typing.Union[InfosetReference, ActionReference]
    ) -> typing.Union[MixedAction, ProfileDType]:
        """Access a component of the mixed behavior specified by `index`.

        Parameters
        ----------
        index : Infoset, Action, or str
            The part of the mixed behavior to return:

            * If `index` is an ``Infoset``, returns a ``MixedAction`` over the infoset's actions
            * If `index` is an ``Action``, returns the probability the action is played
            * If `index` is a ``str``, attempts to resolve the referenced object by first searching
              for an infoset with that label, and then for an action with that label.

        Raises
        ------
        MismatchError
            If `infoset` not an ``Infoset`` for the mixed behavior's player, or `action`
            is not an ``Action`` for the mixed behavior's player.
        """
        if isinstance(index, Infoset):
            if index.player != self.player:
                raise MismatchError("infoset must belong to this player")
            return self.profile[index]
        if isinstance(index, Action):
            if index.player != self.player:
                raise MismatchError("action must belong to this player")
            return self.profile[index]
        if isinstance(index, str):
            try:
                return self.profile[self.player.infosets[index]]
            except KeyError:
                pass
            try:
                return self.profile[self.player.actions[index]]
            except KeyError:
                raise KeyError(f"no infoset or action with label '{index}' for player") from None
        raise TypeError(
            f"behavior index must be Infoset, Action or str, not {index.__class__.__name__}"
        )

    def __setitem__(self,
                    index: typing.Union[InfosetReference, ActionReference],
                    value: typing.Any) -> None:
        """Sets a component of the mixed behavior to `value`.

        Parameters
        ----------
        index : Infoset, Action, or str
            The component of the mixed behavior to set:

            * If `index` is an `Infoset`, sets the mixed action over that infoset's actions
            * If `index` is an `Action`, sets the probability the action is played
            * If `index` is a `str`, attempts to resolve the referenced object by first searching
              for an infoset with that label, and then for an action with that label.

        Raises
        ------
        MismatchError
            If `infoset` not an ``Infoset`` for the mixed behavior's player, or `action`
            is not an ``Action`` for the mixed behavior's player.
        """
        if isinstance(index, Infoset):
            if index.player != self.player:
                raise MismatchError("infoset must belong to this player")
            self.profile[index] = value
            return
        if isinstance(index, Action):
            if index.player != self.player:
                raise MismatchError("action must belong to this player")
            self.profile[index] = value
            return
        if isinstance(index, str):
            try:
                self.profile[self.player.infosets[index]] = value
                return
            except KeyError:
                pass
            try:
                self.profile[self.player.actions[index]] = value
            except KeyError:
                raise KeyError(f"no infoset or action with label '{index}' for player") from None
            return
        raise TypeError(
            f"behavior index must be Infoset, Action or str, not {index.__class__.__name__}"
        )


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
    def __repr__(self) -> str:
        return str([self[player] for player in self.game.players])

    def _repr_latex_(self) -> str:
        return (
            r"$\left[" +
            ",".join([self[player]._repr_latex_().replace("$", "")
                      for player in self.game.players])
            + r"\right]$"
        )

    @property
    def game(self) -> Game:
        """The game on which this mixed behavior profile is defined."""
        return self._game

    def mixed_behaviors(self) -> typing.Iterator[typing.Tuple[Player, MixedBehavior], None, None]:
        """Iterate over the mixed behaviors in the profile.

        .. versionadded:: 16.2.0

        Yields
        ------
        player : Player
            A player in the game
        behavior : MixedBehavior
            The player's mixed behavior specified in the profile
        """
        for player in self.game.players:
            yield player, self[player]

    def mixed_actions(self) -> typing.Iterator[typing.Tuple[Infoset, MixedAction], None, None]:
        """Iterate over the mixed actions specified by the profile.

        .. versionadded:: 16.2.0

        Yields
        ------
        infoset : Infoset
            An information set in the game
        action : MixedAction
            The mixed action specified at the information set by the profile.
        """
        for infoset in self.game.infosets:
            yield infoset, self[infoset]

    def __iter__(self) -> typing.Iterator[typing.Tuple[Action, ProfileDType], None, None]:
        """Iterate over the probabilities assigned to actions by the profile.

        .. versionadded:: 16.2.0

        Yields
        ------
        action : Action
            An action in the game
        probability : float or Rational
            The probability the profile assigns to the action being played
        """
        for action in self.game.actions:
            yield action, self[action]

    def __getitem__(
            self,
            index: typing.Union[PlayerReference, InfosetReference, ActionReference]
    ) -> typing.Union[MixedBehavior, MixedAction, ProfileDType]:
        """Access a component of the mixed behavior specified by `index`.

        Parameters
        ----------
        index : Player, Infoset, Action, or str
            The part of the profile to return:

            * If `index` is a ``Player``, returns a ``MixedBehavior`` over the player's infosets
            * If `index` is an ``Infoset``, returns a ``MixedAction`` over the infoset's actions
            * If `index` is an ``Action``, returns the probability the action is played
            * If `index` is a ``str``, attempts to resolve the referenced object by first searching
              for a player with that label, then for an infoset with that label, and finally for an
              action with that label.

        Raises
        ------
        MismatchError
            If `player` is a ``Player`` from a different game, `infoset` is an ``Infoset`` from
            a different game, or `action` is an ``Action`` from a different game.`
        """
        self._check_validity()
        if isinstance(index, Action):
            if index.infoset.game != self.game:
                raise MismatchError("action must belong to this game")
            return self._getprob_action(index)
        if isinstance(index, Infoset):
            if index.game != self.game:
                raise MismatchError("infoset must belong to this game")
            return MixedAction(self, index)
        if isinstance(index, Player):
            if index.game != self.game:
                raise MismatchError("player must belong to this game")
            return MixedBehavior(self, index)
        if isinstance(index, str):
            try:
                return MixedBehavior(self, self.game._resolve_player(index, "__getitem__"))
            except KeyError:
                pass
            try:
                return MixedAction(self, self.game._resolve_infoset(index, "__getitem__"))
            except KeyError:
                pass
            try:
                return self._getprob_action(self.game._resolve_action(index, "__getitem__"))
            except KeyError:
                raise KeyError(f"no player, infoset, or action with label '{index}'")
        raise TypeError(
            f"profile index must be Player, Infoset, Action, or str, "
            f"not {index.__class__.__name__}"
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

    def __setitem__(
            self,
            index: typing.Union[PlayerReference, InfosetReference, ActionReference],
            value: typing.Any
    ) -> None:
        """Sets a probability, mixed agent strategy, or mixed behavior strategy to `value`.

        Parameters
        ----------
        index : Player, Infoset, Action, or str
            The part of the profile to return:

            * If `index` is a ``Player``, sets the ``MixedBehavior`` over the player's infosets
            * If `index` is an ``Infoset``, sets the ``MixedAction`` over the infoset's actions
            * If `index` is an ``Action``, sets the probability the action is played
            * If `index` is a ``str``, attempts to resolve the referenced object by first searching
              for a player with that label, then for an infoset with that label, and finally for an
              action with that label.

        Raises
        ------
        MismatchError
            If `player` is a ``Player`` from a different game, `infoset` is an ``Infoset`` from a
            different game, or `action` is an ``Action`` from a different game.`
        """
        self._check_validity()
        if isinstance(index, Action):
            if index.infoset.game != self.game:
                raise MismatchError("action must belong to this game")
            self._setprob_action(index, value)
            return
        if isinstance(index, Infoset):
            if index.game != self.game:
                raise MismatchError("infoset must belong to this game")
            self._setprob_infoset(index, value)
            return
        if isinstance(index, Player):
            if index.game != self.game:
                raise MismatchError("player must belong to this game")
            self._setprob_player(index, value)
            return
        if isinstance(index, str):
            try:
                self._setprob_player(self.game._resolve_player(index, "__getitem__"), value)
                return
            except KeyError:
                pass
            try:
                self._setprob_infoset(self.game._resolve_infoset(index, "__getitem__"), value)
                return
            except KeyError:
                pass
            try:
                self._setprob_action(self.game._resolve_action(index, "__getitem__"), value)
            except KeyError:
                raise KeyError(f"no player, infoset, or action with label '{index}'")
            return
        raise TypeError(
            f"profile index must be Player, Infoset, Action, or str, "
            f"not {index.__class__.__name__}"
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

    def belief(self, node: NodeReference) -> ProfileDType:
        """Returns the conditional probability that a node is reached, given that
        its information set is reached.

        Parameters
        ----------
        node
            The node of the game tree

        Raises
        ------
        MismatchError
            If `node` is not in the same game as the profile
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

    def infoset_value(self, infoset: InfosetReference) -> ProfileDType:
        """Returns the expected payoff to the player conditional on reaching an information set,
        if all players play according to the profile.

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
        """
        self._check_validity()
        resolved_infoset = self.game._resolve_infoset(infoset, "infoset_value")
        if resolved_infoset.player.is_chance:
            raise ValueError("infoset_value() is not defined for the chance player")
        return self._infoset_value(resolved_infoset)

    def action_value(self, action: ActionReference) -> ProfileDType:
        """Returns the expected payoff to the player of playing an action conditional on reaching
        its information set, if all players play according to the profile.

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

    def infoset_prob(self, infoset: NodeReference) -> ProfileDType:
        """Returns the probability with which an information set is reached.

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
        max_regret
        """
        self._check_validity()
        return self._infoset_regret(self.game._resolve_infoset(infoset, "infoset_regret"))

    def max_regret(self) -> ProfileDType:
        """Returns the maximum regret of any player.

        A profile is an agent Nash equilibrium if and only if `max_regret()` is 0.

        .. versionadded:: 16.2.0

        See Also
        --------
        action_regret
        infoset_regret
        liap_value
        """
        self._check_validity()
        return self._max_regret()

    def liap_value(self) -> ProfileDType:
        """Returns the Lyapunov value (see [McK91]_) of the strategy profile.

        The Lyapunov value is a non-negative number which is zero exactly at
        agent Nash equilibria.

        See Also
        --------
        max_regret
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
    profile = cython.declare(shared_ptr[c_MixedBehaviorProfileDouble])

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
        return deref(self.profile).GetPayoff(player.player.deref().GetNumber())

    def _belief(self, node: Node) -> float:
        return deref(self.profile).GetBeliefProb(node.node)

    def _realiz_prob(self, node: Node) -> float:
        return deref(self.profile).GetRealizProb(node.node)

    def _infoset_prob(self, infoset: Infoset) -> float:
        return deref(self.profile).GetInfosetProb(infoset.infoset)

    def _infoset_value(self, infoset: Infoset) -> float:
        return deref(self.profile).GetPayoff(infoset.infoset)

    def _node_value(self, player: Player, node: Node) -> float:
        return deref(self.profile).GetPayoff(player.player, node.node)

    def _action_value(self, action: Action) -> float:
        return deref(self.profile).GetPayoff(action.action)

    def _action_regret(self, action: Action) -> float:
        return deref(self.profile).GetRegret(action.action)

    def _infoset_regret(self, infoset: Infoset) -> float:
        return deref(self.profile).GetRegret(infoset.infoset)

    def _max_regret(self) -> float:
        return deref(self.profile).GetMaxRegret()

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, MixedBehaviorProfileDouble) and
            deref(self.profile) == deref(cython.cast(MixedBehaviorProfileDouble, other).profile)
        )

    def _copy(self) -> MixedBehaviorProfileDouble:
        behav = MixedBehaviorProfileDouble()
        behav.profile = make_shared[c_MixedBehaviorProfileDouble](deref(self.profile))
        return behav

    def _as_strategy(self) -> MixedStrategyProfileDouble:
        mixed = MixedStrategyProfileDouble()
        mixed.profile = (
            make_shared[c_MixedStrategyProfileDouble](deref(self.profile).ToMixedProfile())
        )
        return mixed

    def _liap_value(self) -> float:
        return deref(self.profile).GetLiapValue()

    def _normalize(self) -> MixedBehaviorProfileDouble:
        profile = MixedBehaviorProfileDouble()
        profile.profile = (
            make_shared[c_MixedBehaviorProfileDouble](deref(self.profile).Normalize())
        )
        return profile

    @property
    def _game(self) -> Game:
        g = Game()
        g.game = deref(self.profile).GetGame()
        return g


@cython.cclass
class MixedBehaviorProfileRational(MixedBehaviorProfile):
    profile = cython.declare(shared_ptr[c_MixedBehaviorProfileRational])

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
        return rat_to_py(deref(self.profile).GetPayoff(player.player.deref().GetNumber()))

    def _belief(self, node: Node) -> Rational:
        return rat_to_py(deref(self.profile).GetBeliefProb(node.node))

    def _realiz_prob(self, node: Node) -> Rational:
        return rat_to_py(deref(self.profile).GetRealizProb(node.node))

    def _infoset_prob(self, infoset: Infoset) -> Rational:
        return rat_to_py(deref(self.profile).GetInfosetProb(infoset.infoset))

    def _infoset_value(self, infoset: Infoset) -> Rational:
        return rat_to_py(deref(self.profile).GetPayoff(infoset.infoset))

    def _node_value(self, player: Player, node: Node) -> Rational:
        return rat_to_py(deref(self.profile).GetPayoff(player.player, node.node))

    def _action_value(self, action: Action) -> Rational:
        return rat_to_py(deref(self.profile).GetPayoff(action.action))

    def _action_regret(self, action: Action) -> Rational:
        return rat_to_py(deref(self.profile).GetRegret(action.action))

    def _infoset_regret(self, infoset: Infoset) -> Rational:
        return rat_to_py(deref(self.profile).GetRegret(infoset.infoset))

    def _max_regret(self) -> Rational:
        return rat_to_py(deref(self.profile).GetMaxRegret())

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, MixedBehaviorProfileRational) and
            deref(self.profile) == deref(cython.cast(MixedBehaviorProfileRational, other).profile)
        )

    def _copy(self) -> MixedBehaviorProfileRational:
        behav = MixedBehaviorProfileRational()
        behav.profile = make_shared[c_MixedBehaviorProfileRational](deref(self.profile))
        return behav

    def _as_strategy(self) -> MixedStrategyProfileRational:
        mixed = MixedStrategyProfileRational()
        mixed.profile = (
            make_shared[c_MixedStrategyProfileRational](deref(self.profile).ToMixedProfile())
        )
        return mixed

    def _liap_value(self) -> Rational:
        return rat_to_py(deref(self.profile).GetLiapValue())

    def _normalize(self) -> MixedBehaviorProfileRational:
        profile = MixedBehaviorProfileRational()
        profile.profile = (
            make_shared[c_MixedBehaviorProfileRational](deref(self.profile).Normalize())
        )
        return profile

    @property
    def _game(self) -> Game:
        g = Game()
        g.game = deref(self.profile).GetGame()
        return g
