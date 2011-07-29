cdef class Actions:
    cdef c_GameInfoset infoset

    def __repr__(self):
        return str(list(self))

    def __len__(self):
        return self.infoset.deref().NumActions()

    def __getitem__(self, act):
        cdef Action a
        if act < 0 or act >= len(self):
            raise IndexError("no action with index '%s'" % act)
        a = Action()
        a.action = self.infoset.deref().GetAction(act+1)
        return a
