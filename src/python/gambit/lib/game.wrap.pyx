cdef extern from "libgambit/libgambit.h":
    pass

cdef extern from "string":
    ctypedef struct cxx_string "string":
        char *c_str()
        cxx_string assign(char *)


cdef extern from "libgambit/game.h":
    ctypedef struct c_GameRep
    ctypedef struct c_GamePlayerRep
    ctypedef struct c_GameOutcomeRep
    
    ctypedef struct c_Game "GameObjectPtr<GameRep>":
        c_GameRep *deref "operator->"()

    ctypedef struct c_GamePlayer "GameObjectPtr<GamePlayerRep>":
        c_GamePlayerRep *deref "operator->"()

    ctypedef struct c_GameOutcome "GameObjectPtr<GameOutcomeRep>":
        c_GameOutcomeRep *deref "operator->"()

    ctypedef struct c_GamePlayerRep "GamePlayerRep":
        c_Game GetGame()
        int GetNumber()
        int IsChance()
        
        cxx_string GetLabel()
        void SetLabel(cxx_string)

    ctypedef struct c_GameOutcomeRep "GameOutcomeRep":
        c_Game GetGame()
        int GetNumber()
        
        cxx_string GetLabel()
        void SetLabel(cxx_string)

    ctypedef struct c_GameRep "GameRep":
        int IsTree()
        
        cxx_string GetTitle()
        void SetTitle(cxx_string)

        int NumPlayers()
        c_GamePlayer GetPlayer(int)
        c_GamePlayer GetChance()

        int NumOutcomes()
        c_GameOutcome GetOutcome(int)
        
        int NumNodes()

    c_Game NewTree()

cdef extern from "game.wrap.h":
    c_Game ReadGame(char *) except +IOError
    cxx_string WriteGame(c_Game, int) except +IOError


cdef class Player:
    cdef c_GamePlayer player

    def __repr__(self):
        if self.is_chance:
            return "<Player [CHANCE] in game '%s'>" % self.player.deref().GetGame().deref().GetTitle().c_str()
        return "<Player [%d] '%s' in game '%s'>" % (self.player.deref().GetNumber()-1,
                                                    self.label,
                                                    self.player.deref().GetGame().deref().GetTitle().c_str())
    
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

    
cdef class Players:
    cdef c_Game game

    def __repr__(self):
        return "<Players of game '%s'>" % self.game.deref().GetTitle().c_str()

    def __len__(self):
        return self.game.deref().NumPlayers()

    def __getitem__(self, pl):
        cdef Player p
        if pl < 0 or pl >= len(self):
            raise IndexError("no player with index '%s'" % pl)
        p = Player()
        p.player = self.game.deref().GetPlayer(pl+1)
        return p

    property chance:
        def __get__(self):
            cdef Player p
            p = Player()
            p.player = self.game.deref().GetChance()
            return p

cdef class Outcome:
    cdef c_GameOutcome outcome

    def __repr__(self):
        return "<Outcome [%d] '%s' in game '%s'>" % (self.outcome.deref().GetNumber()-1,
                                                     self.label,
                                                     self.outcome.deref().GetGame().deref().GetTitle().c_str())
    
    property label:
        def __get__(self):
            return self.outcome.deref().GetLabel().c_str()
        def __set__(self, char *value):
            cdef cxx_string s
            s.assign(value)
            self.outcome.deref().SetLabel(s)


cdef class Outcomes:
    cdef c_Game game

    def __repr__(self):
        return "<Outcomes of game '%s'>" % self.game.deref().GetTitle().c_str()

    def __len__(self):
        return self.game.deref().NumOutcomes()

    def __getitem__(self, outc):
        cdef Outcome c
        if outc < 0 or outc >= len(self):
            raise IndexError("no outcome with index '%s'" % outc)
        c = Outcome()
        c.outcome = self.game.deref().GetOutcome(outc+1)
        return c

cdef class Game:
    cdef c_Game game

    def __str__(self):
        return "<Game '%s'>" % self.title

    def __repr__(self):
        return self.write()

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


    def num_nodes(self):
        return self.game.deref().NumNodes()

    def write(self, strategic=False):
        if strategic or not self.is_tree:
            return WriteGame(self.game, 1).c_str()
        else:
            return WriteGame(self.game, 0).c_str()


def new_tree():
    cdef Game g
    g = Game()
    g.game = NewTree()
    return g

def read_game(char *fn):
    cdef Game g
    g = Game()
    try:
        g.game = ReadGame(fn)
    except IOError:
        raise IOError("Unable to read game from file '%s'" % fn)
    return g
        
