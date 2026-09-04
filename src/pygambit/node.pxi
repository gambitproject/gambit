#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/node.pxi
# Cython wrapper for nodes
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

Branch = collections.namedtuple("Branch", ["node", "label"])
Branch.__doc__ = """The action labeled `label`, taken at `node`.

Returned by `Node.prior_action` and `Node.own_prior_action`; `node` is the node at
which the action was taken (not the node it leads to), so ``branch.node.actions``
and, for a chance event, ``branch.node.action_probs[branch.label]`` are always
well-defined.

.. versionadded:: 17.0.0
"""


@cython.cfunc
def _decode_prob(py_string: string) -> object:
    """Internal: decode a probability formatted by the C++ core as ``Decimal`` or
    ``Rational``, matching whichever representation was used to specify it."""
    if "." in py_string.decode("ascii"):
        return decimal.Decimal(py_string.decode("ascii"))
    else:
        return Rational(py_string.decode("ascii"))


@cython.cclass
class NodeChildren:
    """The set of nodes which are direct descendants of a node."""
    parent = cython.declare(c_GameNode)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create NodeChildren outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(parent: c_GameNode) -> NodeChildren:
        obj: NodeChildren = NodeChildren.__new__(NodeChildren)
        obj.parent = parent
        return obj

    def __len__(self) -> int:
        return self.parent.deref().GetChildren().size()

    def __repr__(self) -> str:
        return f"NodeChildren(parent={Node.wrap(self.parent)})"

    def __iter__(self) -> typing.Iterator[Node]:
        for child in self.parent.deref().GetChildren():
            yield Node.wrap(child)

    def __getitem__(self, action: typing.Any) -> Node:
        """Returns the successor node which is reached after the action labeled
        `action` is played.

        Raises
        ------
        KeyError
            If no action with that label exists at the node's infoset, or if the
            node is terminal.
        ValueError
            If `action` is an empty or all-whitespace string.
        TypeError
            If `action` is not a ``str``.

        .. versionchanged:: 16.5.0
            Previously indexing by string searched the labels of the child nodes,
            rather than referring to actions.  This implements the more natural
            interpretation that strings refer to action labels.

        .. versionchanged:: 16.7.0
            Integer indexing is no longer supported; index by the action's label, or
            iterate.  A label matching no action now raises ``KeyError``.

        .. versionchanged:: 17.0.0
            No longer indexable by an ``Action`` object, following its removal.
        """
        if isinstance(action, str):
            if not action.strip():
                raise ValueError("Action label cannot be empty or all whitespace")
            if self.parent.deref().GetInfoset() == cython.cast(c_GameInfoset, NULL):
                raise KeyError(f"No action with label '{action}' at node")
            for act in self.parent.deref().GetInfoset().deref().GetActions():
                if act.deref().GetLabel().decode("utf-8") == cython.cast(str, action):
                    return Node.wrap(self.parent.deref().GetChild(act))
            raise KeyError(f"No action with label '{action}' at node")
        if isinstance(action, int):
            raise TypeError(
                "node children cannot be indexed by position; index by the action's "
                "label, or iterate. (Integer indexing was removed in 16.7.0.)"
            )
        raise TypeError(f"Index must be a str label, not {action.__class__.__name__}")


@cython.cclass
class NodeOutcome:
    """The outcome attached to a node.

    A lazy, node-anchored view: holds the node and resolves its outcome on each access,
    so the value reflects the current state of the game even after the game is mutated.

    .. versionadded:: 16.7.0

    .. versionchanged:: 17.0.0
        A node with no outcome attached resolves to the game's null outcome: the view is
        falsy, its ``label`` is ``None``, its payoffs read as zero, and it compares unequal
        to every outcome — including another null and itself — and to ``None``.
    """
    node = cython.declare(c_GameNode)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a NodeOutcome outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(node: c_GameNode) -> NodeOutcome:
        obj: NodeOutcome = NodeOutcome.__new__(NodeOutcome)
        obj.node = node
        return obj

    @cython.cfunc
    def _resolve(self) -> Outcome:
        return Outcome.wrap(self.node.deref().GetOutcome())

    def __getattr__(self, name):
        if name.startswith("_"):
            raise AttributeError(f"'NodeOutcome' object has no attribute '{name}'")
        return getattr(self._resolve(), name)

    def __getitem__(self, player):
        return self._resolve()[player]

    def __setitem__(self, player, value):
        self._resolve()[player] = value

    @property
    def label(self):
        return self._resolve().label

    @label.setter
    def label(self, value):
        self._resolve().label = value

    def __repr__(self) -> str:
        return repr(self._resolve())

    def __eq__(self, other: typing.Any) -> bool:
        return self._resolve() == other

    def __bool__(self) -> bool:
        return not self.node.deref().GetOutcome().deref().IsNull()

    def __hash__(self) -> int:
        return hash(self._resolve())


