#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/hsel.pxi
# First sketch of the H selector algebra: game-neutral expressions built by
# pygambit.H, evaluated only when handed to a Game.  Deliberately minimal --
# just enough operations to validate the architecture, not the full roster.
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


def _matches_suffix(node: Node, labels: tuple) -> bool:
    """Whether `node`'s own history ends with exactly `labels`."""
    current: Node = node
    for label in reversed(labels):
        parent = current._parent()
        if parent is None or current._prior_action().label != label:
            return False
        current = parent
    return True


def _history_of(node: Node) -> History:
    """The History for `node` -- walks back to the root via the private
    `Node._parent`/`._prior_action` navigation, recording a `HistoryTransition`
    per step."""
    return _history_of_cached(node, {})


@cython.cfunc
def _history_of_cached(node: Node, cache: dict) -> History:
    """Internal: `_history_of`, memoized on `cache` (keyed by `Node`) for the
    duration of one top-level call. Without this, each `HistoryTransition.state`
    triggers a fresh `_canonical_history`/`_history_of` walk of its own, and since
    that walk repeats for every ancestor, the total cost is exponential in tree
    depth rather than linear."""
    if node in cache:
        return cache[node]
    transitions: list = []
    current: Node = node
    while current._parent() is not None:
        branch = current._prior_action()
        transitions.append(
            HistoryTransition(
                player=branch.node.player,
                state=_canonical_history_cached(branch.node, cache),
                action=branch.label,
            )
        )
        current = branch.node
    transitions.reverse()
    result: History = History(transitions)
    cache[node] = result
    return result


def _canonical_history(node: Node) -> History:
    """The History of the canonical member of `node`'s current information set or
    event: the first member encountered in the pre-order depth-first traversal of the
    game tree (``GetMember(1)``), matching the order `Node.members` itself uses. Used
    as the stable key identifying an information set or event, in place of any one
    particular member node.

    Raises
    ------
    AttributeError
        If `node` currently belongs to no information set or event (a terminal node).
    """
    return _canonical_history_cached(node, {})


@cython.cfunc
def _canonical_history_cached(node: Node, cache: dict) -> History:
    """Internal: `_canonical_history`, threading the same `cache` used by
    `_history_of_cached` through to its own `_history_of_cached` call."""
    resolved: c_GameInfoset = node._infoset_handle()
    if resolved == cython.cast(c_GameInfoset, NULL):
        raise AttributeError("node currently belongs to no information set or event")
    return _history_of_cached(Node.wrap(resolved.deref().GetMember(1)), cache)


@cython.cfunc
def _node_last_acted_child(node: c_GameNode, player: str) -> c_GameNode:
    """Internal: the child (a raw handle, not a `Node`) reached by `player`'s
    last action on the path to `node` -- i.e. the node whose parent is where
    `player` acted -- or a null handle if `player` hasn't acted yet. Shared
    by `_node_last_action` (the action's label) and `_node_last_action_key`
    (the `(infoset, label)` pair `.by_last_action(player)` keys on).

    Raises
    ------
    KeyError
        If no player in the game has label `player`.
    TypeError
        If `player` is not a `str`.
    ValueError
        If `player` is an empty string or all spaces.
    """
    game: Game = Game._wrap(node.deref().GetGame())
    game._resolve_player(player, "last_action")
    current: c_GameNode = node
    parent: c_GameNode = current.deref().GetParent()
    while parent != cython.cast(c_GameNode, NULL):
        player_handle: c_GamePlayer = parent.deref().GetPlayer()
        if (
            player_handle != cython.cast(c_GamePlayer, NULL) and
            player_handle.deref().GetLabel().decode("utf-8") == player
        ):
            return current
        current = parent
        parent = current.deref().GetParent()
    return cython.cast(c_GameNode, NULL)


@cython.cfunc
def _node_last_action(node: c_GameNode, player: str) -> object:
    """The label of the last action `player` took on the path to `node`
    (a raw handle, not a `Node`), wherever it fell -- `None` if `player`
    hasn't acted yet. The `c_GameNode`-only core of `_last_action`, used
    directly by `HistoryView`, which must never hold a `Node`.

    Raises
    ------
    KeyError
        If no player in the game has label `player`.
    TypeError
        If `player` is not a `str`.
    ValueError
        If `player` is an empty string or all spaces.
    """
    child: c_GameNode = _node_last_acted_child(node, player)
    if child != cython.cast(c_GameNode, NULL):
        return child.deref().GetPriorAction().deref().GetLabel().decode("utf-8")
    return None


