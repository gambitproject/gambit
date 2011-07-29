cdef class Infoset:
    cdef c_GameInfoset infoset

    def __repr__(self):
         return "<Infoset [%d] '%s' for player '%s' in game '%s'>" % \
                (self.infoset.deref().GetNumber()-1, self.label,
                 self.infoset.deref().GetPlayer().deref().GetLabel().c_str(),
                 self.infoset.deref().GetPlayer().deref().GetGame().deref().GetTitle().c_str())
    
    def __richcmp__(Infoset self, other, whichop):
        if isinstance(other, Infoset):
            if whichop == 2:
                return self.infoset.deref() == ((<Infoset>other).infoset).deref()
            elif whichop == 3:
                return self.infoset.deref() != ((<Infoset>other).infoset).deref()
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
        return long(<long>self.infoset.deref())

    property label:
        def __get__(self):
            return self.infoset.deref().GetLabel().c_str()
        def __set__(self, char *value):
            cdef cxx_string s
            s.assign(value)
            self.infoset.deref().SetLabel(s)

    property actions:
        def __get__(self):
            cdef Actions a
            a = Actions()
            a.infoset = self.infoset
            return a

    property members:
        def __get__(self):
            cdef Members m
            m = Members()
            m.infoset = self.infoset
            return m