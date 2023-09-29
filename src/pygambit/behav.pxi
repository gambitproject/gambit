#
# This file is part of Gambit
# Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/lib/behav.pxi
# Cython wrapper for behavior strategies
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


class MixedAgentStrategy:
    """Represents a probability distribution over a player's actions at an information set.
    """
    def __init__(self, profile, infoset):
        self.profile = profile
        self.infoset = infoset

    def __repr__(self) -> str:
        return str([self.profile[action] for action in self.infoset.actions])

    def _repr_latex_(self) -> str:
        if isinstance(self.profile, MixedBehaviorProfileRational):
            return r"$\left[" + ",".join(
                self.profile[i]._repr_latex_().replace("$", "") for i in self.infoset.actions) + r"\right]$"
        else:
            return repr(self)

    def __eq__(self, other: typing.Any) -> bool:
        if isinstance(other, list):
            return [self[action] for action in self.infoset.actions] == other
        if not isinstance(other, MixedAgentStrategy) or self.infoset != other.infoset:
            return False
        return (
            [self[action] for action in self.infoset.actions] ==
            [other[action] for action in other.infoset.actions]
        )

    def __len__(self) -> len:
        return len(self.infoset.actions)

    def __getitem__(self, action: typing.Union[Action, str]):
        if isinstance(action, Action) and action.infoset != self.infoset:
            raise MismatchError("action must belong to this infoset")
        return self.profile[action]

    def __setitem__(self, action: typing.Union[Action, str], value: typing.Any) -> None:
        if isinstance(action, Action) and action.infoset != self.infoset:
            raise MismatchError("action must belong to this infoset")
        self.profile[action] = value


class MixedBehaviorStrategy:
    """Represents a probability distribution over a player's actions."""
    def __init__(self, profile, player):
        self.profile = profile
        self.player = player

    def __repr__(self) -> str:
        return str([self.profile[infoset] for infoset in self.player.infosets])

    def _repr_latex_(self) -> str:
        if isinstance(self.profile, MixedBehaviorProfileRational):
            return r"$\left[" + ",".join(
                self.profile[i]._repr_latex_().replace("$", "") for i in self.player.infosets) + r"\right]$"
        else:
            return repr(self)

    def __eq__(self, other: typing.Any) -> bool:
        if isinstance(other, list):
            return [self[infoset] for infoset in self.player.infosets] == other
        if not isinstance(other, MixedBehaviorStrategy) or self.player != other.player:
            return False
        return (
            [self[infoset] for infoset in self.player.infosets] ==
            [other[infoset] for infoset in other.player.infosets]
        )

    def __len__(self) -> int:
        return len(self.player.infosets)

    def __getitem__(self, infoset: typing.Union[Infoset, str]):
        if isinstance(infoset, Infoset) and infoset.player != self.player:
            raise MismatchError("infoset must belong to this player")
        return self.profile[infoset]

    def __setitem__(self, infoset: typing.Union[Infoset, str], value: typing.Any) -> None:
        if isinstance(infoset, Infoset) and infoset.player != self.player:
            raise MismatchError("infoset must belong to this player")
        self.profile[infoset] = value


