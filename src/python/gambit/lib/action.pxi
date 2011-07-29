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

    property label:
        def __get__(self):
            return self.action.deref().GetLabel().c_str()
        def __set__(self, char *value):
            cdef cxx_string s
            s.assign(value)
            self.action.deref().SetLabel(s)
