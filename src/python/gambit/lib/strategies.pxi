cdef class Strategies:
    cdef c_GamePlayer player

    def __repr__(self):
        return str(list(self))

    def __len__(self):
        return self.player.deref().NumStrategies()

    def __getitem__(self, st):
        cdef Strategy s
        
        if isinstance(st, int):
            if st < 0 or st >= len(self):
                raise IndexError("no strategy with index '%s'" % st)
            s = Strategy()
            s.strategy = self.player.deref().GetStrategy(st+1)
            return s
        elif isinstance(st, str):
            try:
                return self[ [ x.label for x in self ].index(st) ]
            except ValueError:
                raise IndexError("no strategy with label '%s'" % st)
        else:
            raise TypeError("strategies indexable by int or str")
        