@cython.cclass
class MixedBehaviorProfile:
    """A behavior strategy profile over the actions in a game."""
    def __repr__(self) -> str:
        return str([self[player] for player in self.game.players])

    def _repr_latex_(self) -> str:
        return (
            r"$\left[" +
            ",".join([self[player]._repr_latex_().replace("$","") for player in self.game.players])
            + r"\right]$"
        )

    def __getitem__(self, index: typing.Union[Player, Infoset, Action, str]):
        """Returns a probability, mixed agent strategy, or mixed behavior strategy.

        Parameters
        ----------
        index : Player, Infoset, Action, or str
            The part of the profile to return:

            * If `index` is a `Player`, returns a ``MixedBehaviorStrategy`` over the player's infosets
            * If `index` is an `Infoset`, returns a ``MixedAgentStrategy`` over the infoset's actions
            * If `index` is an `Action`, returns the probability the action is playe
            * If `index` is a `str`, attempts to resolve the referenced object by first searching
              for a player with that label, then for an infoset with that label, and finally for an
              action with that label.

        Raises
        ------
        MismatchError
            If `player` is a ``Player`` from a different game, `infoset` is an ``Infoset`` from a different
            game, or `action` is an ``Action`` from a different game.`
        """
        if isinstance(index, Action):
            if index.infoset.game != self.game:
                raise MismatchError("action must belong to this game")
            return self._getprob_action(index)
        if isinstance(index, Infoset):
            if index.game != self.game:
                raise MismatchError("infoset must belong to this game")
            return MixedAgentStrategy(self, index)
        if isinstance(index, Player):
            if index.game != self.game:
                raise MismatchError("player must belong to this game")
            return MixedBehaviorStrategy(self, index)
        if isinstance(index, str):
            try:
                return MixedBehaviorStrategy(self, self.game._resolve_player(index, '__getitem__'))
            except KeyError:
                pass
            try:
               return MixedAgentStrategy(self, self.game._resolve_infoset(index, '__getitem__'))
            except KeyError:
                pass
            return self._getprob_action(self.game._resolve_action(index, '__getitem__'))
        raise TypeError(
            f"profile index must be Player, Infoset, Action, or str, not {index.__class__.__name__}"
        )

    def _setprob_infoset(self, infoset: Infoset, value: typing.Any) -> None:
        if len(infoset.actions) != len(value):
            raise ValueError("when setting an agent strategy, must specify exactly one value per action")
        for a, v in zip(infoset.actions, value):
            self._setprob_action(a, v)

    def _setprob_player(self, player: Player, value: typing.Any) -> None:
        if len(player.infosets) != len(value):
            raise ValueError(
                "when setting a behavior strategy, must specify exactly one distribution per infoset"
            )
        for s, v in zip(player.infosets, value):
            self._setprob_infoset(s, v)

    def __setitem__(self, index: typing.Union[Player, Infoset, Action, str], value: typing.Any) -> None:
        """Sets a probability, mixed agent strategy, or mixed behavior strategy to `value`.

        Parameters
        ----------
        index : Player, Infoset, Action, or str
            The part of the profile to return:

            * If `index` is a `Player`, returns a ``MixedBehaviorStrategy`` over the player's infosets
            * If `index` is an `Infoset`, returns a ``MixedAgentStrategy`` over the infoset's actions
            * If `index` is an `Action`, returns the probability the action is playe
            * If `index` is a `str`, attempts to resolve the referenced object by first searching
              for a player with that label, then for an infoset with that label, and finally for an
              action with that label.

        Raises
        ------
        MismatchError
            If `player` is a `Player` from a different game, `infoset` is an `Infoset` from a different
            game, or `action` is an `Action` from a different game.`
        """
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
                self._setprob_player(self.game._resolve_player(index, '__getitem__'), value)
                return
            except KeyError:
                pass
            try:
                self._setprob_infoset(self.game._resolve_infoset(index, '__getitem__'), value)
                return
            except KeyError:
                pass
            self._setprob_action(self.game._resolve_action(index, '__getitem__'), value)
            return
        raise TypeError(
            f"profile index must be Player, Infoset, Action, or str, not {index.__class__.__name__}"
        )

    def is_defined_at(self, infoset: typing.Union[Infoset, str]) -> bool:
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
            If `infoset` is an `Infoset` from a different game.
        KeyError
            If `infoset` is a string and no information set in the game has that label.
        """
        return self._is_defined_at(self.game._resolve_infoset(infoset, 'is_defined_at'))

    def belief(self, node: Node):
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
        if node.game != self.game:
            raise MismatchError("belief: node must be part of the same game as the profile")
        return self._belief(node)

    def payoff(self, player: typing.Union[Player, str]):
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
        return self._payoff(self.game._resolve_player(player, 'payoff'))

    def infoset_value(self, infoset: typing.Union[Infoset, str]):
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
            If `infoset` is an `Infoset` from a different game.
        KeyError
            If `infoset` is a string and no information set in the game has that label.
        """
        return self._infoset_value(self.game._resolve_infoset(infoset, 'infoset_value'))

    def action_value(self, action: typing.Union[Action, str]):
        """Returns the expected payoff to the player of playing an action conditional on reaching its
        information set, if all players play according to the profile.

        Parameters
        ----------
        action : Action or str
            The action to get the payoff for.  If a string is passed, the
            action is determined by finding the action with that label, if any.

        Raises
        ------
        MismatchError
            If `action` is an `Action` from a different game.
        KeyError
            If `action` is a string and no action in the game has that label.
        """
        return self._action_value(self.game._resolve_action(action, 'action_value'))

    def infoset_prob(self, infoset: typing.Union[Infoset, str]):
        """Returns the probability with which an information set is reached.

        Parameters
        ----------
        infoset : Infoset or str
            The information set to get the payoff for.  If a string is passed, the
            information set is determined by finding the information set with that label, if any.

        Raises
        ------
        MismatchError
            If `infoset` is an `Infoset` from a different game.
        KeyError
            If `infoset` is a string and no information set in the game has that label.
        """
        return self._infoset_prob(self.game._resolve_infoset(infoset, 'infoset_prob'))

    def regret(self, action: typing.Union[Action, str]):
        """Returns the regret associated with `action`.  The regret is the loss of
        payoff relative to the best response to the profile.

        Parameters
        ----------
        action : Action or str
            The action to get the regret for.  If a string is passed, the
            action is determined by finding the action with that label, if any.

        Raises
        ------
        MismatchError
            If `action` is an `Action` from a different game.
        KeyError
            If `action` is a string and no action in the game has that label.
        """
        return self._regret(self.game._resolve_action(action, 'regret'))


