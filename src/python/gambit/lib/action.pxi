from gambit.lib.error import UndefinedOperationError

cdef class Action:
    cdef c_GameAction action

    def __repr__(self):
         return "<Action [%d] '%s' at infoset '%s' for player '%s' in game '%s'>" % \
                (self.action.deref().GetNumber()-1, self.label,
                 self.action.deref().GetInfoset().deref().GetLabel().c_str(),
                 self.action.deref().GetInfoset().deref().GetPlayer().deref().GetLabel().c_str(),
                 self.action.deref().GetInfoset().deref().GetPlayer().deref().GetGame().deref().GetTitle().c_str())
    
    def __richcmp__(Action self, other, whichop):
        if isinstance(other, Action):
            if whichop == 2:
                return self.action.deref() == ((<Action>other).action).deref()
            elif whichop == 3:
                return self.action.deref() != ((<Action>other).action).deref()
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
        return long(<long>self.action.deref())

    def delete(self):
        if len(self.infoset.actions) == 1:
            raise UndefinedOperationError("it is not possible to delete the \
                                            last action of an infoset")
        self.action.deref().DeleteAction()

    def precedes(self, node):
        if isinstance(node, Node):
            return self.action.deref().Precedes(((<Node>node).node))
        else:
            raise TypeError("Precedes takes a Node object as its input")

    property label:
        def __get__(self):
            return self.action.deref().GetLabel().c_str()
        def __set__(self, char *value):
            cdef cxx_string s
            s.assign(value)
            self.action.deref().SetLabel(s)

    property infoset:
        def __get__(self):
            cdef Infoset i
            i = Infoset()
            i.infoset = self.action.deref().GetInfoset()
            return i

    property prob:
        def __get__(self):
            cdef bytes py_string
            py_string = self.action.deref().GetInfoset().deref().GetActionProb(
                self.action.deref().GetNumber()).as_string().c_str()
            if "." in py_string:
                return decimal.Decimal(py_string)
            else:
                return fractions.Fraction(py_string)
        
        def __set__(self, value):
            cdef cxx_string s
            if isinstance(value, (int, decimal.Decimal, fractions.Fraction)):
                v = str(value)
                s.assign(v)
                self.action.deref().GetInfoset().deref().SetActionProb(
                    self.action.deref().GetNumber(), s)
            else:
                raise TypeError, "numeric argument required for action \
                    probability"
            