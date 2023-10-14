from libcpp cimport bool
from libcpp.string cimport string
from libcpp.memory cimport shared_ptr


cdef extern from "gambit.h":
     # We don't wrap anything from the basic header, but it ensures
     # it gets included in the output
     pass


cdef extern from "core/rational.h":
    cdef cppclass c_Rational "Rational":
        c_Rational()
    string rat_str "lexical_cast<std::string>"(c_Rational)
    c_Rational str_rat "lexical_cast<Rational>"(string)


cdef extern from "games/number.h":
    cdef cppclass c_Number "Number":
        c_Number()
        c_Number(string)
        string as_string "operator const string &"()


cdef extern from "core/array.h":
     cdef cppclass Array[T]:
          T getitem "operator[]"(int) except +
          int Length()
          Array()
          Array(int)

cdef extern from "core/list.h":
     cdef cppclass c_List "List"[T]:
          T & getitem "operator[]"(int) except +
          int Length()
          void push_back(T)

cdef extern from "games/game.h":
     cdef cppclass c_GameRep "GameRep"
     cdef cppclass c_GameStrategyRep "GameStrategyRep"
     cdef cppclass c_GameActionRep "GameActionRep"
     cdef cppclass c_GameInfosetRep "GameInfosetRep"
     cdef cppclass c_GamePlayerRep "GamePlayerRep"
     cdef cppclass c_GameOutcomeRep "GameOutcomeRep"
     cdef cppclass c_GameNodeRep "GameNodeRep"

     cdef cppclass c_Game "GameObjectPtr<GameRep>":
          c_GameRep *deref "operator->"() except +RuntimeError

     cdef cppclass c_GamePlayer "GameObjectPtr<GamePlayerRep>":
          bool operator !=(c_GamePlayer)
          c_GamePlayerRep *deref "operator->"() except +RuntimeError

     cdef cppclass c_GameOutcome "GameObjectPtr<GameOutcomeRep>":
          bool operator==(c_GameOutcome)
          bool operator !=(c_GameOutcome)
          c_GameOutcomeRep *deref "operator->"() except +RuntimeError

     cdef cppclass c_GameNode "GameObjectPtr<GameNodeRep>":
          bool operator !=(c_GameNode)
          c_GameNodeRep *deref "operator->"() except +RuntimeError

     cdef cppclass c_GameAction "GameObjectPtr<GameActionRep>":
          bool operator !=(c_GameAction)
          c_GameActionRep *deref "operator->"() except +RuntimeError

     cdef cppclass c_GameInfoset "GameObjectPtr<GameInfosetRep>":
          bool operator !=(c_GameInfoset)
          c_GameInfosetRep *deref "operator->"() except +RuntimeError

     cdef cppclass c_GameStrategy "GameObjectPtr<GameStrategyRep>":
          c_GameStrategyRep *deref "operator->"() except +RuntimeError

     cdef cppclass c_PureStrategyProfile "PureStrategyProfile":
          c_PureStrategyProfileRep *deref "operator->"()
          c_PureStrategyProfile(c_PureStrategyProfile)

     cdef cppclass c_PureBehaviorProfile "PureBehaviorProfile":
          c_PureBehaviorProfile(c_Game)

     cdef cppclass c_GameStrategyRep "GameStrategyRep":
          int GetNumber()
          int GetId()
          c_GamePlayer GetPlayer()

          string GetLabel()
          void SetLabel(string)

          void DeleteStrategy()

     cdef cppclass c_GameActionRep "GameActionRep":
          int GetNumber()
          c_GameInfoset GetInfoset()
          bint Precedes(c_GameNode)
          void DeleteAction() except +ValueError

          string GetLabel()
          void SetLabel(string)

     cdef cppclass c_GameInfosetRep "GameInfosetRep":
          int GetNumber()
          c_Game GetGame()
          c_GamePlayer GetPlayer()
          void SetPlayer(c_GamePlayer) except +

          string GetLabel()
          void SetLabel(string)

          int NumActions()
          c_GameAction GetAction(int) except +IndexError
          c_GameAction InsertAction(c_GameAction) except +ValueError

          c_Number GetActionProb(int) except +IndexError
          void SetActionProb(int, c_Number) except +IndexError

          int NumMembers()
          c_GameNode GetMember(int) except +IndexError

          void Reveal(c_GamePlayer)
          bint IsChanceInfoset()
          bint Precedes(c_GameNode)

     cdef cppclass c_GamePlayerRep "GamePlayerRep":
          c_Game GetGame()
          int GetNumber()
          int IsChance()

          string GetLabel()
          void SetLabel(string)

          int NumStrategies()
          c_GameStrategy GetStrategy(int) except +IndexError

          int NumInfosets()
          c_GameInfoset GetInfoset(int) except +IndexError
          c_GameStrategy NewStrategy()

     cdef cppclass c_GameOutcomeRep "GameOutcomeRep":
          c_Game GetGame()
          int GetNumber()

          string GetLabel()
          void SetLabel(string)

          c_Number GetPayoff(c_GamePlayer) except +IndexError
          void SetPayoff(c_GamePlayer, c_Number) except +IndexError

     cdef cppclass c_GameNodeRep "GameNodeRep":
          c_Game GetGame()
          int GetNumber()

          string GetLabel()
          void SetLabel(string)

          c_GameInfoset GetInfoset()
          void SetInfoset(c_GameInfoset) except +ValueError
          c_GameInfoset LeaveInfoset()
          c_GamePlayer GetPlayer()
          c_GameNode GetParent()
          int NumChildren()
          c_GameNode GetChild(int) except +IndexError
          c_GameOutcome GetOutcome()
          void SetOutcome(c_GameOutcome)
          c_GameNode GetPriorSibling()
          c_GameNode GetNextSibling()
          bint IsTerminal()
          bint IsSuccessorOf(c_GameNode)
          bint IsSubgameRoot()
          c_GameAction GetPriorAction()

          c_GameInfoset AppendMove(c_GamePlayer, int) except +ValueError
          c_GameInfoset AppendMove(c_GameInfoset) except +ValueError
          c_GameInfoset InsertMove(c_GamePlayer, int) except +ValueError
          c_GameInfoset InsertMove(c_GameInfoset) except +ValueError
          void DeleteParent()
          void DeleteTree()
          void CopyTree(c_GameNode) except +ValueError
          void MoveTree(c_GameNode) except +ValueError

     cdef cppclass c_GameRep "GameRep":
          int IsTree()

          string GetTitle()
          void SetTitle(string)

          string GetComment()
          void SetComment(string)

          int NumPlayers()
          c_GamePlayer GetPlayer(int) except +IndexError
          c_GamePlayer GetChance()
          c_GamePlayer NewPlayer()

          int NumOutcomes()
          c_GameOutcome GetOutcome(int) except +IndexError
          c_GameOutcome NewOutcome()
          void DeleteOutcome(c_GameOutcome)

          int NumNodes()
          c_GameNode GetRoot()

          c_GameStrategy GetStrategy(int) except +IndexError
          int MixedProfileLength()

          c_GameInfoset GetInfoset(int) except +IndexError
          Array[int] NumInfosets()

          c_GameAction GetAction(int) except +IndexError
          int BehavProfileLength()

          bool IsConstSum()
          c_Rational GetMinPayoff(int)
          c_Rational GetMaxPayoff(int)
          bool IsPerfectRecall()

          c_Game SetChanceProbs(c_GameInfoset, Array[c_Number]) except +

          c_PureStrategyProfile NewPureStrategyProfile()
          c_MixedStrategyProfileDouble NewMixedStrategyProfile(double)
          c_MixedStrategyProfileRational NewMixedStrategyProfile(c_Rational)

     cdef cppclass c_PureStrategyProfileRep "PureStrategyProfileRep":
          c_GameStrategy GetStrategy(c_GamePlayer)
          void SetStrategy(c_GameStrategy)

          c_GameOutcome GetOutcome()
          void SetOutcome(c_GameOutcome)

          c_Rational GetPayoff(c_GamePlayer)

     c_Game NewTree()
     c_Game NewTable(Array[int] *)