@cython.cclass
class MixedBehaviorProfileDouble(MixedBehaviorProfile):
    profile = cython.declare(shared_ptr[c_MixedBehaviorProfileDouble])

    def __len__(self) -> int:
        return deref(self.profile).Length()

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

    def _infoset_prob(self, infoset: Infoset) -> float:
        return deref(self.profile).GetRealizProb(infoset.infoset)

    def _infoset_value(self, infoset: Infoset) -> float:
        return deref(self.profile).GetPayoff(infoset.infoset)

    def _action_value(self, action: Action) -> float:
        return deref(self.profile).GetPayoff(action.action)

    def _regret(self, action: Action) -> float:
        return deref(self.profile).GetRegret(action.action)

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, MixedBehaviorProfileDouble) and
            deref(self.profile) == deref(cython.cast(MixedBehaviorProfileDouble, other).profile)
        )

    def copy(self) -> MixedBehaviorProfileDouble:
        """Creates a copy of the behavior strategy profile."""
        behav = MixedBehaviorProfileDouble()
        behav.profile = make_shared[c_MixedBehaviorProfileDouble](deref(self.profile))
        return behav

    def as_strategy(self) -> MixedStrategyProfileDouble:
        """Returns a `MixedStrategyProfile` which is equivalent
        to the profile.
        """
        mixed = MixedStrategyProfileDouble()
        mixed.profile = make_shared[c_MixedStrategyProfileDouble](deref(self.profile).ToMixedProfile())
        return mixed

    def liap_value(self) -> float:
        """Returns the Lyapunov value (see [McK91]_) of the strategy profile.  The
        Lyapunov value is a non-negative number which is zero exactly at
        Nash equilibria.
        """
        return deref(self.profile).GetLiapValue()

    def normalize(self) -> MixedBehaviorProfileDouble:
        """Create a profile with the same action proportions as this
        one, but normalised so probabilities for each infoset sum to one.
        """
        profile = MixedBehaviorProfileDouble()
        profile.profile = make_shared[c_MixedBehaviorProfileDouble](deref(self.profile).Normalize())
        return profile
    
    def randomize(self, denom=None) -> None:
        """Randomizes the probabilities in the profile.  These are
        generated as uniform distributions over the actions at each
        information set.  If
        ``denom`` is specified, all probabilities are divisible by
        ``denom``, that is, the distribution is uniform over a discrete
        grid of mixed strategies.
        """
        if denom is None:
            deref(self.profile).Randomize()
        else:
            deref(self.profile).Randomize(denom)

    @property
    def game(self) -> Game:
        """The game on which this mixed behaviour profile is defined.
        """
        g = Game()
        g.game = deref(self.profile).GetGame()
        return g


