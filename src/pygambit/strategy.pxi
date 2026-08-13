#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/strategy.pxi
# Cython wrapper for strategies
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
class Strategy:
    """A plan of action for a ``Player`` in a ``Game``."""
    strategy = cython.declare(c_GameStrategy)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a Strategy outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(strategy: c_GameStrategy) -> Strategy:
        obj: Strategy = Strategy.__new__(Strategy)
        obj.strategy = strategy
        return obj

    def __repr__(self) -> str:
        if self.label:
            return f"Strategy(player={self.player}, label='{self.label}')"
        else:
            return f"Strategy(player={self.player}, number={self.number})"

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, Strategy) and
            self.strategy.deref() == cython.cast(Strategy, other).strategy.deref()
        )

    def __hash__(self) -> int:
        return cython.cast(cython.long, self.strategy.deref())

    @property
    def label(self) -> str:
        """Get or set the text label associated with the strategy.

        .. versionchanged:: 16.7.0
            A strategy label must be nonempty and unique among the player's strategies;
            an empty or duplicate label now raises ``ValueError``.

        .. versionchanged:: 17.0.0
            A label may now be any well-formed UTF-8 text, not just ASCII; it must still
            contain no control characters, and must not begin/end with whitespace or have
            two consecutive whitespace characters.  "Whitespace" means any Unicode space
            separator (e.g. U+00A0 NO-BREAK SPACE), not just the ASCII space.
        """
        return self.strategy.deref().GetLabel().decode("utf-8")

    @label.setter
    def label(self, value: str) -> None:
        self.strategy.deref().SetLabel(value.encode("utf-8"))

    @property
    def game(self) -> Game:
        """The game to which the strategy belongs."""
        return Game.wrap(self.strategy.deref().GetPlayer().deref().GetGame())

    @property
    def player(self) -> Player:
        """The player to which the strategy belongs."""
        return Player.wrap(self.strategy.deref().GetPlayer())

    @property
    def number(self) -> int:
        """The number of the strategy."""
        return self.strategy.deref().GetNumber() - 1

    def action(self, infoset: Infoset | str) -> Action | None:
        """Get the action prescribed by a strategy for a given information set.

        .. versionadded:: 16.4.0

        Parameters
        ----------
        infoset
            The information set for which to find the prescribed action.
            Can be an Infoset object or its string label.

        Returns
        -------
        Action or None
            The prescribed action or None if the strategy is not defined for this
            information set, that is, the information set is unreachable under this strategy.

        Raises
        ------
        UndefinedOperationError
            If the game is not an extensive-form (tree) game.
        ValueError
            If the information set belongs to a different player than the strategy.

        See Also
        --------
        Game.get_behavior :
            A map-like view of the strategy's full mapping from information sets to actions.
        """
        if not self.game.is_tree:
            raise UndefinedOperationError(
                "Strategy.action is only defined for strategies in extensive-form games."
            )

        resolved_infoset: Infoset = self.game._resolve_infoset(infoset, "Strategy.action")

        if resolved_infoset.player != self.player:
            raise ValueError(
                f"Information set {resolved_infoset} belongs to player "
                f"'{resolved_infoset.player.label}', but this strategy "
                f"belongs to player '{self.player.label}'."
            )

        action: c_GameAction = self.strategy.deref().GetAction(resolved_infoset.infoset)
        if not action:
            return None
        return Action.wrap(action)


