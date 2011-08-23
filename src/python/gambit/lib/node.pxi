cdef class Children(Collection):
    "Represents the collection of direct children of a node."
    cdef c_GameNode parent
    def __len__(self):    return self.node.deref().NumChildren()
    def __getitem__(self, i):
        if not isinstance(i, int):  return Collection.__getitem__(self, i)
        cdef Node n
        n = Node()
        n.node = self.parent.deref().GetChild(i+1)
        return n

cdef class Node:
    cdef c_GameNode node

    def __repr__(self):
        return "<Node [%d] '%s' in game '%s'>" % (self.node.deref().GetNumber(),
                                                  self.label,
                                                  self.node.deref().GetGame().deref().GetTitle().c_str())
    
    def __richcmp__(Node self, other, whichop):
        if isinstance(other, Node):
            if whichop == 2:
                return self.node.deref() == ((<Node>other).node).deref()
            elif whichop == 3:
                return self.node.deref() != ((<Node>other).node).deref()
            else:
                raise NotImplementedError
        else:
            if whichop == 2:
                return False
            elif whichop == 3:
                return True
            else:
                raise NotImplementedError

    def __hash__(self):
        return long(<long>self.node.deref())

    def is_successor_of(self, node):
        if isinstance(node, Node):
            return self.node.deref().IsSuccessorOf(((<Node>node).node))
        else:
            raise TypeError, "is_successor_of takes a Node object as its input"

    def is_subgame_root(self):
        return self.node.deref().IsSubgameRoot()


    property label:
        def __get__(self):
            return self.node.deref().GetLabel().c_str()
        def __set__(self, char *value):
            cdef cxx_string s
            s.assign(value)
            self.node.deref().SetLabel(s)

    property children:
        def __get__(self):
            cdef Children c
            c = Children()
            c.parent = self.node
            return c

    property infoset:
        def __get__(self):
            cdef Infoset i
            if self.node.deref().GetInfoset() != <c_GameInfoset>NULL:
                i = Infoset()
                i.infoset = self.node.deref().GetInfoset()
                return i
            return None

    property player:
        def __get__(self):
            cdef Player p
            if self.node.deref().GetPlayer() != <c_GamePlayer>NULL:
                p = Player()
                p.player = self.node.deref().GetPlayer()
                return p
            return None

    property parent:
        def __get__(self):
            cdef Node n
            if self.node.deref().GetParent() != <c_GameNode>NULL:
                n = Node()
                n.node = self.node.deref().GetParent()
                return n
            return None
            
    property prior_action:
        def __get__(self):
            cdef Action a
            if self.node.deref().GetPriorAction() != <c_GameAction>NULL:
                a = Action()
                a.action = self.node.deref().GetPriorAction()
                return a
            return None

    property prior_sibling:
        def __get__(self):
            cdef Node n
            if self.node.deref().GetPriorSibling() != <c_GameNode>NULL:
                n = Node()
                n.node = self.node.deref().GetPriorSibling()
                return n
            return None

    property next_sibling:
        def __get__(self):
            cdef Node n
            if self.node.deref().GetNextSibling() != <c_GameNode>NULL:
                n = Node()
                n.node = self.node.deref().GetNextSibling()
                return n
            return None
            
    property is_terminal:
        def __get__(self):
            return self.node.deref().IsTerminal()

    property outcome:
        def __get__(self):
            cdef Outcome o
            if self.node.deref().GetOutcome() != <c_GameOutcome>NULL:
                o = Outcome()
                o.outcome = self.node.deref().GetOutcome()
                return o
            return None
        