cdef extern from "games/mixed.h":
     cdef cppclass c_MixedStrategyProfileDouble "MixedStrategyProfile<double>":
          bool operator==(c_MixedStrategyProfileDouble)
          bool operator!=(c_MixedStrategyProfileDouble)
          c_Game GetGame()
          int MixedProfileLength()
          c_StrategySupportProfile GetSupport()
          c_MixedStrategyProfileDouble Normalize()
          void Randomize() except +TypeError
          void Randomize(int)
          double getitem_strategy "operator[]"(c_GameStrategy) except +IndexError
          double GetPayoff(c_GamePlayer)
          double GetPayoff(c_GameStrategy)
          double GetRegret(c_GameStrategy)
          double GetPayoffDeriv(int, c_GameStrategy, c_GameStrategy)
          double GetLiapValue()
          c_MixedStrategyProfileDouble ToFullSupport()
          c_MixedStrategyProfileDouble(c_MixedStrategyProfileDouble)

     cdef cppclass c_MixedStrategyProfileRational "MixedStrategyProfile<Rational>":
          bool operator==(c_MixedStrategyProfileRational)
          bool operator!=(c_MixedStrategyProfileRational)
          c_Game GetGame()
          int MixedProfileLength()
          c_StrategySupportProfile GetSupport()
          c_MixedStrategyProfileRational Normalize()
          void Randomize()
          void Randomize(int)
          c_Rational getitem_strategy "operator[]"(c_GameStrategy) except +IndexError
          c_Rational GetPayoff(c_GamePlayer)
          c_Rational GetPayoff(c_GameStrategy)
          c_Rational GetRegret(c_GameStrategy)
          c_Rational GetPayoffDeriv(int, c_GameStrategy, c_GameStrategy)
          c_Rational GetLiapValue()
          c_MixedStrategyProfileRational ToFullSupport()
          c_MixedStrategyProfileRational(c_MixedStrategyProfileRational)

