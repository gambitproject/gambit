#
# This file is part of Gambit
# Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/lib/node.pxi
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
class Children(Collection):
    """Represents the collection of direct children of a node."""
    parent = cython.declare(c_GameNode)

    def __len__(self):
        return self.parent.deref().NumChildren()

    def __getitem__(self, i):
        if not isinstance(i, int):
            return Collection.__getitem__(self, i)
        n = Node()
        n.node = self.parent.deref().GetChild(i+1)
        return n


@cython.cclass
class Node:
    """A node in a ``Game``."""
    node = cython.declare(c_GameNode)

    def __repr__(self) -> str:
        return (
            f"<Node [{self.node.deref().GetNumber()}] '{self.label}' "
            f"in game '{self.game.title}'>"
        )

    def __eq__(self, other: typing.Any) -> bool:
        return isinstance(other, Node) and self.node.deref() == cython.cast(Node, other).node.deref()

    def __hash__(self) -> long:
        return cython.cast(long, self.node.deref())

    def is_successor_of(self, node: Node) -> bool:
        """Returns whether this node is a successor of `node`."""
        return self.node.deref().IsSuccessorOf((<Node>node).node)

    @property
    def label(self) -> str:
        """The text label associated with the node."""
        return self.node.deref().GetLabel().decode('ascii')

    @label.setter
    def label(self, value: str) -> None:
        self.node.deref().SetLabel(value.encode('ascii'))

    @property
    def children(self) -> Children:
        """The set of children of this node."""
        c = Children()
        c.parent = self.node
        return c

    @property
    def game(self) -> Game:
        """Gets the ``Game`` to which the node belongs."""
        g = Game()
        g.game = self.node.deref().GetGame()
        return g

    @property
    def infoset(self) -> typing.Optional[Infoset]:
        """The information set to which this node belongs.

        If this is a terminal node, which belongs to no information set,
        None is returned.
        """
        if self.node.deref().GetInfoset() != cython.cast(c_GameInfoset, NULL):
            i = Infoset()
            i.infoset = self.node.deref().GetInfoset()
            return i
        return None

    @property
    def player(self) -> typing.Optional[Player]:
        """The player who makes the decision at this node.

        If this is a terminal node, None is returned.
        """
        if self.node.deref().GetPlayer() != cython.cast(c_GamePlayer, NULL):
            p = Player()
            p.player = self.node.deref().GetPlayer()
            return p
        return None

    @property
    def parent(self) -> typing.Optional[Node]:
        """The parent of this node.

        If this is the root node, None is returned.
        """
        if self.node.deref().GetParent() != cython.cast(c_GameNode, NULL):
            n = Node()
            n.node = self.node.deref().GetParent()
            return n
        return None

    @property
    def prior_action(self) -> typing.Optional[Action]:
        """The action which leads to this node.

        If this is the root node, None is returned.
        """
        if self.node.deref().GetPriorAction() != cython.cast(c_GameAction, NULL):
            a = Action()
            a.action = self.node.deref().GetPriorAction()
            return a
        return None

    @property
    def prior_sibling(self) -> typing.Optional[Node]:
        """The node which is immediately before this one in its parent's children.

        If this is the root node or the first child of its parent,
        None is returned.
        """
        if self.node.deref().GetPriorSibling() != cython.cast(c_GameNode, NULL):
            n = Node()
            n.node = self.node.deref().GetPriorSibling()
            return n
        return None

    @property
    def next_sibling(self) -> typing.Optional[Node]:
        """The node which is immediately after this one in its parent's children.

        If this is the root node or the last child of its parent,
        None is returned.
        """
        if self.node.deref().GetNextSibling() != cython.cast(c_GameNode, NULL):
            n = Node()
            n.node = self.node.deref().GetNextSibling()
            return n
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
    def outcome(self) -> typing.Optional[Outcome]:
        """Returns the outcome attached to the node.

        If no outcome is attached to the node, None is returned.
        """
        if self.node.deref().GetOutcome() == cython.cast(c_GameOutcome, NULL):
            return None
        o = Outcome()
        o.outcome = self.node.deref().GetOutcome()
        return o

