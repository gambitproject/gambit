import decimal
import fractions
import warnings

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
    ctypedef struct c_GameActionRep
    ctypedef struct c_GameInfosetRep
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

    ctypedef struct c_GameAction "GameObjectPtr<GameActionRep>":
        c_GameActionRep *deref "operator->"()

    ctypedef struct c_GameInfoset "GameObjectPtr<GameInfosetRep>":
        c_GameInfosetRep *deref "operator->"()

    ctypedef struct c_GameStrategy "GameObjectPtr<GameStrategyRep>":
        c_GameStrategyRep *deref "operator->"()

    ctypedef struct c_GameStrategyRep "GameStrategyRep":
        int GetNumber()
        c_GamePlayer GetPlayer()

        cxx_string GetLabel()
        void SetLabel(cxx_string)

    ctypedef struct c_GameActionRep "GameActionRep":
        int GetNumber()
        c_GameInfoset GetInfoset()

        cxx_string GetLabel()
        void SetLabel(cxx_string)

    ctypedef struct c_GameInfosetRep "GameInfosetRep":
        int GetNumber()
        c_GamePlayer GetPlayer()

        cxx_string GetLabel()
        void SetLabel(cxx_string)

        int NumActions()
        c_GameAction GetAction(int) except +IndexError
        
        c_Number GetActionProb "GetActionProb<Number>"(int) except +IndexError
        void SetActionProb(int, cxx_string) except +IndexError 

        int NumMembers()
        c_GameNode GetMember(int) except +IndexError
        

        bint Precedes(c_GameNode)

    ctypedef struct c_GamePlayerRep "GamePlayerRep":
        c_Game GetGame()
        int GetNumber()
        int IsChance()
        
        cxx_string GetLabel()
        void SetLabel(cxx_string)
        
        int NumStrategies()
        c_GameStrategy GetStrategy(int) except +IndexError

        int NumInfosets()
        c_GameInfoset GetInfoset(int) except +IndexError
        c_GameStrategy NewStrategy()

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

        c_GameInfoset GetInfoset()

        c_GamePlayer GetPlayer()

        c_GameNode GetParent()

        int NumChildren()
        c_GameNode GetChild(int) except +IndexError
        
        c_GameOutcome GetOutcome()

        c_GameNode GetPriorSibling()
        c_GameNode GetNextSibling() 
        
        bint IsTerminal()
        bint IsSuccessorOf(c_GameNode)
        bint IsSubgameRoot()
        
        c_GameAction GetPriorAction()
        

    ctypedef struct c_GameRep "GameRep":
        int IsTree()
        
        cxx_string GetTitle()
        void SetTitle(cxx_string)

        int NumPlayers()
        c_GamePlayer GetPlayer(int) except +IndexError
        c_GamePlayer GetChance()
        c_GamePlayer NewPlayer()

        int NumOutcomes()
        c_GameOutcome GetOutcome(int) except +IndexError
        
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
        c_Game GetGame()
        int MixedProfileLength()
        double getitem_int "operator[]"(int) except +IndexError
        double getitem_Strategy "operator[]"(c_GameStrategy)
        double GetPayoff(c_GamePlayer)
        double GetStrategyValue(c_GameStrategy)
        double GetLiapValue()
    c_MixedStrategyProfileDouble *new_MixedStrategyProfileDouble "new MixedStrategyProfile<double>"(c_Game)
    void del_MixedStrategyProfileDouble "delete"(c_MixedStrategyProfileDouble *)

cdef extern from "util.h":
    c_Game ReadGame(char *) except +IOError
    cxx_string WriteGame(c_Game, int) except +IOError

    void setitem_ArrayInt(c_ArrayInt *, int, int)
    void setitem_MixedStrategyProfileDouble_int(c_MixedStrategyProfileDouble *, 
                                                int, double)
    void setitem_MixedStrategyProfileDouble_Strategy(c_MixedStrategyProfileDouble *, 
                                                     c_GameStrategy, double)


import gambit.gameiter


cdef class Collection(object):
    "Represents a collection of related objects in a game."
    def __repr__(self):   return str(list(self))

    def __getitem__(self, i):
        if isinstance(i, str):
            try:
                return self[ [ x.label for x in self ].index(i) ]
            except ValueError:
                raise IndexError("no object with label '%s'" % i)
        else:
            raise TypeError("collection indexes must be int or str, not %s" %
                             i.__class__.__name__)


######################
# Includes
######################

include "action.pxi"
include "infoset.pxi"
include "strategy.pxi"
include "player.pxi"
include "outcome.pxi"
include "node.pxi"
include "mixed.pxi"
include "game.pxi"


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
        
