cdef class Outcomes(Collection):
    "Represents a collection of outcomes in a game."
    cdef c_Game game
    def __len__(self):    return self.game.deref().NumOutcomes()
    def __getitem__(self, outc):
        if not isinstance(outc, int):  return Collection.__getitem__(self, outc)
        cdef Outcome c
        c = Outcome()
        c.outcome = self.game.deref().GetOutcome(outc+1)
        return c

cdef class Players(Collection):
    "Represents a collection of players in a game."
    cdef c_Game game
    def __len__(self):       return self.game.deref().NumPlayers()
    def __getitem__(self, pl):
        if not isinstance(pl, int):  return Collection.__getitem__(self, pl)
        cdef Player p
        p = Player()
        p.player = self.game.deref().GetPlayer(pl+1)
        return p

    def add(self, label=""):
        cdef Player p
        p = Player()
        p.player = self.game.deref().NewPlayer()
        if label != "": p.label = str(label)
        return p

    property chance:
        def __get__(self):
            cdef Player p
            p = Player()
            p.player = self.game.deref().GetChance()
            return p

cdef class Game:
    cdef c_Game game

    def __str__(self):
        return "<Game '%s'>" % self.title

    def __repr__(self):
        return self.write()

    def __richcmp__(Game self, other, whichop):
        if isinstance(other, Game):
            if whichop == 2:
                return self.game.deref() == ((<Game>other).game).deref()
            elif whichop == 3:
                return self.game.deref() != ((<Game>other).game).deref()
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
        return long(<long>self.game.deref())

    property is_tree:
        def __get__(self):
            return True if self.game.deref().IsTree() != 0 else False

    property title:
        def __get__(self):
            return self.game.deref().GetTitle().c_str()
        def __set__(self, char *value):
            cdef cxx_string s
            s.assign(value)
            self.game.deref().SetTitle(s)

    property players:
        def __get__(self):
            cdef Players p
            p = Players()
            p.game = self.game
            return p

    property outcomes:
        def __get__(self):
            cdef Outcomes c
            c = Outcomes()
            c.game = self.game
            return c

    property contingencies:
        def __get__(self):
            return gambit.gameiter.Contingencies(self)

    property root:
        def __get__(self):
            cdef Node n
            n = Node()
            n.node = self.game.deref().GetRoot()
            return n

    def _get_contingency(self, *args):
        cdef c_PureStrategyProfile *psp
        cdef Outcome outcome
        psp = new_PureStrategyProfile(self.game)
        
        
        for (pl, st) in enumerate(args):
            psp.SetStrategy(self.game.deref().GetPlayer(pl+1).deref().GetStrategy(st+1))

        outcome = Outcome()
        outcome.outcome = psp.GetOutcome()
        del_PureStrategyProfile(psp)
        return outcome



    # As of Cython 0.11.2, cython does not support the * notation for the argument
    # to __getitem__, which is required for multidimensional slicing to work. 
    # We work around this by providing a shim.
    def __getitem__(self, i):
        try:
            if len(i) != len(self.players):
                raise KeyError, "Number of strategies is not equal to the number of players"
        except TypeError:
            raise TypeError, "contingency must be a tuple-like object"
        cont = [ 0 ] * len(self.players)
        for (pl, st) in enumerate(i):
            if isinstance(st, int):
                if st < 0 or st >= len(self.players[st].strategies):
                    raise IndexError, "Provided strategy index %d out of range" % st
                cont[pl] = st
            elif isinstance(st, str):
                try:
                    cont[pl] = [ s.label for s in self.players[pl].strategies ].index(st)
                except ValueError:
                    raise IndexError, "Provided strategy label '%s' not defined" % st
            elif isinstance(st, Strategy):
                try:
                    cont[pl] = list(self.players[pl].strategies).index(st)
                except ValueError:
                    raise IndexError, "Provided strategy '%s' not available to player" % st
            else:
                raise TypeError("Must use a tuple of ints, strategy labels, or strategies")
        return self._get_contingency(*tuple(cont))



    def mixed_profile(self):
        cdef MixedStrategyProfileDouble msp
        msp = MixedStrategyProfileDouble()
        msp.profile = new_MixedStrategyProfileDouble(self.game)
        return msp
 
    def num_nodes(self):
        return self.game.deref().NumNodes()

    def write(self, strategic=False):
        if strategic or not self.is_tree:
            return WriteGame(self.game, 1).c_str()
        else:
            return WriteGame(self.game, 0).c_str()
