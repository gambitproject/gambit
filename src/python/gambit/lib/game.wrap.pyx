import decimal
import fractions

cdef extern from "libgambit/libgambit.h":
    pass

cdef extern from "string":
    ctypedef struct cxx_string "string":
        char *c_str()
        cxx_string assign(char *)

cdef extern from "libgambit/number.h":
    ctypedef struct c_Number "Number":
        cxx_string as_string "operator const string &"()
     
cdef extern from "libgambit/array.h":
    ctypedef struct c_ArrayInt "Array<int>":
        int getitem "operator[]"(int)
    c_ArrayInt *new_ArrayInt "new Array<int>"(int)
    void del_ArrayInt "delete"(c_ArrayInt *)

cdef extern from "libgambit/game.h":
    ctypedef struct c_GameRep
    ctypedef struct c_GameStrategyRep
    ctypedef struct c_GamePlayerRep
    ctypedef struct c_GameOutcomeRep
    ctypedef struct c_GameNodeRep
    
    ctypedef struct c_Game "GameObjectPtr<GameRep>":
        c_GameRep *deref "operator->"()

    ctypedef struct c_GamePlayer "GameObjectPtr<GamePlayerRep>":
        c_GamePlayerRep *deref "operator->"()

    ctypedef struct c_GameOutcome "GameObjectPtr<GameOutcomeRep>":
        c_GameOutcomeRep *deref "operator->"()
 
    ctypedef struct c_GameNode "GameObjectPtr<GameNodeRep>":
        c_GameNodeRep *deref "operator->"()

    ctypedef struct c_GameStrategy "GameObjectPtr<GameStrategyRep>":
        c_GameStrategyRep *deref "operator->"()

    ctypedef struct c_GameStrategyRep "GameStrategyRep":
        int GetNumber()
        c_GamePlayer GetPlayer()

        cxx_string GetLabel()
        void SetLabel(cxx_string)

    ctypedef struct c_GamePlayerRep "GamePlayerRep":
        c_Game GetGame()
        int GetNumber()
        int IsChance()
        
        cxx_string GetLabel()
        void SetLabel(cxx_string)
        
        int NumStrategies()
        c_GameStrategy GetStrategy(int) except +IndexError

    ctypedef struct c_GameOutcomeRep "GameOutcomeRep":
        c_Game GetGame()
        int GetNumber()
        
        cxx_string GetLabel()
        void SetLabel(cxx_string)
     
        c_Number GetPayoffNumber "GetPayoff<Number>"(int) except +IndexError
        void SetPayoff(int, cxx_string) except +IndexError

    ctypedef struct c_GameNodeRep "GameNodeRep":
        c_Game GetGame()
        int GetNumber()

        cxx_string GetLabel()
        void SetLabel(cxx_string)

    ctypedef struct c_GameRep "GameRep":
        int IsTree()
        
        cxx_string GetTitle()
        void SetTitle(cxx_string)

        int NumPlayers()
        c_GamePlayer GetPlayer(int) except +IndexError
        c_GamePlayer GetChance()
        c_GamePlayer NewPlayer()

        int NumOutcomes()
        c_GameOutcome GetOutcome(int)
        
        int NumNodes()
        c_GameNode GetRoot()

    ctypedef struct c_PureStrategyProfile "PureStrategyProfile":
        c_GameStrategy GetStrategy(c_GamePlayer)
        void SetStrategy(c_GameStrategy)

        c_GameOutcome GetOutcome()
        void SetOutcome(c_GameOutcome)

    c_PureStrategyProfile *new_PureStrategyProfile "new PureStrategyProfile"(c_Game)
    void del_PureStrategyProfile "delete"(c_PureStrategyProfile *)

    c_Game NewTree()
    c_Game NewTable(c_ArrayInt *)

cdef extern from "libgambit/mixed.h":
    ctypedef struct c_MixedStrategyProfileDouble "MixedStrategyProfile<double>":
        int MixedProfileLength()
        double getitem_int "operator[]"(int) except +IndexError
        double getitem_Strategy "operator[]"(c_GameStrategy)
        double GetPayoff(c_GamePlayer)
        double GetStrategyValue(c_GameStrategy)
        double GetLiapValue()
    c_MixedStrategyProfileDouble *new_MixedStrategyProfileDouble "new MixedStrategyProfile<double>"(c_Game)
    void del_MixedStrategyProfileDouble "delete"(c_MixedStrategyProfileDouble *)

cdef extern from "game.wrap.h":
    c_Game ReadGame(char *) except +IOError
    cxx_string WriteGame(c_Game, int) except +IOError

    void setitem_ArrayInt(c_ArrayInt *, int, int)
    void setitem_MixedStrategyProfileDouble_int(c_MixedStrategyProfileDouble *, 
                                                int, double)
    void setitem_MixedStrategyProfileDouble_Strategy(c_MixedStrategyProfileDouble *, 
                                                     c_GameStrategy, double)


import gambit.gameiter


######################
# Includes
######################

include "strategy.pyx"
include "strategies.pyx"
    
include "player.pyx"
include "players.pyx"
    
include "outcome.pyx"
include "outcomes.pyx"

include "node.pyx"

include "mixed.pyx"

include "game.pyx"


def new_tree():
    cdef Game g
    g = Game()
    g.game = NewTree()
    return g

def new_table(dim):
    cdef Game g
    cdef c_ArrayInt *d
    d = new_ArrayInt(len(dim))
    for i in range(1, len(dim)+1):
        setitem_ArrayInt(d, i, dim[i-1])
    g = Game()
    g.game = NewTable(d)
    del_ArrayInt(d)
    return g

def read_game(char *fn):
    cdef Game g
    g = Game()
    try:
        g.game = ReadGame(fn)
    except IOError:
        raise IOError("Unable to read game from file '%s'" % fn)
    return g
        
