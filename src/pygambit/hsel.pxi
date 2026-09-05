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


class _PathStep:
    """One `H.path(*steps)` operation: each step is an exact action label or
    the wildcard `...`.  Root-anchored if it is the first op in a Selector,
    "this many more steps from here" otherwise -- the evaluator doesn't need
    to distinguish the two cases, since they're the same operation applied to
    whatever's already been selected (the root, for a bare seed)."""

    def __init__(self, steps: tuple) -> None:
        self.steps = steps

    def __repr__(self) -> str:
        return f"_PathStep(steps={self.steps!r})"


class _PlaysStep:
    """One `.plays` operation: expand to the current terminal frontier."""

    def __repr__(self) -> str:
        return "_PlaysStep()"


class _AfterStep:
    """One `.after(*labels)` operation: an unconstrained (possibly empty)
    prefix, then exactly these trailing labels. As the first op in a
    Selector, matches anywhere in the whole game, not just root's frontier --
    the natural counterpart to `.path(...)`'s root anchoring. Chained onto an
    existing selection, it's a pure filter: no new nodes are considered, just
    whichever already-selected ones end in this suffix."""

    def __init__(self, labels: tuple) -> None:
        self.labels = labels

    def __repr__(self) -> str:
        return f"_AfterStep(labels={self.labels!r})"


def _matches_suffix(node: Node, labels: tuple) -> bool:
    """Whether `node`'s own history ends with exactly `labels`."""
    current: Node = node
    for label in reversed(labels):
        parent = current._parent()
        if parent is None or current._prior_action().label != label:
            return False
        current = parent
    return True


class _FilterStep:
    """One `.filter(callable)` operation: keep only elements where
    `predicate`, given a HistoryView, returns something truthy. Chained-only
    -- unlike `.after(...)`, there's no natural "whole game" domain for a
    bare predicate to start from, so it's not exposed as an `H.filter(...)`
    seed."""

    def __init__(self, predicate: typing.Callable) -> None:
        self.predicate = predicate

    def __repr__(self) -> str:
        return f"_FilterStep(predicate={self.predicate!r})"


class Selector:
    """A game-neutral description of a set of nodes.  Carries no reference to
    any game -- it's just a recipe, evaluated only when handed to a `Game`
    method that accepts one, such as `append_move` or `make_outcome`.

    .. versionadded:: 17.0.0
    """

    def __init__(self, ops: tuple = ()) -> None:
        self._ops = ops

    def __repr__(self) -> str:
        return f"Selector(ops={self._ops!r})"

    def _extend(self, op) -> Selector:
        return Selector(self._ops + (op,))

    def path(self, *steps: str) -> Selector:
        """`N` more steps from wherever this selection currently is.  Each
        step is an exact action label, or `...` to match any single action.
        """
        return self._extend(_PathStep(steps))

    @property
    def plays(self) -> Selector:
        """The current terminal frontier of this selection -- not
        necessarily one step forward, whatever is currently terminal beneath
        each already-selected node."""
        return self._extend(_PlaysStep())

    def after(self, *labels: str) -> Selector:
        """Filter this selection to just the elements whose own trailing
        labels are exactly `labels`, whatever came before them."""
        return self._extend(_AfterStep(labels))

    def filter(self, predicate: typing.Callable) -> Selector:
        """Keep only the elements of this selection where `predicate`,
        called once per element with a read-only `HistoryView` of it,
        returns something truthy. The general escape hatch for a filter
        `.after(...)`'s label-pattern matching can't express -- e.g.
        anything needing `.last_action(player)` rather than a plain
        trailing-label match."""
        return self._extend(_FilterStep(predicate))

    def by(self, key: typing.Callable) -> GroupedSelector:
        """Partition this selection by `key`, called once per element with a
        read-only `HistoryView` of it. Distinct return values become distinct
        groups; game-neutral until evaluated, same as `Selector` itself."""
        return GroupedSelector(self, key)


