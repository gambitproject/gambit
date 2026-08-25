#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/behavspt.pxi
# Cython wrapper for behavior support profiles
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
class ActionSupport:
    """A set of actions at a specified information set in a `BehaviorSupportProfile`.

    An immutable snapshot taken from a ``BehaviorSupportProfile`` at retrieval time: it
    does not reflect later changes to the profile. The information set is accessible
    via `infoset`.
    """
    _infoset = cython.declare(Infoset)
    _actions = cython.declare(tuple)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create an ActionSupport outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(infoset: Infoset, actions: tuple) -> ActionSupport:
        obj: ActionSupport = ActionSupport.__new__(ActionSupport)
        obj._infoset = infoset
        obj._actions = actions
        return obj

    @property
    def infoset(self) -> Infoset:
        return self._infoset

    def __repr__(self) -> str:
        return str(list(self._actions))

    def __eq__(self, other: typing.Any) -> bool:
        if isinstance(other, (set, frozenset, list, tuple)):
            return set(self._actions) == set(other)
        if not isinstance(other, ActionSupport) or self.infoset != other.infoset:
            return False
        return set(self._actions) == set(cython.cast(ActionSupport, other)._actions)

    def __len__(self) -> int:
        return len(self._actions)

    def __iter__(self) -> typing.Generator[str, None, None]:
        yield from self._actions

    def __contains__(self, label: str) -> bool:
        return label in self._actions


@cython.cclass
class BehaviorSupport:
    """The action supports at the information sets belonging to a specified player in
    a `BehaviorSupportProfile`.

    An immutable snapshot taken from a ``BehaviorSupportProfile`` at retrieval time: it
    does not reflect later changes to the profile. The player is accessible via
    `player`.
    """
    _player = cython.declare(Player)
    _values = cython.declare(dict)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a BehaviorSupport outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(player: Player, values: dict) -> BehaviorSupport:
        obj: BehaviorSupport = BehaviorSupport.__new__(BehaviorSupport)
        obj._player = player
        obj._values = values
        return obj

    @property
    def player(self) -> Player:
        """The player for whom this behavior support is defined."""
        return self._player

    def __repr__(self) -> str:
        return str(list(self._values.values()))

    def __eq__(self, other: typing.Any) -> bool:
        values = list(self._values.values())
        if isinstance(other, list):
            return values == other
        if not isinstance(other, BehaviorSupport) or self.player != other.player:
            return False
        return values == list(cython.cast(BehaviorSupport, other)._values.values())

    def __len__(self) -> int:
        return len(self._values)

    def __iter__(self) -> typing.Generator[ActionSupport, None, None]:
        """Iterate over the action supports specified by the behavior support, one per
        information set belonging to the player.

        Yields
        ------
        infoset : Infoset
            An information set belonging to the player
        support : ActionSupport
            The support at the information set
        """
        for infoset in self.player.infosets:
            yield self[infoset]

    def __getitem__(self, infoset: Infoset) -> ActionSupport:
        """Returns the action support at `infoset`.

        Parameters
        ----------
        infoset : Infoset
            The information set to return the support for.

        Raises
        ------
        MismatchError
            If `infoset` does not belong to this player.
        """
        if infoset.player != self.player:
            raise MismatchError("infoset must belong to this player")
        return self._values[infoset]


