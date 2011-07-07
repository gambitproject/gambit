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
