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
class StrategyBehavior:
    """A read-only, map-like view of the actions prescribed by a reduced strategy.

    The keys of the mapping are the information sets of the strategy's player at
    which the strategy prescribes an action; an unreachable information set is not a key.
    The corresponding values are the prescribed ``Action`` objects.
    Iteration yields the keys in the player's information set order.

    .. versionadded:: 17.0.0
    """
    _game = cython.declare(Game)
    _player_label = cython.declare(str)
    _strategy_label = cython.declare(str)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a StrategyBehavior outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(game: Game, player_label: str, strategy_label: str) -> StrategyBehavior:
        obj: StrategyBehavior = StrategyBehavior.__new__(StrategyBehavior)
        obj._game = game
        obj._player_label = player_label
        obj._strategy_label = strategy_label
        return obj

    def __repr__(self) -> str:
        return (
            f"StrategyBehavior(player='{self._player_label}', "
            f"strategy='{self._strategy_label}')"
        )

    @property
    def player(self) -> str:
        """The label of the player to which the strategy belongs."""
        return self._player_label

    @property
    def strategy(self) -> str:
        """The label of the strategy of which this is the behavior."""
        return self._strategy_label

    def _action_at(self, infoset: Infoset) -> Action | None:
        """The action prescribed by the strategy at `infoset`, or None if unreachable."""
        player = cython.cast(Player, self._game.players[self._player_label])
        handle = self._game._resolve_strategy(
            player, self._strategy_label, "StrategyBehavior"
        )
        action: c_GameAction = handle.deref().GetAction(cython.cast(Infoset, infoset).infoset)
        if not action:
            return None
        return Action.wrap(action)

    def _resolve_key(self, key: Infoset | str) -> Infoset:
        """Resolve `key` to an information set at which the player has the move."""
        infoset = self._game._resolve_infoset(key, "StrategyBehavior", "key")
        if infoset.player.label != self._player_label:
            raise ValueError(
                f"Player '{self._player_label}' does not have the move at {infoset}."
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
        action = self._action_at(infoset)
        if action is None:
            raise KeyError(
                f"Strategy '{self._strategy_label}' prescribes no action at {infoset}."
            )
        return action

    def get(self, key: Infoset | str, default: typing.Any = None) -> Action | None:
        """Return the action prescribed at `key`, or `default` if none is prescribed."""
        infoset = self._resolve_key(key)
        action = self._action_at(infoset)
        return default if action is None else action

    def __contains__(self, key: typing.Any) -> bool:
        try:
            infoset = self._resolve_key(key)
        except (KeyError, ValueError, TypeError):
            return False
        return self._action_at(infoset) is not None

    def __iter__(self) -> typing.Iterator[Infoset]:
        player = self._game.players[self._player_label]
        for infoset in player.infosets:
            if self._action_at(infoset) is not None:
                yield infoset

    def __len__(self) -> int:
        return sum(1 for _ in self)

    def keys(self) -> list[Infoset]:
        """The information sets at which the strategy prescribes an action."""
        return list(self)

    def values(self) -> list[Action]:
        """The prescribed actions, in the order of `keys`."""
        return [self._action_at(infoset) for infoset in self]

    def items(self) -> list[tuple[Infoset, Action]]:
        """(information set, action) pairs, in the order of `keys`."""
        return [(infoset, self._action_at(infoset)) for infoset in self]


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
