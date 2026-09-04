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
class ActionSupport(_LabelSet):
    """A set of actions at a specified information set in a `BehaviorSupportProfile`.

    An immutable snapshot taken from a ``BehaviorSupportProfile`` at retrieval time: it
    does not reflect later changes to the profile. The information set is accessible
    via `infoset`.
    """
    @staticmethod
    @cython.cfunc
    def wrap(infoset: Infoset, actions: tuple) -> ActionSupport:
        obj: ActionSupport = ActionSupport.__new__(ActionSupport)
        obj._owner = infoset
        obj._labels = actions
        return obj

    @property
    def infoset(self) -> Infoset:
        return self._owner


@cython.cclass
class BehaviorSupport:
    """The action supports at the information sets belonging to a specified player in
    a `BehaviorSupportProfile`.

    An immutable snapshot taken from a ``BehaviorSupportProfile`` at retrieval time: it
    does not reflect later changes to the profile. The player is accessible via
    `player`.
    """
    _player = cython.declare(str)
    _values = cython.declare(dict)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a BehaviorSupport outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(player: str, values: dict) -> BehaviorSupport:
        obj: BehaviorSupport = BehaviorSupport.__new__(BehaviorSupport)
        obj._player = player
        obj._values = values
        return obj

    @property
    def player(self) -> str:
        """The label of the player for whom this behavior support is defined."""
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
        support : ActionSupport
            The support at an information set belonging to the player
        """
        yield from self._values.values()

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
        if infoset.player != self._player:
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
            yield self[player]

    def __getitem__(self, index: typing.Any) -> BehaviorSupport | ActionSupport:
        """Access a component of the support profile specified by `index`.

        Parameters
        ----------
        index : str or Selector
            The part of the profile to return:

            * If `index` is a ``str``, returns a ``BehaviorSupport`` over the player's
              information sets. The player is determined by finding the player with
              that label, if any.
            * If `index` is a ``Selector`` (an `H`-built expression) resolving to a
              single node, returns an ``ActionSupport`` over the actions in the
              support at that node's information set.

        Raises
        ------
        TypeError
            If `index` is not a ``str`` or a ``Selector``.
        ValueError
            If `index` is a ``Selector`` resolving to a terminal node, which belongs
            to no information set, or to a chance event.
        KeyError
            If `index` is a ``str`` and no player in the game has that label.
        """
        if isinstance(index, str):
            values = {
                node.infoset: self._action_support_at(node.infoset)
                for node in self.game.get_infosets(index)
            }
            return BehaviorSupport.wrap(index, values)
        if isinstance(index, Selector):
            resolved_infoset = self.game._resolve_infoset(
                index, "BehaviorSupportProfile.__getitem__"
            )
            return self._action_support_at(resolved_infoset)
        raise TypeError(
            f"profile index must be str or Selector, not {index.__class__.__name__}"
        )

    def _action_support_at(self, infoset: Infoset) -> ActionSupport:
        """Returns a snapshot of the action support at infoset, as of now."""
        infoset_handle = cython.cast(Infoset, infoset)._resolve()
        actions = tuple(
            a.deref().GetLabel().decode("utf-8")
            for a in deref(self.profile).GetActions(infoset_handle)
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
        labels = set(infoset.actions)
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
        action_handles = cython.cast(Infoset, infoset)._resolve().deref().GetActions()
        for a in action_handles:
            if a.deref().GetLabel().decode("utf-8") in given:
                deref(self.profile).AddAction(a)
        for a in action_handles:
            if a.deref().GetLabel().decode("utf-8") not in given:
                deref(self.profile).RemoveAction(a)

    def __setitem__(self, infoset: typing.Any, actions: typing.Iterable[str]) -> None:
        """Sets the support at `infoset` to exactly the given actions.

        Parameters
        ----------
        infoset : Selector
            An `H`-built expression resolving to a single node belonging to the
            information set whose support is to be set.
        actions : Iterable[str]
            The labels of the actions which should be in the support at the
            information set. Every other action at the information set is removed
            from the support.

        Raises
        ------
        TypeError
            If `infoset` is not a ``Selector``.
        ValueError
            If any entry of `actions` is not one of the information set's action
            labels, or if `actions` is empty; or if `infoset` resolves to a
            terminal node, which belongs to no information set, or to a chance
            event.
        """
        if not isinstance(infoset, Selector):
            raise TypeError(
                f"profile index must be Selector, not {infoset.__class__.__name__}"
            )
        resolved_infoset = self.game._resolve_infoset(
            infoset, "BehaviorSupportProfile.__setitem__"
        )
        self._set_support(resolved_infoset, actions)

    def copy(self) -> BehaviorSupportProfile:
        """Creates a copy of the support profile.

        The copy shares its underlying data with the original until one of them is
        next assigned into, at which point the one being assigned into transparently
        takes its own private copy first. Both profiles are fully independent from
        each other's perspective; this only affects when the underlying duplication
        happens, not whether it happens.
        """
        return BehaviorSupportProfile.wrap(self.profile)

    def is_infoset_reachable(self, infoset: Selector) -> bool:
        """Returns whether `infoset` can be reached under this support, i.e. whether
        there is some path of play consistent with the support that reaches it.

        Parameters
        ----------
        infoset : Selector
            An `H`-built expression resolving to a single node belonging to the
            information set to check.

        Raises
        ------
        TypeError
            If `infoset` is not a ``Selector``.
        ValueError
            If `infoset` resolves to a terminal node, which belongs to no
            information set, or to a chance event.
        """
        if not isinstance(infoset, Selector):
            raise TypeError(
                f"is_infoset_reachable(): infoset must be a Selector, "
                f"not {infoset.__class__.__name__}"
            )
        resolved_infoset: Infoset = self.game._resolve_infoset(infoset, "is_infoset_reachable")
        return deref(self.profile).IsReachable(resolved_infoset._resolve())
