cdef class Player:
    cdef c_GamePlayer player

    def __repr__(self):
        if self.is_chance:
            return "<Player [CHANCE] in game '%s'>" % self.player.deref().GetGame().deref().GetTitle().c_str()
        return "<Player [%d] '%s' in game '%s'>" % (self.player.deref().GetNumber()-1,
                                                    self.label,
                                                    self.player.deref().GetGame().deref().GetTitle().c_str())
    
    def __richcmp__(Player self, other, whichop):
        if isinstance(other, Player):
            if whichop == 2:
                return self.player.deref() == ((<Player>other).player).deref()
            elif whichop == 3:
                return self.player.deref() != ((<Player>other).player).deref()
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
        return long(<long>self.player.deref())

    property label:
        def __get__(self):
            return self.player.deref().GetLabel().c_str()
        def __set__(self, char *value):
            cdef cxx_string s
            s.assign(value)
            self.player.deref().SetLabel(s)

    property is_chance:
        def __get__(self):
            return True if self.player.deref().IsChance() != 0 else False

    property strategies:
        def __get__(self):
            cdef Strategies s
            s = Strategies()
            s.player = self.player
            return s