@cython.cfunc
def _node_last_action_key(node: c_GameNode, player: str) -> object:
    """The `(infoset, label)` pair identifying `player`'s last action on the
    path to `node` (a raw handle, not a `Node`), wherever it fell -- `None`
    if `player` hasn't acted yet. `infoset` is the canonical `History` of the
    infoset at which the action was taken (see `_canonical_history`,
    matching `HistoryTransition.state`), so two infosets that happen to
    share an action label are still told apart. Used by
    `.by_last_action(player)`'s evaluation, via `_last_action_key`.

    Raises
    ------
    KeyError
        If no player in the game has label `player`.
    TypeError
        If `player` is not a `str`.
    ValueError
        If `player` is an empty string or all spaces.
    """
    child: c_GameNode = _node_last_acted_child(node, player)
    if child != cython.cast(c_GameNode, NULL):
        return (
            _canonical_history(Node.wrap(child.deref().GetParent())),
            child.deref().GetPriorAction().deref().GetLabel().decode("utf-8"),
        )
    return None


def _last_action_key(node: Node, player: str) -> tuple | None:
    """The `(infoset, label)` pair identifying `player`'s last action on the
    path to `node`, wherever it fell -- `None` if `player` hasn't acted yet.
    Used by `.by_last_action(player)`'s evaluation, which still works in
    terms of `Node`."""
    return _node_last_action_key(cython.cast(Node, node).node, player)


@cython.cclass
class HistoryView:
    """The object a `.filter(callable)`/`.by(callable)` predicate, or
    `Game.behavior_support_profile`'s `actions` callback, actually receives.
    Supports indexing/slicing like a `History` -- so each element is a
    `HistoryTransition`, not a bare action label -- plus `.actions` (the
    plain label-tuple projection) and limited game-aware navigation
    (`.last_action(player)`) -- but never exposes the `Node`/game it's
    privately backed by.  Not constructible directly.

    A Cython extension type holding a raw `c_GameNode` handle, not a `Node`
    object: `_node`/`_history` are C-level fields, not `__dict__` entries, so
    a predicate/callback cannot read them back out via `view._node`; and
    since `c_GameNode` is a C++ value with no Python identity, there is no
    Python object here for `gc.get_referents()` to find either, unlike a
    cclass holding a `Node` (or any other Python-object field), which Cython
    would still make reachable that way regardless of attribute access.

    .. versionadded:: 17.0.0
    """
    _node = cython.declare(c_GameNode)
    _history = cython.declare(object)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a HistoryView directly.")

    @staticmethod
    @cython.cfunc
    def _wrap(node: Node, history: History) -> HistoryView:
        obj: HistoryView = HistoryView.__new__(HistoryView)
        obj._node = cython.cast(Node, node).node
        obj._history = history
        return obj

    def __repr__(self) -> str:
        return f"HistoryView({self._history!r})"

    def __len__(self) -> int:
        return len(self._history)

    def __getitem__(self, index: typing.Any) -> typing.Any:
        return self._history[index]

    @property
    def actions(self) -> tuple[str, ...]:
        """The plain tuple of action labels, dropping the player/state context
        each step also carries."""
        return self._history.actions

    def last_action(self, player: str) -> str | None:
        """The label of the last action `player` took on the path to this
        history, wherever it fell -- `None` if `player` hasn't acted yet.

        Raises
        ------
        KeyError
            If no player in the game has label `player`.
        TypeError
            If `player` is not a `str`.
        ValueError
            If `player` is an empty string or all spaces.
        """
        return _node_last_action(self._node, player)

    @property
    def members(self) -> list[History]:
        """The Histories of the nodes which are members of the information set or
        event to which this history currently belongs -- whichever applies.

        Raises
        ------
        AttributeError
            If this history currently belongs to no information set or event (a
            terminal node).
        """
        infoset_handle: c_GameInfoset = self._node.deref().GetInfoset()
        if infoset_handle == cython.cast(c_GameInfoset, NULL):
            raise AttributeError("node currently belongs to no information set or event")
        return [_history_of(Node.wrap(member)) for member in infoset_handle.deref().GetMembers()]
