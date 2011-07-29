cdef class Children:
    "Represents the collection of direct children of a node."
    cdef c_GameNode node
    def __repr__(self):
        return str(list(self))
    def __len__(self):
        return self.node.deref().NumChildren()
    def __getitem__(self, i):
        cdef Node n
        if i < 0 or i >= len(self):
            raise IndexError("no child with index '%s'" % i)
        n = Node()
        n.node = self.node.deref().GetChild(i+1)
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
            c.node = self.node
            return c