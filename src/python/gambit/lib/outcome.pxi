cdef class Outcome:
    cdef c_GameOutcome outcome
    cdef StrategicRestriction support
    
    def __repr__(self):
        return "<Outcome [%d] '%s' in game '%s'>" % (self.outcome.deref().GetNumber()-1,
                                                     self.label,
                                                     self.game.title)
    
    def __richcmp__(Outcome self, other, whichop):
        if isinstance(other, Outcome):
            if whichop == 2:
                return self.outcome.deref() == ((<Outcome>other).outcome).deref()
            elif whichop == 3:
                return self.outcome.deref() != ((<Outcome>other).outcome).deref()
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
        return long(<long>self.outcome.deref())

    def delete(self):
        if self.support is not None:
            raise UndefinedOperationError("Changing objects in a support is not supported")
        (<Game>self.game).game.deref().DeleteOutcome(self.outcome)

    property game:
        def __get__(self):
            cdef Game g
            if self.support is not None:
                return self.support
            g = Game()
            g.game = self.outcome.deref().GetGame()
            return g

    property label:
        def __get__(self):
            return self.outcome.deref().GetLabel().c_str()
        def __set__(self, char *value):
            if self.support is not None:
                raise UndefinedOperationError("Changing objects in a support is not supported")
            if value in [ i.label for i in self.game.outcomes ]:
                warnings.warn("Another outcome with an identical label exists")
            cdef cxx_string s
            s.assign(value)
            self.outcome.deref().SetLabel(s)

    def __getitem__(self, pl):
        cdef bytes py_string
        py_string = self.outcome.deref().GetPayoffNumber(pl+1).as_string().c_str()
        if "." in py_string:
            return decimal.Decimal(py_string)
        else:
            return fractions.Fraction(py_string)

    def __setitem__(self, pl, value):
        if self.support is not None:
            raise UndefinedOperationError("Changing objects in a support is not supported")
        cdef cxx_string s
        if isinstance(value, int) or isinstance(value, decimal.Decimal) or \
           isinstance(value, fractions.Fraction):
            v = str(value)
            s.assign(v)
            self.outcome.deref().SetPayoff(pl+1, s)
        else:
            raise TypeError, "numeric argument required for payoff"

    def unrestrict(self):
        cdef Outcome o
        o = Outcome()
        o.outcome = self.outcome
        return o
