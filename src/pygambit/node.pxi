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

    def __getitem__(self, action: int | str | Action) -> Node:
        """Returns the successor node which is reached after 'action' is played.

        .. versionchanged: 16.5.0
            Previously indexing by string searched the labels of the child nodes,
            rather than referring to actions.  This implements the more natural
            interpretation that strings refer to action labels.

            Relatedly, the collection can now be indexed by an Action object.
        """
        if isinstance(action, str):
            if not action.strip():
                raise ValueError("Action label cannot be empty or all whitespace")
            if self.parent.deref().GetInfoset() == cython.cast(c_GameInfoset, NULL):
                raise ValueError(f"No action with label '{action}' at node")
            for act in self.parent.deref().GetInfoset().deref().GetActions():
                if act.deref().GetLabel().decode("ascii") == cython.cast(str, action):
                    return Node.wrap(self.parent.deref().GetChild(act))
            raise ValueError(f"No action with label '{action}' at node")
        if isinstance(action, Action):
            try:
                return Node.wrap(self.parent.deref().GetChild(cython.cast(Action, action).action))
            except IndexError:
                raise ValueError(f"Action is from a different information set than node")
        if isinstance(action, int):
            if self.parent.deref().GetInfoset() == cython.cast(c_GameInfoset, NULL):
                raise IndexError("Index out of range")
            return Node.wrap(self.parent.deref().GetChild(
                self.parent.deref().GetInfoset().deref().GetAction(action + 1)
            ))
        raise TypeError(f"Index must be int, str, or Action, not {action.__class__.__name__}")


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
            path.append(node.prior_action.number)
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
        """The text label associated with the node."""
        return self.node.deref().GetLabel().decode("ascii")

    @label.setter
    def label(self, value: str) -> None:
        self.node.deref().SetLabel(value.encode("ascii"))

    @property
    def children(self) -> NodeChildren:
        """The set of children of this node."""
        return NodeChildren.wrap(self.node)

    @property
    def game(self) -> Game:
        """Gets the ``Game`` to which the node belongs."""
        return Game.wrap(self.node.deref().GetGame())

    @property
    def infoset(self) -> Infoset | None:
        """The information set to which this node belongs.

        If this is a terminal node, which belongs to no information set,
        None is returned.
        """
        if self.node.deref().GetInfoset() != cython.cast(c_GameInfoset, NULL):
            return Infoset.wrap(self.node.deref().GetInfoset())
        return None

    @property
    def player(self) -> Player | None:
        """The player who makes the decision at this node.

        If this is a terminal node, None is returned.
        """
        if self.node.deref().GetPlayer() != cython.cast(c_GamePlayer, NULL):
            return Player.wrap(self.node.deref().GetPlayer())
        return None

    @property
    def parent(self) -> Node | None:
        """The parent of this node.

        If this is the root node, None is returned.
        """
        if self.node.deref().GetParent() != cython.cast(c_GameNode, NULL):
            return Node.wrap(self.node.deref().GetParent())
        return None

    @property
    def prior_action(self) -> Action | None:
        """The action which leads to this node.

        If this is the root node, None is returned.
        """
        if self.node.deref().GetPriorAction() != cython.cast(c_GameAction, NULL):
            return Action.wrap(self.node.deref().GetPriorAction())
        return None

    @property
    def own_prior_action(self) -> Action | None:
        """The last action taken by the node's owner before reaching this node.

        Returns
        -------
        Action or None
            The action object, or None if the player has not moved previously
            on the path to this node.
        .. versionadded:: 16.5.0

        See Also
        --------
        Infoset.own_prior_actions
        """
        if self.node.deref().GetOwnPriorAction() != cython.cast(c_GameAction, NULL):
            return Action.wrap(self.node.deref().GetOwnPriorAction())
        return None

    @property
    def prior_sibling(self) -> Node | None:
        """The node which is immediately before this one in its parent's children.

        If this is the root node or the first child of its parent,
        None is returned.
        """
        if self.node.deref().GetPriorSibling() != cython.cast(c_GameNode, NULL):
            return Node.wrap(self.node.deref().GetPriorSibling())
        return None

    @property
    def next_sibling(self) -> Node | None:
        """The node which is immediately after this one in its parent's children.

        If this is the root node or the last child of its parent,
        None is returned.
        """
        if self.node.deref().GetNextSibling() != cython.cast(c_GameNode, NULL):
            return Node.wrap(self.node.deref().GetNextSibling())
        return None

    @property
    def is_terminal(self) -> bool:
        """Returns whether this is a terminal node of the game."""
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
    def outcome(self) -> Outcome | None:
        """Returns the outcome attached to the node.

        If no outcome is attached to the node, None is returned.
        """
        if self.node.deref().GetOutcome() == cython.cast(c_GameOutcome, NULL):
            return None
        return Outcome.wrap(self.node.deref().GetOutcome())

    @property
    def plays(self) -> list[Node]:
        """Returns a list of all terminal `Node` objects consistent with it.
        """
        return [Node.wrap(n) for n in self.node.deref().GetGame().deref().GetPlays(self.node)]
