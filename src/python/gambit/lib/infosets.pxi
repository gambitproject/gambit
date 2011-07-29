cdef class Infosets:
    cdef c_GamePlayer player

    def __repr__(self):
        return str(list(self))

    def __len__(self):
        return self.player.deref().NumInfosets()

    def __getitem__(self, st):
        cdef Infoset s
        if st < 0 or st >= len(self):
            raise IndexError("no infoset with index '%s'" % st)
        s = Infoset()
        s.infoset = self.player.deref().GetInfoset(st+1)
        return s