cdef extern from "games/behav.h":
     cdef cppclass c_MixedBehaviorProfileDouble "MixedBehaviorProfile<double>":
          bool operator==(c_MixedBehaviorProfileDouble)
          bool operator!=(c_MixedBehaviorProfileDouble)
          c_Game GetGame()
          int Length()
          bool IsDefinedAt(c_GameInfoset)
          c_MixedBehaviorProfileDouble Normalize()
          void Randomize() except +TypeError
          void Randomize(int)
          double getitem "operator[]"(int) except +IndexError
          double getaction "operator()"(c_GameAction) except +IndexError
          double GetPayoff(int)
          double GetBeliefProb(c_GameNode)
          double GetRealizProb(c_GameInfoset)
          double GetPayoff(c_GameInfoset)
          double GetPayoff(c_GamePlayer, c_GameNode)
          double GetPayoff(c_GameAction)
          double GetRegret(c_GameAction)
          double GetLiapValue()
          c_MixedStrategyProfileDouble ToMixedProfile()
          c_MixedBehaviorProfileDouble(c_MixedStrategyProfileDouble) except +NotImplementedError
          c_MixedBehaviorProfileDouble(c_Game)
          c_MixedBehaviorProfileDouble(c_MixedBehaviorProfileDouble)

     cdef cppclass c_MixedBehaviorProfileRational "MixedBehaviorProfile<Rational>":
          bool operator==(c_MixedBehaviorProfileRational)
          bool operator!=(c_MixedBehaviorProfileRational)
          c_Game GetGame()
          int Length()
          bool IsDefinedAt(c_GameInfoset)
          c_MixedBehaviorProfileRational Normalize()
          void Randomize()
          void Randomize(int)
          c_Rational getitem "operator[]"(int) except +IndexError
          c_Rational getaction "operator()"(c_GameAction) except +IndexError
          c_Rational GetPayoff(int)
          c_Rational GetBeliefProb(c_GameNode)
          c_Rational GetRealizProb(c_GameInfoset)
          c_Rational GetPayoff(c_GameInfoset)
          c_Rational GetPayoff(c_GamePlayer, c_GameNode)
          c_Rational GetPayoff(c_GameAction)
          c_Rational GetRegret(c_GameAction)
          c_Rational GetLiapValue()
          c_MixedStrategyProfileRational ToMixedProfile()
          c_MixedBehaviorProfileRational(c_MixedStrategyProfileRational) except +NotImplementedError
          c_MixedBehaviorProfileRational(c_Game)
          c_MixedBehaviorProfileRational(c_MixedBehaviorProfileRational)