@cython.cclass
class MixedBehaviorProfileRational(MixedBehaviorProfile):
    profile = cython.declare(shared_ptr[c_MixedBehaviorProfileRational])

    def __len__(self) -> int:
        return deref(self.profile).Length()

    def _is_defined_at(self, infoset: Infoset) -> bool:
        return deref(self.profile).IsDefinedAt(infoset.infoset)

    def _getprob_action(self, index: Action) -> Rational:
        return rat_to_py(deref(self.profile).getaction(index.action))

    def _setprob_action(self, index: Action, value: typing.Any) -> None:
        if not isinstance(value, (int, fractions.Fraction)):
            raise TypeError("rational precision profile requires int or Fraction probability, not %s" %
                            value.__class__.__name__)
        setitem_mbpr_action(deref(self.profile), index.action,
                            to_rational(str(value).encode('ascii')))

    def _payoff(self, player: Player) -> Rational:
        return rat_to_py(deref(self.profile).GetPayoff(player.player.deref().GetNumber()))

    def _belief(self, node: Node) -> Rational:
        return rat_to_py(deref(self.profile).GetBeliefProb(node.node))

    def _infoset_prob(self, infoset: Infoset) -> Rational:
        return rat_to_py(deref(self.profile).GetRealizProb(infoset.infoset))

    def _infoset_value(self, infoset: Infoset) -> Rational:
        return rat_to_py(deref(self.profile).GetPayoff(infoset.infoset))

    def _action_value(self, action: Action) -> Rational:
        return rat_to_py(deref(self.profile).GetPayoff(action.action))

    def _regret(self, action: Action) -> Rational:
        return rat_to_py(deref(self.profile).GetRegret(action.action))
    
    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, MixedBehaviorProfileRational) and
            deref(self.profile) == deref(cython.cast(MixedBehaviorProfileRational, other).profile)
        )

    def copy(self) -> MixedBehaviorProfileRational:
        """Creates a copy of the behavior strategy profile."""
        behav = MixedBehaviorProfileRational()
        behav.profile = make_shared[c_MixedBehaviorProfileRational](deref(self.profile))
        return behav

    def as_strategy(self) -> MixedStrategyProfileRational:
        """Returns a `MixedStrategyProfile` which is equivalent
        to the profile.
        """
        mixed = MixedStrategyProfileRational()
        mixed.profile = make_shared[c_MixedStrategyProfileRational](deref(self.profile).ToMixedProfile())
        return mixed

    def liap_value(self) -> Rational:
        """Returns the Lyapunov value (see [McK91]_) of the strategy profile.  The
        Lyapunov value is a non-negative number which is zero exactly at
        Nash equilibria.
        """
        return rat_to_py(deref(self.profile).GetLiapValue())

    def normalize(self) -> MixedBehaviorProfileRational:
        """Create a profile with the same action proportions as this
        one, but normalised so probabilites for each infoset sum to one.
        """
        profile = MixedBehaviorProfileRational()
        profile.profile = make_shared[c_MixedBehaviorProfileRational](deref(self.profile).Normalize())
        return profile

    def randomize(self, denom) -> None:
        """Randomizes the probabilities in the profile.  These are
        generated as uniform distributions over the actions at each
        information set.
        """
        deref(self.profile).Randomize(denom)

    @property
    def game(self) -> Game:
        """The game on which this mixed behaviour profile is defined.
        """
        g = Game()
        g.game = deref(self.profile).GetGame()
        return g
