cdef class Infosets(Collection):
    "Represents a collection of infosets for a player."
    cdef c_GamePlayer player
    def __len__(self):    return self.player.deref().NumInfosets()
    def __getitem__(self, iset):
        if not isinstance(iset, int):  return Collection.__getitem__(self, iset)
        cdef Infoset s
        s = Infoset()
        s.infoset = self.player.deref().GetInfoset(iset+1)
        return s

cdef class Strategies(Collection):
    "Represents a collection of strategies for a player."
    cdef c_GamePlayer player

    def add(self, label=""):
        cdef Game g
        g = Game()
        g.game = self.player.deref().GetGame()
        if g.is_tree:
            raise TypeError, "Adding strategies is only applicable to games in strategic form"
        cdef Strategy s
        s = Strategy()
        s.strategy = self.player.deref().NewStrategy()
        s.label = str(label)
        return s

    def __len__(self):    return self.player.deref().NumStrategies()
    def __getitem__(self, st):
        if not isinstance(st, int):  return Collection.__getitem__(self, st)
        cdef Strategy s
        s = Strategy()
        s.strategy = self.player.deref().GetStrategy(st+1)
        return s
    

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

    property game:
        def __get__(self):
            cdef Game g
            g = Game()
            g.game = self.player.deref().GetGame()
            return g

    property label:
        def __get__(self):
            return self.player.deref().GetLabel().c_str()

        def __set__(self, char *value):
            # check to see if the player's name has been used elsewhere
            if value in [ i.label for i in self.game.players ]:
                warnings.warn("Another player with an identical label exists")
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
        
    property infosets:
        def __get__(self):
            cdef Infosets s
            s = Infosets()
            s.player = self.player
            return s