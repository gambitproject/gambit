cdef class Strategies:
    cdef c_GamePlayer player

    def __repr__(self):
        return str(list(self))

    def __len__(self):
        return self.player.deref().NumStrategies()

    def __getitem__(self, st):
        cdef Strategy s
        if st < 0 or st >= len(self):
            raise IndexError("no strategy with index '%s'" % st)
        s = Strategy()
        s.strategy = self.player.deref().GetStrategy(st+1)
        return s