cdef extern from "games/stratspt.h":
     cdef cppclass c_StrategySupportProfile "StrategySupportProfile":
          c_StrategySupportProfile(c_Game)
          c_StrategySupportProfile(c_StrategySupportProfile)
          bool operator ==(c_StrategySupportProfile)
          bool operator !=(c_StrategySupportProfile)
          c_Game GetGame()
          Array[int] NumStrategies()
          int MixedProfileLength()
          int GetIndex(c_GameStrategy)
          int NumStrategiesPlayer "NumStrategies"(int) except +IndexError
          bool IsSubsetOf(c_StrategySupportProfile)
          bool RemoveStrategy(c_GameStrategy)
          c_GameStrategy GetStrategy(int, int) except +IndexError
          bool Contains(c_GameStrategy)
          c_StrategySupportProfile Undominated(bool, bool)
          c_MixedStrategyProfileDouble NewMixedStrategyProfileDouble "NewMixedStrategyProfile<double>"()
          c_MixedStrategyProfileRational NewMixedStrategyProfileRational "NewMixedStrategyProfile<Rational>"()


cdef extern from "games/behavspt.h":
     cdef cppclass c_BehaviorSupportProfile "BehaviorSupportProfile":
          c_BehaviorSupportProfile(c_Game)


cdef extern from "util.h":
     c_Game ReadGame(char *) except +IOError
     c_Game ParseGame(char *) except +IOError
     string WriteGame(c_Game, string) except +IOError
     string WriteGame(c_StrategySupportProfile) except +IOError

     c_Rational to_rational(char *)

     void setitem_array_int "setitem"(Array[int] *, int, int)
     void setitem_array_number "setitem"(Array[c_Number], int, c_Number)

     void setitem_mspd_int "setitem"(c_MixedStrategyProfileDouble, int, double)
     void setitem_mspd_strategy "setitem"(c_MixedStrategyProfileDouble,
                                          c_GameStrategy, double)
     void setitem_mspr_int "setitem"(c_MixedStrategyProfileRational, int, c_Rational)
     void setitem_mspr_strategy "setitem"(c_MixedStrategyProfileRational,
                                          c_GameStrategy, c_Rational)

     void setitem_mbpd_int "setitem"(c_MixedBehaviorProfileDouble, int, double)
     void setitem_mbpd_action "setitem"(c_MixedBehaviorProfileDouble,
                                        c_GameAction, double)
     void setitem_mbpr_int "setitem"(c_MixedBehaviorProfileRational, int, c_Rational)
     void setitem_mbpr_action "setitem"(c_MixedBehaviorProfileRational,
                                        c_GameAction, c_Rational)

     shared_ptr[c_MixedStrategyProfileDouble] copyitem_list_mspd "sharedcopyitem"(c_List[c_MixedStrategyProfileDouble], int)
     shared_ptr[c_MixedStrategyProfileRational] copyitem_list_mspr "sharedcopyitem"(c_List[c_MixedStrategyProfileRational], int)
     shared_ptr[c_MixedBehaviorProfileDouble] copyitem_list_mbpd "sharedcopyitem"(c_List[c_MixedBehaviorProfileDouble], int)
     shared_ptr[c_MixedBehaviorProfileRational] copyitem_list_mbpr "sharedcopyitem"(c_List[c_MixedBehaviorProfileRational], int)
     shared_ptr[c_LogitQREMixedStrategyProfile] copyitem_list_qrem "sharedcopyitem"(c_List[c_LogitQREMixedStrategyProfile], int)


cdef extern from "solvers/enumpure/enumpure.h":
    c_List[c_MixedStrategyProfileRational] EnumPureStrategySolve(c_Game) except +RuntimeError
    c_List[c_MixedBehaviorProfileRational] EnumPureAgentSolve(c_Game) except +RuntimeError

