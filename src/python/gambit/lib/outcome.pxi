cdef class Outcome:
    cdef c_GameOutcome outcome

    def __repr__(self):
        return "<Outcome [%d] '%s' in game '%s'>" % (self.outcome.deref().GetNumber()-1,
                                                     self.label,
                                                     self.outcome.deref().GetGame().deref().GetTitle().c_str())
    
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

    property label:
        def __get__(self):
            return self.outcome.deref().GetLabel().c_str()
        def __set__(self, char *value):
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
        cdef cxx_string s
        if isinstance(value, int) or isinstance(value, decimal.Decimal) or \
           isinstance(value, fractions.Fraction):
            v = str(value)
            s.assign(v)
            self.outcome.deref().SetPayoff(pl+1, s)
        else:
            raise TypeError, "numeric argument required for payoff"
