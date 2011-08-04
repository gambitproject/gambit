cdef class Players:
    cdef c_Game game

    def __repr__(self):
        return str(list(self))

    def __len__(self):
        return self.game.deref().NumPlayers()

    def __getitem__(self, pl):
        cdef Player p
        if isinstance(pl, int):
            if pl < 0 or pl >= len(self):
                raise IndexError("no player with index '%s'" % pl)
            p = Player()
            p.player = self.game.deref().GetPlayer(pl+1)
            return p
        elif isinstance(pl, str):
            try:
                return self[ [ x.label for x in self ].index(pl) ]
            except ValueError:
                raise IndexError("no player with label '%s'" % pl)
        else:
            raise TypeError("players indexable by int or str")

    def add(self, label=""):
        cdef Player p
        p = Player()
        p.player = self.game.deref().NewPlayer()
        p.label = str(label)
        return p

    property chance:
        def __get__(self):
            cdef Player p
            p = Player()
            p.player = self.game.deref().GetChance()
            return p