@cython.cclass
class Node:
    """A node in a ``Game``."""
    node = cython.declare(c_GameNode)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a Node outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(node: c_GameNode) -> Node:
        obj: Node = Node.__new__(Node)
        obj.node = node
        return obj

    def __repr__(self) -> str:
        if self.label:
            return f"Node(game={self.game}, label='{self.label}')"
        path = []
        node = self
        while node.parent:
            path.append(
                cython.cast(Node, node).node.deref().GetPriorAction().deref().GetNumber() - 1
            )
            node = node.parent
        return f"Node(game={self.game}, path={path})"

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, Node) and
            self.node.deref() == cython.cast(Node, other).node.deref()
        )

    def __hash__(self) -> long:
        return cython.cast(long, self.node.deref())

    def is_successor_of(self, node: Node) -> bool:
        """Returns whether this node is a successor of `node`."""
        return self.node.deref().IsSuccessorOf((<Node>node).node)

    @property
    def label(self) -> str:
        """The text label associated with the node.

        .. versionchanged:: 17.0.0
            A label may now be any well-formed UTF-8 text, not just ASCII; it must still
            contain no control characters, and must not begin/end with whitespace or have
            two consecutive whitespace characters.  "Whitespace" means any Unicode space
            separator (e.g. U+00A0 NO-BREAK SPACE), not just the ASCII space.
        """
        return self.node.deref().GetLabel().decode("utf-8")

    @label.setter
    def label(self, value: str) -> None:
        self.node.deref().SetLabel(value.encode("utf-8"))

    @property
    def number(self) -> int:
        """Returns the number of the node in its game.
        Nodes are numbered starting with 0.
        """
        return self.node.deref().GetNumber() - 1

    @property
    def children(self) -> NodeChildren:
        """The set of children of this node."""
        return NodeChildren.wrap(self.node)

    @property
    def game(self) -> Game:
        """Gets the ``Game`` to which the node belongs."""
        return Game.wrap(self.node.deref().GetGame())

    @cython.cfunc
    def _infoset_handle(self) -> c_GameInfoset:
        """The node's current information set or event, as a raw handle -- null if
        the node is currently terminal. Not part of the public API; the information
        set/event a node belongs to is otherwise only exposed piecemeal, via
        `members`/`actions`/`action_probs` and the `Game`/`Selector`-based resolvers.
        """
        return self.node.deref().GetInfoset()

    @property
    def members(self) -> list[Node]:
        """The nodes which are members of the information set or event to which
        this node currently belongs -- whichever applies.

        .. versionadded:: 17.0.0

        Raises
        ------
        AttributeError
            If this node currently belongs to no information set or event (a terminal
            node).
        """
        resolved: c_GameInfoset = self._infoset_handle()
        if resolved == cython.cast(c_GameInfoset, NULL):
            raise AttributeError("node currently belongs to no information set or event")
        return [Node.wrap(member) for member in resolved.deref().GetMembers()]

    @property
    def actions(self) -> list[str]:
        """The labels of the actions available at the node's current information set
        or event, whichever applies.

        .. versionadded:: 17.0.0

        Raises
        ------
        AttributeError
            If this node currently belongs to no information set or event (a
            terminal node).
        """
        resolved: c_GameInfoset = self._infoset_handle()
        if resolved == cython.cast(c_GameInfoset, NULL):
            raise AttributeError("node currently belongs to no information set or event")
        return [a.deref().GetLabel().decode("utf-8") for a in resolved.deref().GetActions()]

    @property
    def action_probs(self) -> dict[str, decimal.Decimal | Rational]:
        """The probability of each action at the node's current chance event, keyed
        by label.

        .. versionadded:: 17.0.0

        Raises
        ------
        UndefinedOperationError
            If the node does not currently belong to a chance event.
        """
        resolved: c_GameInfoset = self._infoset_handle()
        if resolved == cython.cast(c_GameInfoset, NULL) or not resolved.deref().IsChanceInfoset():
            raise UndefinedOperationError(
                "action probabilities are only defined at events"
            )
        result: dict = {}
        for a in resolved.deref().GetActions():
            result[a.deref().GetLabel().decode("utf-8")] = _decode_prob(
                cython.cast(string, resolved.deref().GetActionProb(a))
            )
        return result

    @property
    def player(self) -> str | None:
        """The label of the player associated with this node: the one who makes the
        decision, if this is a personal node, or the chance player, if this is an
        event.

        `None` for a terminal node, which has no player.

        .. versionchanged:: 17.0.0
            Returns the player's label (or `None`) directly, rather than a lazy,
            node-anchored view.
        """
        player: c_GamePlayer = self.node.deref().GetPlayer()
        if not (player != cython.cast(c_GamePlayer, NULL)):
            return None
        return player.deref().GetLabel().decode("utf-8")

    @property
    def parent(self) -> Node | None:
        """The parent of this node.

        If this is the root node, None is returned.
        """
        if self.node.deref().GetParent() != cython.cast(c_GameNode, NULL):
            return Node.wrap(self.node.deref().GetParent())
        return None

    @property
    def prior_action(self) -> Branch | None:
        """The branch -- the parent node and the label of the action taken from it --
        which leads to this node.

        If this is the root node, None is returned.

        .. versionchanged:: 17.0.0
            Returns a `Branch` (the parent node and the action's label) rather than
            an `Action` object, following its removal.
        """
        prior: c_GameAction = self.node.deref().GetPriorAction()
        if prior != cython.cast(c_GameAction, NULL):
            return Branch(self.parent, prior.deref().GetLabel().decode("utf-8"))
        return None

    @property
    def own_prior_action(self) -> Branch | None:
        """The last branch -- the node and the label of the action taken there -- at
        which the node's owner acted before reaching this node.

        Returns
        -------
        Branch or None
            The node at which the node's owner last acted, paired with the label of
            the action taken there, or None if the player has not moved previously
            on the path to this node.

        .. versionadded:: 16.5.0
        .. versionchanged:: 17.0.0
            Returns a `Branch` (the node and the action's label) rather than an
            `Action` object, following its removal.
        """
        prior: c_GameAction = self.node.deref().GetOwnPriorAction()
        if not (prior != cython.cast(c_GameAction, NULL)):
            return None
        label = prior.deref().GetLabel().decode("utf-8")
        cur: c_GameNode = self.node
        while cur.deref().GetPriorAction() != prior:
            cur = cur.deref().GetParent()
        return Branch(Node.wrap(cur.deref().GetParent()), label)

    @cython.cfunc
    def _is_terminal(self) -> cython.bint:
        """Whether this is a terminal node of the game. Not part of the public
        API; a node is terminal exactly when `Game.get_actions` is empty for it.
        """
        return self.node.deref().IsTerminal()

    @property
    def is_subgame_root(self) -> bool:
        """Returns whether the node is the root of a proper subgame.

        .. versionchanged:: 16.1.0
            Changed to being a property instead of a member function.
        """
        return self.node.deref().IsSubgameRoot()

    @property
    def is_strategy_reachable(self) -> bool:
        """Returns whether this node is reachable by any pure strategy profile.

        A node is considered reachable if there exists at least one pure
        strategy profile where the resulting path of play passes
        through that node.

        In games with absent-mindedness, some nodes may be unreachable because
        any path to them requires conflicting choices at the same information set.
        """
        return self.node.deref().IsStrategyReachable()

    @property
    def outcome(self) -> NodeOutcome:
        """The outcome currently attached to this node.

        Returns a lazy, node-anchored view resolved on each access, so the value reflects
        the current state of the game even if the game is mutated after this property is read.
        When no outcome is attached, the view resolves to the game's null outcome:
        its ``label`` is ``None``, its payoffs read as zero, and it compares unequal
        to every outcome, including another null.

        .. versionchanged:: 16.7.0
            Now returns a lazily-evaluated, node-anchored view rather than capturing the
            outcome at the time of access.

        .. versionchanged:: 17.0.0
            Resolves to the null outcome rather than ``None`` when no outcome is attached;
            two null outcomes compare unequal.
        """
        return NodeOutcome.wrap(self.node)

    @cython.cfunc
    def _plays(self) -> list:
        """The terminal nodes consistent with this node. Not part of the public
        API; the public equivalent is a `Selector`'s `.plays` step, e.g.
        `game.get_histories(H.path(...).plays)`.
        """
        return [Node.wrap(n) for n in self.node.deref().GetGame().deref().GetPlays(self.node)]


