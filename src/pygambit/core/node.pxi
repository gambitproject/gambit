#
# This file is part of Gambit
# Copyright (c) 1994-2022, The Gambit Project (http://www.gambit-project.org)
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

cdef class Children(Collection):
    """Represents the collection of direct children of a node."""
    cdef c_GameNode parent

    def __len__(self):
        return self.parent.deref().NumChildren()

    def __getitem__(self, i):
        if not isinstance(i, int):
            return Collection.__getitem__(self, i)
        cdef Node n
        n = Node()
        n.node = self.parent.deref().GetChild(i+1)
        return n


cdef class Node:
    """Represents a node in a :py:class:`Game`."""
    cdef c_GameNode node

    def __repr__(self) -> str:
        return (
            f"<Node [{self.node.deref().GetNumber()}] '{self.label}' "
            f"in game '{self.game.title}'>"
        )

    def __richcmp__(Node self, other, whichop) -> bool:
        if isinstance(other, Node):
            if whichop == 2:
                return self.node.deref() == (<Node>other).node.deref()
            elif whichop == 3:
                return self.node.deref() != (<Node>other).node.deref()
            else:
                raise NotImplementedError
        else:
            if whichop == 2:
                return False
            elif whichop == 3:
                return True
            else:
                raise NotImplementedError

    def __hash__(self) -> long:
        return long(<long>self.node.deref())

    def is_successor_of(self, node: Node) -> bool:
        return self.node.deref().IsSuccessorOf((<Node>node).node)

    def is_subgame_root(self) -> bool:
        return self.node.deref().IsSubgameRoot()

    def append_move(self, player, actions=None):
        cdef Infoset i
        if len(self.children) > 0:
            raise UndefinedOperationError("append_move can only be applied at a terminal node")
        if isinstance(player, Player):
            if actions is None:
                raise UndefinedOperationError("append_move with a Player requires actions to be specified")
            if actions < 1:
                raise UndefinedOperationError("append_move requires actions >= 1")
            if player.game != self.game:
                raise MismatchError("append_move can only be applied between objects of the same game")
            i = Infoset()
            i.infoset = self.node.deref().AppendMove((<Player>player).player, actions)
            return i
        elif isinstance(player, Infoset):
            if actions is not None:
                raise UndefinedOperationError("append_move with an Infoset cannot specify number of actions")
            if player.game != self.game:
                raise MismatchError("append_move can only be applied between objects of the same game")
            i = Infoset()
            i.infoset = self.node.deref().AppendMove((<Infoset>player).infoset)
            return i
        raise TypeError, "append_move accepts either a Player or Infoset to specify information"

    def insert_move(self, player, actions=None):
        cdef Infoset i
        if isinstance(player, Player):
            if actions is None:
                raise UndefinedOperationError("insert_move with a Player requires actions to be specified")
            if actions < 1:
                raise UndefinedOperationError("insert_move requires actions >= 1")
            if player.game != self.game:
                raise MismatchError("append_move can only be applied between objects of the same game")
            i = Infoset()
            i.infoset = self.node.deref().InsertMove((<Player>player).player, actions)
            return i
        elif isinstance(player, Infoset):
            if actions is not None:
                raise UndefinedOperationError("insert_move with an Infoset cannot specify number of actions")
            if player.game != self.game:
                raise MismatchError("append_move can only be applied between objects of the same game")
            i = Infoset()
            i.infoset = self.node.deref().InsertMove((<Infoset>player).infoset)
            return i
        raise TypeError, "insert_move accepts either a Player or Infoset to specify information"

    def leave_infoset(self) -> Infoset:
        """Removes this node from its information set. If this node is the only node
        in its information set, this operation has no effect.

        Returns
        -------
        Infoset
            The information set to which the node belongs after the operation.
        """
        cdef Infoset i
        i = Infoset()
        i.infoset = self.node.deref().LeaveInfoset()
        return i

    def delete_parent(self):
        self.node.deref().DeleteParent()

    def delete_tree(self):
        self.node.deref().DeleteTree()

    def copy_tree(self, node: Node):
        if node.game != self.game:
            raise MismatchError(
                f"copy_tree(): trees can only be copied within the same game"
            )
        self.node.deref().CopyTree((<Node>node).node)

    def move_tree(self, node: Node):
        if node.game != self.game:
            raise MismatchError(
                f"move_tree(): trees can only be moved within the same game"
            )
        self.node.deref().MoveTree((<Node>node).node)
 
    property label:
        def __get__(self):
            return self.node.deref().GetLabel().decode('ascii')

        def __set__(self, str value):
            self.node.deref().SetLabel(value.encode('ascii'))

    property children:
        def __get__(self):
            cdef Children c
            c = Children()
            c.parent = self.node
            return c

    property game:
        """Gets the :py:class:`Game` to which the node belongs."""
        def __get__(self) -> Game:
            cdef Game g
            g = Game()
            g.game = self.node.deref().GetGame()
            return g

    property infoset:
        def __get__(self) -> typing.Optional[Infoset]:
            cdef Infoset i
            if self.node.deref().GetInfoset() != <c_GameInfoset>NULL:
                i = Infoset()
                i.infoset = self.node.deref().GetInfoset()
                return i
            return None

        def __set__(self, infoset: Infoset):
            try:
                self.node.deref().SetInfoset((<Infoset>infoset).infoset)
            except ValueError:
                raise ValueError(
                    f"in setting infoset: node has {len(self.children)} children, but infoset has {len(infoset.actions)} actions"
                ) from None

    property player:
        def __get__(self) -> typing.Optional[Player]:
            cdef Player p
            if self.node.deref().GetPlayer() != <c_GamePlayer>NULL:
                p = Player()
                p.player = self.node.deref().GetPlayer()
                return p
            return None

    property parent:
        def __get__(self) -> typing.Optional[Node]:
            cdef Node n
            if self.node.deref().GetParent() != <c_GameNode>NULL:
                n = Node()
                n.node = self.node.deref().GetParent()
                return n
            return None
            
    property prior_action:
        def __get__(self) -> typing.Optional[Action]:
            cdef Action a
            if self.node.deref().GetPriorAction() != <c_GameAction>NULL:
                a = Action()
                a.action = self.node.deref().GetPriorAction()
                return a
            return None

    property prior_sibling:
        def __get__(self) -> typing.Optional[Node]:
            cdef Node n
            if self.node.deref().GetPriorSibling() != <c_GameNode>NULL:
                n = Node()
                n.node = self.node.deref().GetPriorSibling()
                return n
            return None

    property next_sibling:
        def __get__(self) -> typing.Optional[None]:
            cdef Node n
            if self.node.deref().GetNextSibling() != <c_GameNode>NULL:
                n = Node()
                n.node = self.node.deref().GetNextSibling()
                return n
            return None
            
    property is_terminal:
        def __get__(self) -> bool:
            return self.node.deref().IsTerminal()

    property is_subgame_root:
        """Returns `True` if the node is the root of a proper subgame.

        .. versionchanged:: 16.1.0
            Changed to being a property instead of a member function.
        """
        def __get__(self) -> bool:
            return self.node.deref().IsSubgameRoot()

    property outcome:
        def __get__(self) -> typing.Optional[Outcome]:
            cdef Outcome o
            if self.node.deref().GetOutcome() != <c_GameOutcome>NULL:
                o = Outcome()
                o.outcome = self.node.deref().GetOutcome()
                return o
            return None
        
        def __set__(self, outcome: typing.Optional[Outcome]):
            if outcome is None:
                self.node.deref().SetOutcome(<c_GameOutcome>NULL)
            elif isinstance(outcome, Outcome):
                self.node.deref().SetOutcome((<Outcome>outcome).outcome)
            else:
                raise TypeError(f"argument must be an Outcome or None, not '{outcome.__class__.__name__}'")
             
