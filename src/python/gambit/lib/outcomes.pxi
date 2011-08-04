cdef class Outcomes:
    cdef c_Game game

    def __repr__(self):
        return "<Outcomes of game '%s'>" % self.game.deref().GetTitle().c_str()

    def __len__(self):
        return self.game.deref().NumOutcomes()

    def __getitem__(self, outc):
        cdef Outcome c
        if isinstance(outc, int):
            if outc < 0 or outc >= len(self):
                raise IndexError("no outcome with index '%s'" % outc)
            c = Outcome()
            c.outcome = self.game.deref().GetOutcome(outc+1)
            return c
        elif isinstance(outc, str):
            try:
                return self[ [ c.label for c in self ].index(outc) ]
            except ValueError:
                raise IndexError("no outcome with label '%s'" % outc)
        else:
            raise TypeError("outcomes indexable by int or str")
  