@cython.cclass
class StrategyBehavior:
    """A read-only, map-like view of the actions prescribed by a reduced strategy.

    The keys of the mapping are the information sets of the strategy's player at
    which the strategy prescribes an action; an unreachable information set is not a key.
    The corresponding values are the prescribed ``Action`` objects.
    Iteration yields the keys in the player's information set order.

    .. versionadded:: 17.0.0
    """
    _player = cython.declare(Player)
    _strategy = cython.declare(Strategy)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a StrategyBehavior outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(player: Player, strategy: Strategy) -> StrategyBehavior:
        obj: StrategyBehavior = StrategyBehavior.__new__(StrategyBehavior)
        obj._player = player
        obj._strategy = strategy
        return obj

    def __repr__(self) -> str:
        return f"StrategyBehavior(player={self._player}, strategy={self._strategy})"

    @property
    def player(self) -> Player:
        """The player to which the strategy belongs."""
        return self._player

    @property
    def strategy(self) -> Strategy:
        """The strategy of which this is the behavior."""
        return self._strategy

    def _resolve_key(self, key: Infoset | str) -> Infoset:
        """Resolve `key` to an information set at which the player has the move."""
        infoset = self._player.game._resolve_infoset(key, "StrategyBehavior", "key")
        if infoset.player != self._player:
            raise ValueError(
                f"Player '{self._player.label}' does not have the move at {infoset}."
            )
        return infoset

    def __getitem__(self, key: Infoset | str) -> Action:
        """Return the action prescribed at the information set referenced by `key`.

        Raises
        ------
        KeyError
            If the strategy prescribes no action at the information set,
            or if `key` is a string and no information set has that label.
        ValueError
            If the information set belongs to a different player.
        """
        infoset = self._resolve_key(key)
        action = self._strategy.action(infoset)
        if action is None:
            raise KeyError(
                f"Strategy '{self._strategy.label}' prescribes no action at {infoset}."
            )
        return action

    def get(self, key: Infoset | str, default: typing.Any = None) -> Action | None:
        """Return the action prescribed at `key`, or `default` if none is prescribed."""
        infoset = self._resolve_key(key)
        action = self._strategy.action(infoset)
        return default if action is None else action

    def __contains__(self, key: typing.Any) -> bool:
        try:
            infoset = self._resolve_key(key)
        except (KeyError, ValueError, TypeError):
            return False
        return self._strategy.action(infoset) is not None

    def __iter__(self) -> typing.Iterator[Infoset]:
        for infoset in self._player.infosets:
            if self._strategy.action(infoset) is not None:
                yield infoset

    def __len__(self) -> int:
        return sum(1 for _ in self)

    def keys(self) -> list[Infoset]:
        """The information sets at which the strategy prescribes an action."""
        return list(self)

    def values(self) -> list[Action]:
        """The prescribed actions, in the order of `keys`."""
        return [self._strategy.action(infoset) for infoset in self]

    def items(self) -> list[tuple[Infoset, Action]]:
        """(information set, action) pairs, in the order of `keys`."""
        return [(infoset, self._strategy.action(infoset)) for infoset in self]


@cython.cclass
class Sequence:
    """A sequence ``Player`` in a ``Game``.

    .. versionadded:: 16.7.0
    """
    sequence = cython.declare(c_GameSequence)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a Sequence outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(sequence: c_GameSequence) -> Sequence:
        obj: Sequence = Sequence.__new__(Sequence)
        obj.sequence = sequence
        return obj

    def __repr__(self) -> str:
        return f"Sequence(player={self.player}, actions={self.actions})"

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, Sequence) and
            self.sequence.deref() == cython.cast(Sequence, other).sequence.deref()
        )

    def __hash__(self) -> int:
        return cython.cast(cython.long, self.sequence.deref())

    @property
    def game(self) -> Game:
        """The game to which the sequence belongs."""
        return Game.wrap(self.sequence.deref().GetPlayer().deref().GetGame())

    @property
    def player(self) -> Player:
        """The player to which the sequence belongs."""
        return Player.wrap(self.sequence.deref().GetPlayer())

    @property
    def parent(self) -> Sequence | None:
        """The parent (predecessor) of the sequence."""
        if self.sequence.deref().GetParent() == cython.cast(c_GameSequence, NULL):
            return None
        return Sequence.wrap(self.sequence.deref().GetParent())

    @property
    def children(self) -> list[Sequence]:
        """The immediate children (successors) of the sequence."""
        ret: list[Sequence] = []
        for seq in self.player.sequences:
            if seq.parent == self:
                ret.append(seq)
        return ret

    @property
    def actions(self) -> list[Action]:
        """Get the collection of actions defining this sequence.

        Returns the empty list for the root sequence of the player.
        """
        actions: list[Action] = []
        seq = self.sequence
        while seq.deref().GetAction() != cython.cast(c_GameAction, NULL):
            actions.insert(0, Action.wrap(seq.deref().GetAction()))
            seq = seq.deref().GetParent()
        return actions
