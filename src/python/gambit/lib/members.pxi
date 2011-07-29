cdef class Members:
    cdef c_GameInfoset infoset

    def __repr__(self):
        return str(list(self))

    def __len__(self):
        return self.infoset.deref().NumMembers()

    def __getitem__(self, i):
        cdef Node n
        if i < 0 or i >= len(self):
            raise IndexError("no member with index '%s'" % i)
        n = Node()
        n.node = self.infoset.deref().GetMember(i+1)
        return n
