from gambit.lib.error import UndefinedOperationError

cdef class Strategy:
    cdef c_GameStrategy strategy
    cdef StrategicRestriction support

    def __repr__(self):
        return "<Strategy [%d] '%s' for player '%s' in game '%s'>" % \
                (self.number, self.label,
                 self.player.label,
                 self.player.game.title)
    
    def __richcmp__(Strategy self, other, whichop):
        if isinstance(other, Strategy):
            if whichop == 2:
                return self.strategy.deref() == ((<Strategy>other).strategy).deref()
            elif whichop == 3:
                return self.strategy.deref() != ((<Strategy>other).strategy).deref()
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
        return long(<long>self.strategy.deref())

    property label:
        def __get__(self):
            return self.strategy.deref().GetLabel().c_str()
        def __set__(self, char *value):
            if self.support is not None:
                raise UndefinedOperationError("Changing objects in a support is not supported")
            if value in [ i.label for i in self.player.strategies ]:
                warnings.warn("This player has another strategy with an identical label")
            cdef cxx_string s
            s.assign(value)
            self.strategy.deref().SetLabel(s)

    property player:
        def __get__(self):
            p = Player()
            p.support = self.support
            p.player = self.strategy.deref().GetPlayer()
            return p

    property number:
        def __get__(self):
            return self.strategy.deref().GetNumber() - 1 

    def unrestrict(self):
        cdef Strategy s
        s = Strategy()
        s.strategy = self.strategy
        return s
