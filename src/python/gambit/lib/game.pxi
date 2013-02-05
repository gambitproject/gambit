from libcpp cimport bool
from gambit.lib.error import UndefinedOperationError

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

    def add(self, label=""):
        cdef Outcome c
        c = Outcome()
        c.outcome = self.game.deref().NewOutcome()
        if label != "": c.label = str(label)
        return c

cdef class Players(Collection):
    "Represents a collection of players in a game."
    cdef c_Game game
    cdef StrategicRestriction restriction
    def __len__(self):       return self.game.deref().NumPlayers()
    def __getitem__(self, pl):
        if not isinstance(pl, int):  return Collection.__getitem__(self, pl)
        cdef Player p
        p = Player()
        p.player = self.game.deref().GetPlayer(pl+1)
        if self.restriction is not None:
            p.restriction = self.restriction
        return p

    def add(self, label=""):
        cdef Player p
        if self.restriction is not None:
            raise UndefinedOperationError("Changing objects in a restriction is not supported")
        p = Player()
        p.player = self.game.deref().NewPlayer()
        if label != "": p.label = str(label)
        return p

    property chance:
        def __get__(self):
            cdef Player p
            p = Player()
            p.player = self.game.deref().GetChance()
            p.restriction = self.restriction
            return p

cdef class GameActions(Collection):
    "Represents a collection of actions in a game."
    cdef c_Game game
    def __len__(self):
        return self.game.deref().BehavProfileLength()
    def __getitem__(self, action):
        if not isinstance(action, int):
            return Collection.__getitem__(self, action)
        cdef Action a
        a = Action()
        a.action = self.game.deref().GetAction(action+1)
        return a

cdef class GameInfosets(Collection):
    "Represents a collection of infosets in a game."
    cdef c_Game game
    def __len__(self):
        cdef Array[int] num_infosets
        num_infosets = self.game.deref().NumInfosets()
        size = num_infosets.Length()
        n = 0
        for i in range(1,size+1):
            n += num_infosets.getitem(i)
        return n
    def __getitem__(self, infoset):
        if not isinstance(infoset, int):
            return Collection.__getitem__(self, infoset)
        cdef Infoset i
        i = Infoset()
        i.infoset = self.game.deref().GetInfoset(infoset+1)
        return i

cdef class GameStrategies(Collection):
    "Represents a collection of strategies in a game."
    cdef c_Game game
    def __len__(self):
        return self.game.deref().MixedProfileLength()
    def __getitem__(self, st):
        if not isinstance(st, int):
            return Collection.__getitem__(self, st)
        cdef Strategy s
        s = Strategy()
        s.strategy = self.game.deref().GetStrategy(st+1)
        return s

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

    property comment:
        def __get__(self):
            return self.game.deref().GetComment().c_str()
        def __set__(self, char *value):
            cdef cxx_string s
            s.assign(value)
            self.game.deref().SetComment(s)

    property actions:
        def __get__(self):
            cdef GameActions a
            if self.is_tree:
                a = GameActions()
                a.game = self.game
                return a
            raise UndefinedOperationError("Operation only defined for "\
                                           "games with a tree representation")

    property infosets:
        def __get__(self):
            cdef GameInfosets i
            if self.is_tree:
                i = GameInfosets()
                i.game = self.game
                return i
            raise UndefinedOperationError("Operation only defined for "\
                                           "games with a tree representation")

    property players:
        def __get__(self):
            cdef Players p
            p = Players()
            p.game = self.game
            return p

    property strategies:
        def __get__(self):
            cdef GameStrategies s
            s = GameStrategies()
            s.game = self.game
            return s

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
            if self.is_tree:
                n = Node()
                n.node = self.game.deref().GetRoot()
                return n
            raise UndefinedOperationError("Operation only defined for "\
                                           "games with a tree representation")
                         
    property is_const_sum:
        def __get__(self):
            return self.game.deref().IsConstSum()

    property is_perfect_recall:
        def __get__(self):
            return self.game.deref().IsPerfectRecall()

    property min_payoff:
        def __get__(self):
            return fractions.Fraction(rat_str(self.game.deref().GetMinPayoff(0)).c_str())

    property max_payoff:
        def __get__(self):
            return fractions.Fraction(rat_str(self.game.deref().GetMaxPayoff(0)).c_str())

    def _get_contingency(self, *args):
        cdef c_PureStrategyProfile *psp
        cdef Outcome outcome
        psp = new c_PureStrategyProfile(self.game.deref().NewPureStrategyProfile())
        
        
        for (pl, st) in enumerate(args):
            psp.deref().SetStrategy(self.game.deref().GetPlayer(pl+1).deref().GetStrategy(st+1))

        outcome = Outcome()
        outcome.outcome = psp.deref().GetOutcome()
        del psp
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
                if st < 0 or st >= len(self.players[pl].strategies):
                    raise IndexError, "Provided strategy index %d out of range for player %d" % (st, pl)
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


    def mixed_profile(self, rational=False):
        cdef MixedStrategyProfileDouble mspd
        cdef MixedStrategyProfileRational mspr
        cdef c_Rational dummy_rat
        if not rational:
            mspd = MixedStrategyProfileDouble()
            mspd.profile = new c_MixedStrategyProfileDouble(self.game.deref().NewMixedStrategyProfile(0.0))
            return mspd
        else:
            mspr = MixedStrategyProfileRational()
            mspr.profile = new c_MixedStrategyProfileRational(self.game.deref().NewMixedStrategyProfile(dummy_rat))
            return mspr

    def behav_profile(self, rational=False):
        cdef MixedBehavProfileDouble mbpd
        cdef MixedBehavProfileRational mbpr
        if self.is_tree:
            if not rational:
                mbpd = MixedBehavProfileDouble()
                mbpd.profile = new c_MixedBehavProfileDouble(self.game)
                return mbpd
            else:
                mbpr = MixedBehavProfileRational()
                mbpr.profile = new c_MixedBehavProfileRational(self.game)
                return mbpr
        else:
            raise UndefinedOperationError("Game must have a tree representation"\
                                      " to create a mixed behavior profile")
 
    def support_profile(self):
        return StrategySupportProfile(list(self.strategies), self)

    def num_nodes(self):
        if self.is_tree:
            return self.game.deref().NumNodes()
        return 0

    def unrestrict(self):
        return self

    def write(self, strategic=False):
        if strategic or not self.is_tree:
            return WriteGame(self.game, 1).c_str()
        else:
            return WriteGame(self.game, 0).c_str()