@cython.cclass
class BehaviorSupportProfile:
    """A set-like object representing a subset of the actions in an extensive game.
    A BehaviorSupportProfile always contains at least one action for each information
    set belonging to a personal player in the game.
    """
    profile = cython.declare(shared_ptr[c_BehaviorSupportProfile])

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a BehaviorSupportProfile outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(profile: shared_ptr[c_BehaviorSupportProfile]) -> BehaviorSupportProfile:
        obj: BehaviorSupportProfile = BehaviorSupportProfile.__new__(BehaviorSupportProfile)
        obj.profile = profile
        return obj

    @property
    def game(self) -> Game:
        """The `Game` on which the support profile is defined."""
        return Game.wrap(deref(self.profile).GetGame())

    def __repr__(self) -> str:
        return f"BehaviorSupportProfile(game={self.game})"

    def __len__(self) -> int:
        """Returns the total number of actions in the support profile."""
        return deref(self.profile).BehaviorProfileLength()

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, BehaviorSupportProfile) and
            deref(self.profile) == deref(cython.cast(BehaviorSupportProfile, other).profile)
        )

    def __iter__(self) -> typing.Generator[BehaviorSupport, None, None]:
        """Iterate over the behavior supports in the profile, one per player.

        Yields
        ------
        support : BehaviorSupport
            The player's behavior support specified in the profile
        """
        for player in self.game.players:
            yield self[player.label]

    def __getitem__(self, index: typing.Any) -> BehaviorSupport | ActionSupport:
        """Access a component of the support profile specified by `index`.

        Parameters
        ----------
        index : str or Infoset
            The part of the profile to return:

            * If `index` is a ``str``, returns a ``BehaviorSupport`` over the player's
              information sets. The player is determined by finding the player with
              that label, if any.
            * If `index` is an ``Infoset``, returns an ``ActionSupport`` over the
              actions in the support at the information set.

        Raises
        ------
        TypeError
            If `index` is not a ``str`` or an ``Infoset``.
        MismatchError
            If `index` is an ``Infoset`` from a different game.
        KeyError
            If `index` is a ``str`` and no player in the game has that label.
        """
        if isinstance(index, Infoset):
            if index.game != self.game:
                raise MismatchError("infoset must be part of the same game")
            return self._action_support_at(index)
        if isinstance(index, str):
            resolved_player: Player = self.game.players[index]
            values = {
                infoset: self._action_support_at(infoset) for infoset in resolved_player.infosets
            }
            return BehaviorSupport.wrap(resolved_player, values)
        raise TypeError(
            f"profile index must be str or Infoset, not {index.__class__.__name__}"
        )

    def _action_support_at(self, infoset: Infoset) -> ActionSupport:
        """Returns a snapshot of the action support at infoset, as of now."""
        infoset_handle = cython.cast(Infoset, infoset).infoset
        actions = tuple(
            Action.wrap(a).label for a in deref(self.profile).GetActions(infoset_handle)
        )
        return ActionSupport.wrap(infoset, actions)

    @cython.cfunc
    def _ensure_unshared(self) -> cython.void:
        """Clones the underlying profile if it is shared with another wrapper, so that
        the mutation about to happen is not observed by any other BehaviorSupportProfile.
        """
        if self.profile.use_count() != 1:
            self.profile = make_shared[c_BehaviorSupportProfile](deref(self.profile))

    @cython.cfunc
    def _set_support(self, infoset: Infoset, actions: object) -> cython.void:
        """Validates and sets the whole support at infoset.

        Every entry of `actions` must be one of the information set's action labels, and
        at least one must be given.
        """
        labels = {a.label for a in infoset.actions}
        given = set(actions)
        unknown = given - labels
        if unknown:
            raise ValueError(
                f"not an action label at this information set: {', '.join(sorted(unknown))}"
            )
        if not given:
            raise ValueError("a support must contain at least one action at the information set")
        self._ensure_unshared()
        # Actions to keep are added first, so that a subsequent removal is never asked
        # to remove the last remaining action at the information set. (Unlike
        # RemoveStrategy, RemoveAction does not itself guard against emptying its scope.)
        for a in infoset.actions:
            if a.label in given:
                deref(self.profile).AddAction(cython.cast(Action, a).action)
        for a in infoset.actions:
            if a.label not in given:
                deref(self.profile).RemoveAction(cython.cast(Action, a).action)

    def __setitem__(self, infoset: typing.Any, actions: typing.Iterable[str]) -> None:
        """Sets the support at `infoset` to exactly the given actions.

        Parameters
        ----------
        infoset : Infoset
            The information set whose support is to be set.
        actions : Iterable[str]
            The labels of the actions which should be in the support at the
            information set. Every other action at the information set is removed
            from the support.

        Raises
        ------
        TypeError
            If `infoset` is not an ``Infoset``.
        MismatchError
            If `infoset` is an `Infoset` from a different game.
        ValueError
            If any entry of `actions` is not one of the information set's action
            labels, or if `actions` is empty.
        """
        if not isinstance(infoset, Infoset):
            raise TypeError(f"profile index must be Infoset, not {infoset.__class__.__name__}")
        if infoset.game != self.game:
            raise MismatchError("infoset must be part of the same game")
        self._set_support(infoset, actions)

    def copy(self) -> BehaviorSupportProfile:
        """Creates a copy of the support profile.

        The copy shares its underlying data with the original until one of them is
        next assigned into, at which point the one being assigned into transparently
        takes its own private copy first. Both profiles are fully independent from
        each other's perspective; this only affects when the underlying duplication
        happens, not whether it happens.
        """
        return BehaviorSupportProfile.wrap(self.profile)

    def is_reachable(self, infoset: InfosetReference) -> bool:
        """Returns whether `infoset` can be reached under this support, i.e. whether
        there is some path of play consistent with the support that reaches it.

        Parameters
        ----------
        infoset : Infoset or str
            The information set to check. If a string is passed, the information set
            is determined by finding the information set with that label, if any.

        Raises
        ------
        MismatchError
            If `infoset` is an `Infoset` from a different game.
        KeyError
            If `infoset` is a string and no information set in the game has that label.
        """
        resolved_infoset = self.game._resolve_infoset(infoset, "is_reachable")
        return deref(self.profile).IsReachable(cython.cast(Infoset, resolved_infoset).infoset)