class GroupedSelector:
    """Result of `.by(callable)`.  Game-neutral until evaluated -- pass to a
    `Game` method that accepts a `GroupedSelector`, such as `append_move`,
    which dispatches one call per group.

    `.plays`/`.after(...)` chain onto a `GroupedSelector` the same way they
    chain onto a plain `Selector`, but apply per-group: each group's own
    members are expanded/filtered independently, and the group's key is left
    untouched -- expanding past a decision point doesn't retroactively change
    what a group was keyed by. `.with_recall(player)` is the one exception:
    once set, every subsequent `.plays` on this selector *also* refines each
    group's key by folding in `player`'s last action at that point, so a
    partition built for one decision stays a valid recall-respecting
    partition when reused for a later one, without the caller needing to
    re-derive or manually re-key it. Scoped to `.plays` specifically for now
    (not every expand-style op) -- narrower than the full "any expand-style
    step" idea from the design notes, not yet stress-tested against a shape
    that would need more.

    .. versionadded:: 17.0.0
    """

    def __init__(
        self,
        base: Selector,
        key: typing.Callable,
        post_ops: tuple = (),
        recall_player: str = None,
    ) -> None:
        self.base = base
        self.key = key
        self.post_ops = post_ops
        self.recall_player = recall_player

    def __repr__(self) -> str:
        return (
            f"GroupedSelector(base={self.base!r}, key={self.key!r}, "
            f"post_ops={self.post_ops!r}, recall_player={self.recall_player!r})"
        )

    def _extend(self, op) -> GroupedSelector:
        return GroupedSelector(self.base, self.key, self.post_ops + (op,), self.recall_player)

    @property
    def plays(self) -> GroupedSelector:
        """The current terminal frontier of each group, independently --
        see the class docstring for how this interacts with
        `.with_recall(player)`."""
        return self._extend(_PlaysStep())

    def after(self, *labels: str) -> GroupedSelector:
        """Filter each group to just the members whose own trailing labels
        are exactly `labels`, whatever came before them."""
        return self._extend(_AfterStep(labels))

    def with_recall(self, player: str) -> GroupedSelector:
        """From here on, every `.plays` on this selector also refines each
        group's key by folding in `player`'s last action at that point --
        see the class docstring."""
        return GroupedSelector(self.base, self.key, self.post_ops, player)


def _history_of(node: Node) -> tuple:
    """The plain-tuple History for `node` -- walks back to the root via the
    private `Node._parent`/`._prior_action` navigation."""
    labels: list = []
    current: Node = node
    while current._parent() is not None:
        labels.append(current._prior_action().label)
        current = current._parent()
    labels.reverse()
    return tuple(labels)


def _canonical_history(node: Node) -> tuple:
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
    resolved: c_GameInfoset = node._infoset_handle()
    if resolved == cython.cast(c_GameInfoset, NULL):
        raise AttributeError("node currently belongs to no information set or event")
    return _history_of(Node.wrap(resolved.deref().GetMember(1)))


class HistoryView:
    """The object a `.filter(callable)`/`.by(callable)` predicate, or
    `Game.behavior_support_profile`'s `actions` callback, actually receives.
    Supports plain sequence indexing/slicing like a `History` tuple, plus
    limited game-aware navigation (`.last_action(player)`) -- but never
    exposes the `Node`/game it's privately backed by.  Not constructible
    directly.

    .. versionadded:: 17.0.0
    """

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a HistoryView directly.")

    @staticmethod
    def _wrap(node: Node, history: tuple) -> HistoryView:
        obj: HistoryView = HistoryView.__new__(HistoryView)
        obj._node = node
        obj._history = history
        return obj

    def __repr__(self) -> str:
        return f"HistoryView({self._history!r})"

    def __len__(self) -> int:
        return len(self._history)

    def __getitem__(self, index: typing.Any) -> typing.Any:
        return self._history[index]

    def last_action(self, player: str) -> str | None:
        """The label of the last action `player` took on the path to this
        history, wherever it fell -- `None` if `player` hasn't acted yet."""
        return _last_action(self._node, player)

    @property
    def members(self) -> list[tuple]:
        """The Histories of the nodes which are members of the information set or
        event to which this history currently belongs -- whichever applies.

        Raises
        ------
        AttributeError
            If this history currently belongs to no information set or event (a
            terminal node).
        """
        return [_history_of(member) for member in self._node.members]


def _last_action(node: Node, player: str) -> str | None:
    """The label of the last action `player` took on the path to `node`,
    wherever it fell -- `None` if `player` hasn't acted yet. Shared between
    `HistoryView.last_action` and `.with_recall(player)`'s evaluation."""
    current: Node = node
    while current._parent() is not None:
        if current._parent().player == player:
            return current._prior_action().label
        current = current._parent()
    return None


class H:
    """Namespace of seed constructors for the node-selector algebra.  Not
    meant to be instantiated -- use as `H.path(...)`, conventionally imported
    as `import pygambit.H as H`.

    .. versionadded:: 17.0.0
    """

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("H is a namespace of selector constructors, not instantiable.")

    @staticmethod
    def path(*steps: str) -> Selector:
        """A root-anchored selection.  Each step is an exact action label, or
        `...` to match any single action.  `H.path()` with no steps selects
        the root itself.
        """
        return Selector().path(*steps)

    @staticmethod
    def after(*labels: str) -> Selector:
        """Anywhere in the whole game whose own trailing labels are exactly
        `labels`, whatever came before them -- the suffix-anchored
        counterpart to the root-anchored `.path(...)`.
        """
        return Selector().after(*labels)

    plays: Selector = Selector((_PlaysStep(),))
    """All currently-terminal nodes in the whole game."""