cdef extern from "solvers/enummixed/enummixed.h":
    c_List[c_MixedStrategyProfileDouble] EnumMixedStrategySolveDouble(c_Game) except +RuntimeError
    c_List[c_MixedStrategyProfileRational] EnumMixedStrategySolveRational(c_Game) except +RuntimeError
    c_List[c_MixedStrategyProfileRational] EnumMixedStrategySolveLrs(c_Game) except +RuntimeError

cdef extern from "solvers/lcp/lcp.h":
    c_List[c_MixedStrategyProfileDouble] LcpStrategySolveDouble(c_Game, int p_stopAfter, int p_maxDepth) except +RuntimeError
    c_List[c_MixedStrategyProfileRational] LcpStrategySolveRational(c_Game, int p_stopAfter, int p_maxDepth) except +RuntimeError
    c_List[c_MixedBehaviorProfileDouble] LcpBehaviorSolveDouble(c_Game, int p_stopAfter, int p_maxDepth) except +RuntimeError
    c_List[c_MixedBehaviorProfileRational] LcpBehaviorSolveRational(c_Game, int p_stopAfter, int p_maxDepth) except +RuntimeError

cdef extern from "solvers/lp/nfglp.h":
    c_List[c_MixedStrategyProfileDouble] LpStrategySolveDouble(c_Game) except +RuntimeError
    c_List[c_MixedStrategyProfileRational] LpStrategySolveRational(c_Game) except +RuntimeError

cdef extern from "solvers/lp/efglp.h":
    c_List[c_MixedBehaviorProfileDouble] LpBehaviorSolveDouble(c_Game) except +RuntimeError
    c_List[c_MixedBehaviorProfileRational] LpBehaviorSolveRational(c_Game) except +RuntimeError

cdef extern from "solvers/liap/liap.h":
    c_List[c_MixedStrategyProfileDouble] LiapStrategySolve(c_Game, int p_maxitsN) except +RuntimeError
    c_List[c_MixedBehaviorProfileDouble] LiapBehaviorSolve(c_Game, int p_maxitsN) except +RuntimeError

cdef extern from "solvers/simpdiv/simpdiv.h":
    c_List[c_MixedStrategyProfileRational] SimpdivStrategySolve(c_Game,
                                                                int p_gridResize,
                                                                int p_leashLength) except +RuntimeError

cdef extern from "solvers/ipa/ipa.h":
    c_List[c_MixedStrategyProfileDouble] IPAStrategySolve(c_Game) except +RuntimeError

cdef extern from "solvers/gnm/gnm.h":
    c_List[c_MixedStrategyProfileDouble] GNMStrategySolve(c_Game) except +RuntimeError

cdef extern from "solvers/logit/nfglogit.h":
    c_List[c_MixedStrategyProfileDouble] LogitStrategySolve(c_Game) except +RuntimeError

cdef extern from "solvers/logit/efglogit.h":
    c_List[c_MixedBehaviorProfileDouble] LogitBehaviorSolve(c_Game) except +RuntimeError

cdef extern from "solvers/logit/nfglogit.h":
     cdef cppclass c_LogitQREMixedStrategyProfile "LogitQREMixedStrategyProfile":
          c_LogitQREMixedStrategyProfile(c_Game)
          c_LogitQREMixedStrategyProfile(c_LogitQREMixedStrategyProfile)
          c_Game GetGame()
          c_MixedStrategyProfileDouble GetProfile()
          double GetLambda()
          double GetLogLike()
          int MixedProfileLength()
          double getitem "operator[]"(int) except +IndexError

     cdef cppclass c_StrategicQREEstimator "StrategicQREEstimator":
          c_StrategicQREEstimator()
          c_LogitQREMixedStrategyProfile Estimate(c_LogitQREMixedStrategyProfile,
                                                  c_MixedStrategyProfileDouble,
                                                  double, double, double) except +RuntimeError

cdef extern from "nash.h":
    shared_ptr[c_LogitQREMixedStrategyProfile] _logit_estimate "logit_estimate"(shared_ptr[c_MixedStrategyProfileDouble])
    shared_ptr[c_LogitQREMixedStrategyProfile] _logit_atlambda "logit_atlambda"(c_Game, double)
    c_List[c_LogitQREMixedStrategyProfile] _logit_principal_branch "logit_principal_branch"(c_Game, double)