@cython.cclass
class Subgame:
    """A subgame in a ``Game``.

    .. versionadded:: 16.7.0
    """
    subgame = cython.declare(c_GameSubgame)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a Subgame outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(subgame: c_GameSubgame) -> Subgame:
        obj: Subgame = Subgame.__new__(Subgame)
        obj.subgame = subgame
        return obj

    def __repr__(self) -> str:
        return f"Subgame(root={self.root})"

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, Subgame) and
            self.subgame.deref() == cython.cast(Subgame, other).subgame.deref()
        )

    def __hash__(self) -> int:
        return cython.cast(cython.long, self.subgame.deref())

    @property
    def game(self) -> Game:
        """Gets the ``Game`` to which the subgame belongs.

        .. versionadded:: 16.7.0
        """
        return Game.wrap(self.subgame.deref().GetGame())

    @property
    def root(self) -> Node:
        """Returns the root node of the subgame.

        .. versionadded:: 16.7.0
        """
        return Node.wrap(self.subgame.deref().GetRoot())

    @property
    def parent(self) -> typing.Optional[Subgame]:
        """Returns the parent subgame, or None if this is the root subgame.

        .. versionadded:: 16.7.0
        """
        parent: c_GameSubgame = self.subgame.deref().GetParent()
        if parent != cython.cast(c_GameSubgame, NULL):
            return Subgame.wrap(parent)
        return None

    @property
    def children(self) -> list[Subgame]:
        """Returns the immediate child subgames of this subgame.

        .. versionadded:: 16.7.0
        """
        return [Subgame.wrap(child) for child in self.subgame.deref().GetChildren()]
