from libcpp cimport bool
from libcpp.string cimport string
from libcpp.memory cimport shared_ptr


cdef extern from "gambit.h":
    # We don't wrap anything from the basic header, but it ensures
    # it gets included in the output
    pass


cdef extern from "core/rational.h":
    cdef cppclass c_Rational "Rational":
        c_Rational() except +
    string rat_str "lexical_cast<std::string>"(c_Rational) except +
    c_Rational str_rat "lexical_cast<Rational>"(string) except +


cdef extern from "games/number.h":
    cdef cppclass c_Number "Number":
        c_Number() except +
        c_Number(string) except +
        string as_string "operator const string &"() except +


cdef extern from "core/array.h":
    cdef cppclass Array[T]:
        T getitem "operator[]"(int) except +
        int Length() except +
        Array() except +
        Array(int) except +

cdef extern from "core/list.h":
    cdef cppclass c_List "List"[T]:
        T & getitem "operator[]"(int) except +
        int Length() except +
        void push_back(T) except +

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
        bool operator !=(c_GamePlayer) except +
        c_GamePlayerRep *deref "operator->"() except +RuntimeError

    cdef cppclass c_GameOutcome "GameObjectPtr<GameOutcomeRep>":
        bool operator==(c_GameOutcome) except +
        bool operator !=(c_GameOutcome) except +
        c_GameOutcomeRep *deref "operator->"() except +RuntimeError

    cdef cppclass c_GameNode "GameObjectPtr<GameNodeRep>":
        bool operator !=(c_GameNode) except +
        c_GameNodeRep *deref "operator->"() except +RuntimeError

    cdef cppclass c_GameAction "GameObjectPtr<GameActionRep>":
        bool operator !=(c_GameAction) except +
        c_GameActionRep *deref "operator->"() except +RuntimeError

    cdef cppclass c_GameInfoset "GameObjectPtr<GameInfosetRep>":
        bool operator !=(c_GameInfoset) except +
        c_GameInfosetRep *deref "operator->"() except +RuntimeError

    cdef cppclass c_GameStrategy "GameObjectPtr<GameStrategyRep>":
        c_GameStrategyRep *deref "operator->"() except +RuntimeError

    cdef cppclass c_PureStrategyProfile "PureStrategyProfile":
        c_PureStrategyProfileRep *deref "operator->"() except +
        c_PureStrategyProfile(c_PureStrategyProfile) except +

    cdef cppclass c_PureBehaviorProfile "PureBehaviorProfile":
        c_PureBehaviorProfile(c_Game) except +

    cdef cppclass c_GameStrategyRep "GameStrategyRep":
        int GetNumber() except +
        int GetId() except +
        c_GamePlayer GetPlayer() except +
        string GetLabel() except +
        void SetLabel(string) except +
        void DeleteStrategy() except +

    cdef cppclass c_GameActionRep "GameActionRep":
        int GetNumber() except +
        c_GameInfoset GetInfoset() except +
        bint Precedes(c_GameNode) except +
        void DeleteAction() except +ValueError

        string GetLabel() except +
        void SetLabel(string) except +

    cdef cppclass c_GameInfosetRep "GameInfosetRep":
        int GetNumber() except +
        c_Game GetGame() except +
        c_GamePlayer GetPlayer() except +
        void SetPlayer(c_GamePlayer) except +

        string GetLabel() except +
        void SetLabel(string) except +

        int NumActions() except +
        c_GameAction GetAction(int) except +IndexError
        c_GameAction InsertAction(c_GameAction) except +ValueError

        c_Number GetActionProb(int) except +IndexError
        void SetActionProb(int, c_Number) except +IndexError

        int NumMembers() except +
        c_GameNode GetMember(int) except +IndexError

        void Reveal(c_GamePlayer) except +
        bint IsChanceInfoset() except +
        bint Precedes(c_GameNode) except +

    cdef cppclass c_GamePlayerRep "GamePlayerRep":
        c_Game GetGame() except +
        int GetNumber() except +
        int IsChance() except +

        string GetLabel() except +
        void SetLabel(string) except +

        int NumStrategies() except +
        c_GameStrategy GetStrategy(int) except +IndexError

        int NumInfosets() except +
        c_GameInfoset GetInfoset(int) except +IndexError
        c_GameStrategy NewStrategy() except +

    cdef cppclass c_GameOutcomeRep "GameOutcomeRep":
        c_Game GetGame() except +
        int GetNumber() except +

        string GetLabel() except +
        void SetLabel(string) except +

        c_Number GetPayoff(c_GamePlayer) except +IndexError
        void SetPayoff(c_GamePlayer, c_Number) except +IndexError

    cdef cppclass c_GameNodeRep "GameNodeRep":
        c_Game GetGame() except +
        int GetNumber() except +

        string GetLabel() except +
        void SetLabel(string) except +

        c_GameInfoset GetInfoset() except +
        void SetInfoset(c_GameInfoset) except +ValueError
        c_GameInfoset LeaveInfoset() except +
        c_GamePlayer GetPlayer() except +
        c_GameNode GetParent() except +
        int NumChildren() except +
        c_GameNode GetChild(int) except +IndexError
        c_GameOutcome GetOutcome() except +
        void SetOutcome(c_GameOutcome) except +
        c_GameNode GetPriorSibling() except +
        c_GameNode GetNextSibling() except +
        bint IsTerminal() except +
        bint IsSuccessorOf(c_GameNode) except +
        bint IsSubgameRoot() except +
        c_GameAction GetPriorAction() except +

        c_GameInfoset AppendMove(c_GamePlayer, int) except +ValueError
        c_GameInfoset AppendMove(c_GameInfoset) except +ValueError
        c_GameInfoset InsertMove(c_GamePlayer, int) except +ValueError
        c_GameInfoset InsertMove(c_GameInfoset) except +ValueError
        void DeleteParent() except +
        void DeleteTree() except +
        void CopyTree(c_GameNode) except +ValueError
        void MoveTree(c_GameNode) except +ValueError

    cdef cppclass c_GameRep "GameRep":
        int IsTree() except +

        string GetTitle() except +
        void SetTitle(string) except +

        string GetComment() except +
        void SetComment(string) except +

        int NumPlayers() except +
        c_GamePlayer GetPlayer(int) except +IndexError
        c_GamePlayer GetChance() except +
        c_GamePlayer NewPlayer() except +

        int NumOutcomes() except +
        c_GameOutcome GetOutcome(int) except +IndexError
        c_GameOutcome NewOutcome() except +
        void DeleteOutcome(c_GameOutcome) except +

        int NumNodes() except +
        c_GameNode GetRoot() except +

        c_GameStrategy GetStrategy(int) except +IndexError
        int MixedProfileLength() except +

        c_GameInfoset GetInfoset(int) except +IndexError
        Array[int] NumInfosets() except +

        c_GameAction GetAction(int) except +IndexError
        int BehavProfileLength() except +

        bool IsConstSum() except +
        c_Rational GetMinPayoff(int) except +
        c_Rational GetMaxPayoff(int) except +
        bool IsPerfectRecall() except +

        c_Game SetChanceProbs(c_GameInfoset, Array[c_Number]) except +

        c_PureStrategyProfile NewPureStrategyProfile()  # except + doesn't compile
        c_MixedStrategyProfileDouble NewMixedStrategyProfile(double)  # except + doesn't compile
        c_MixedStrategyProfileRational NewMixedStrategyProfile(
                c_Rational
        )  # except + doesn't compile

    c_Game NewTree() except +
    c_Game NewTable(Array[int] *) except +


cdef extern from "games/stratpure.h":
    cdef cppclass c_PureStrategyProfileRep "PureStrategyProfileRep":
        c_GameStrategy GetStrategy(c_GamePlayer) except +
        void SetStrategy(c_GameStrategy) except +

        c_GameOutcome GetOutcome() except +
        void SetOutcome(c_GameOutcome) except +

        c_Rational GetPayoff(c_GamePlayer) except +


cdef extern from "games/stratmixed.h":
    cdef cppclass c_MixedStrategyProfileDouble "MixedStrategyProfile<double>":
        bool operator==(c_MixedStrategyProfileDouble) except +
        bool operator!=(c_MixedStrategyProfileDouble) except +
        c_Game GetGame() except +
        bool IsInvalidated()
        int MixedProfileLength() except +
        c_StrategySupportProfile GetSupport() except +
        c_MixedStrategyProfileDouble Normalize()  # except + doesn't compile
        double getitem_strategy "operator[]"(c_GameStrategy) except +IndexError
        double GetPayoff(c_GamePlayer) except +
        double GetPayoff(c_GameStrategy) except +
        double GetRegret(c_GameStrategy) except +
        double GetRegret(c_GamePlayer) except +
        double GetMaxRegret() except +
        double GetPayoffDeriv(int, c_GameStrategy, c_GameStrategy) except +
        double GetLiapValue() except +
        c_MixedStrategyProfileDouble ToFullSupport() except +
        c_MixedStrategyProfileDouble(c_MixedStrategyProfileDouble) except +

    cdef cppclass c_MixedStrategyProfileRational "MixedStrategyProfile<Rational>":
        bool operator==(c_MixedStrategyProfileRational) except +
        bool operator!=(c_MixedStrategyProfileRational) except +
        c_Game GetGame() except +
        bool IsInvalidated()
        int MixedProfileLength() except +
        c_StrategySupportProfile GetSupport() except +
        c_MixedStrategyProfileRational Normalize()  # except + doesn't compile
        c_Rational getitem_strategy "operator[]"(c_GameStrategy) except +IndexError
        c_Rational GetPayoff(c_GamePlayer) except +
        c_Rational GetPayoff(c_GameStrategy) except +
        c_Rational GetRegret(c_GameStrategy) except +
        c_Rational GetRegret(c_GamePlayer) except +
        c_Rational GetMaxRegret() except +
        c_Rational GetPayoffDeriv(int, c_GameStrategy, c_GameStrategy) except +
        c_Rational GetLiapValue() except +
        c_MixedStrategyProfileRational ToFullSupport() except +
        c_MixedStrategyProfileRational(c_MixedStrategyProfileRational) except +

cdef extern from "games/behavmixed.h":
    cdef cppclass c_MixedBehaviorProfileDouble "MixedBehaviorProfile<double>":
        bool operator==(c_MixedBehaviorProfileDouble) except +
        bool operator!=(c_MixedBehaviorProfileDouble) except +
        c_Game GetGame() except +
        bool IsInvalidated()
        int BehaviorProfileLength() except +
        bool IsDefinedAt(c_GameInfoset) except +
        c_MixedBehaviorProfileDouble Normalize()  # except + doesn't compile
        double getitem "operator[]"(int) except +IndexError
        double getaction "operator[]"(c_GameAction) except +IndexError
        double GetPayoff(int) except +
        double GetBeliefProb(c_GameNode) except +
        double GetRealizProb(c_GameNode) except +
        double GetInfosetProb(c_GameInfoset) except +
        double GetPayoff(c_GameInfoset) except +
        double GetPayoff(c_GamePlayer, c_GameNode) except +
        double GetPayoff(c_GameAction) except +
        double GetRegret(c_GameAction) except +
        double GetRegret(c_GameInfoset) except +
        double GetMaxRegret() except +
        double GetLiapValue() except +
        c_MixedStrategyProfileDouble ToMixedProfile()  # except + doesn't compile
        c_MixedBehaviorProfileDouble(c_MixedStrategyProfileDouble) except +NotImplementedError
        c_MixedBehaviorProfileDouble(c_Game) except +
        c_MixedBehaviorProfileDouble(c_MixedBehaviorProfileDouble) except +

    cdef cppclass c_MixedBehaviorProfileRational "MixedBehaviorProfile<Rational>":
        bool operator==(c_MixedBehaviorProfileRational) except +
        bool operator!=(c_MixedBehaviorProfileRational) except +
        c_Game GetGame() except +
        bool IsInvalidated()
        int BehaviorProfileLength() except +
        bool IsDefinedAt(c_GameInfoset) except +
        c_MixedBehaviorProfileRational Normalize()  # except + doesn't compile
        c_Rational getitem "operator[]"(int) except +IndexError
        c_Rational getaction "operator[]"(c_GameAction) except +IndexError
        c_Rational GetPayoff(int) except +
        c_Rational GetBeliefProb(c_GameNode) except +
        c_Rational GetRealizProb(c_GameNode) except +
        c_Rational GetInfosetProb(c_GameInfoset) except +
        c_Rational GetPayoff(c_GameInfoset) except +
        c_Rational GetPayoff(c_GamePlayer, c_GameNode) except +
        c_Rational GetPayoff(c_GameAction) except +
        c_Rational GetRegret(c_GameAction) except +
        c_Rational GetRegret(c_GameInfoset) except +
        c_Rational GetMaxRegret() except +
        c_Rational GetLiapValue() except +
        c_MixedStrategyProfileRational ToMixedProfile()  # except + doesn't compile
        c_MixedBehaviorProfileRational(c_MixedStrategyProfileRational) except +NotImplementedError
        c_MixedBehaviorProfileRational(c_Game) except +
        c_MixedBehaviorProfileRational(c_MixedBehaviorProfileRational) except +


cdef extern from "games/stratspt.h":
    cdef cppclass c_StrategySupportProfile "StrategySupportProfile":
        c_StrategySupportProfile(c_Game) except +
        c_StrategySupportProfile(c_StrategySupportProfile) except +
        bool operator ==(c_StrategySupportProfile) except +
        bool operator !=(c_StrategySupportProfile) except +
        c_Game GetGame() except +
        Array[int] NumStrategies() except +
        int MixedProfileLength() except +
        int GetIndex(c_GameStrategy) except +
        int NumStrategiesPlayer "NumStrategies"(int) except +IndexError
        bool IsSubsetOf(c_StrategySupportProfile) except +
        bool RemoveStrategy(c_GameStrategy) except +
        c_GameStrategy GetStrategy(int, int) except +IndexError
        bool Contains(c_GameStrategy) except +
        c_StrategySupportProfile Undominated(bool, bool)  # except + doesn't compile
        c_MixedStrategyProfileDouble NewMixedStrategyProfileDouble \
            "NewMixedStrategyProfile<double>"() except +
        c_MixedStrategyProfileRational NewMixedStrategyProfileRational \
            "NewMixedStrategyProfile<Rational>"() except +


cdef extern from "games/behavspt.h":
    cdef cppclass c_BehaviorSupportProfile "BehaviorSupportProfile":
        c_BehaviorSupportProfile(c_Game) except +


cdef extern from "util.h":
    c_Game ReadGame(char *) except +IOError
    c_Game ParseGame(char *) except +IOError
    string WriteGame(c_Game, string) except +IOError
    string WriteGame(c_StrategySupportProfile) except +IOError

    c_Rational to_rational(char *) except +

    void setitem_array_int "setitem"(Array[int] *, int, int) except +
    void setitem_array_number "setitem"(Array[c_Number], int, c_Number) except +

    void setitem_mspd_int "setitem"(c_MixedStrategyProfileDouble, int, double) except +
    void setitem_mspd_strategy "setitem"(c_MixedStrategyProfileDouble,
                                         c_GameStrategy, double) except +
    void setitem_mspr_int "setitem"(c_MixedStrategyProfileRational, int, c_Rational) except +
    void setitem_mspr_strategy "setitem"(c_MixedStrategyProfileRational,
                                         c_GameStrategy, c_Rational) except +

    void setitem_mbpd_int "setitem"(c_MixedBehaviorProfileDouble, int, double) except +
    void setitem_mbpd_action "setitem"(c_MixedBehaviorProfileDouble,
                                       c_GameAction, double) except +
    void setitem_mbpr_int "setitem"(c_MixedBehaviorProfileRational, int, c_Rational) except +
    void setitem_mbpr_action "setitem"(c_MixedBehaviorProfileRational,
                                       c_GameAction, c_Rational) except +

    shared_ptr[c_MixedStrategyProfileDouble] copyitem_list_mspd "sharedcopyitem"(
            c_List[c_MixedStrategyProfileDouble], int
    ) except +
    shared_ptr[c_MixedStrategyProfileRational] copyitem_list_mspr "sharedcopyitem"(
            c_List[c_MixedStrategyProfileRational], int
    ) except +
    shared_ptr[c_MixedBehaviorProfileDouble] copyitem_list_mbpd "sharedcopyitem"(
            c_List[c_MixedBehaviorProfileDouble], int
    ) except +
    shared_ptr[c_MixedBehaviorProfileRational] copyitem_list_mbpr "sharedcopyitem"(
            c_List[c_MixedBehaviorProfileRational], int
    ) except +
    shared_ptr[c_LogitQREMixedStrategyProfile] copyitem_list_qrem "sharedcopyitem"(
            c_List[c_LogitQREMixedStrategyProfile], int
    ) except +


cdef extern from "solvers/enumpure/enumpure.h":
    c_List[c_MixedStrategyProfileRational] EnumPureStrategySolve(c_Game) except +RuntimeError
    c_List[c_MixedBehaviorProfileRational] EnumPureAgentSolve(c_Game) except +RuntimeError

cdef extern from "solvers/enummixed/enummixed.h":
    c_List[c_MixedStrategyProfileDouble] EnumMixedStrategySolveDouble(c_Game) except +RuntimeError
    c_List[c_MixedStrategyProfileRational] EnumMixedStrategySolveRational(
            c_Game
    ) except +RuntimeError
    c_List[c_MixedStrategyProfileRational] EnumMixedStrategySolveLrs(c_Game) except +RuntimeError

cdef extern from "solvers/lcp/lcp.h":
    c_List[c_MixedStrategyProfileDouble] LcpStrategySolveDouble(
            c_Game, int p_stopAfter, int p_maxDepth
    ) except +RuntimeError
    c_List[c_MixedStrategyProfileRational] LcpStrategySolveRational(
            c_Game, int p_stopAfter, int p_maxDepth
    ) except +RuntimeError
    c_List[c_MixedBehaviorProfileDouble] LcpBehaviorSolveDouble(
            c_Game, int p_stopAfter, int p_maxDepth
    ) except +RuntimeError
    c_List[c_MixedBehaviorProfileRational] LcpBehaviorSolveRational(
            c_Game, int p_stopAfter, int p_maxDepth
    ) except +RuntimeError

cdef extern from "solvers/lp/nfglp.h":
    c_List[c_MixedStrategyProfileDouble] LpStrategySolveDouble(c_Game) except +RuntimeError
    c_List[c_MixedStrategyProfileRational] LpStrategySolveRational(c_Game) except +RuntimeError

cdef extern from "solvers/lp/efglp.h":
    c_List[c_MixedBehaviorProfileDouble] LpBehaviorSolveDouble(c_Game) except +RuntimeError
    c_List[c_MixedBehaviorProfileRational] LpBehaviorSolveRational(c_Game) except +RuntimeError

cdef extern from "solvers/liap/liap.h":
    c_List[c_MixedStrategyProfileDouble] LiapStrategySolve(
            c_MixedStrategyProfileDouble, double p_maxregret, int p_maxitsN
    ) except +RuntimeError
    c_List[c_MixedBehaviorProfileDouble] LiapBehaviorSolve(
            c_MixedBehaviorProfileDouble, double p_maxregret, int p_maxitsN
    ) except +RuntimeError

cdef extern from "solvers/simpdiv/simpdiv.h":
    c_List[c_MixedStrategyProfileRational] SimpdivStrategySolve(
            c_MixedStrategyProfileRational start, c_Rational p_maxregret, int p_gridResize,
            int p_leashLength
    ) except +RuntimeError

cdef extern from "solvers/ipa/ipa.h":
    c_List[c_MixedStrategyProfileDouble] IPAStrategySolve(
            c_MixedStrategyProfileDouble
    ) except +RuntimeError

cdef extern from "solvers/gnm/gnm.h":
    c_List[c_MixedStrategyProfileDouble] GNMStrategySolve(
            c_MixedStrategyProfileDouble, double p_endLambda, int p_steps,
            int p_localNewtonInterval, int p_localNewtonMaxits
    ) except +RuntimeError

cdef extern from "solvers/logit/nfglogit.h":
    c_List[c_MixedStrategyProfileDouble] LogitStrategySolve(c_Game,
                                                            double,
                                                            double,
                                                            double) except +RuntimeError

cdef extern from "solvers/logit/efglogit.h":
    c_List[c_MixedBehaviorProfileDouble] LogitBehaviorSolve(c_Game,
                                                            double,
                                                            double,
                                                            double) except +RuntimeError

cdef extern from "solvers/logit/nfglogit.h":
    cdef cppclass c_LogitQREMixedStrategyProfile "LogitQREMixedStrategyProfile":
        c_LogitQREMixedStrategyProfile(c_Game) except +
        c_LogitQREMixedStrategyProfile(c_LogitQREMixedStrategyProfile) except +
        c_Game GetGame() except +
        c_MixedStrategyProfileDouble GetProfile()  # except + doesn't compile
        double GetLambda() except +
        double GetLogLike() except +
        int MixedProfileLength() except +
        double getitem "operator[]"(int) except +IndexError

    cdef cppclass c_StrategicQREEstimator "StrategicQREEstimator":
        c_StrategicQREEstimator() except +
        c_LogitQREMixedStrategyProfile Estimate(c_LogitQREMixedStrategyProfile,
                                                c_MixedStrategyProfileDouble,
                                                double, double, double) except +RuntimeError

cdef extern from "nash.h":
    shared_ptr[c_LogitQREMixedStrategyProfile] _logit_estimate "logit_estimate"(
            shared_ptr[c_MixedStrategyProfileDouble], double, double
    ) except +
    shared_ptr[c_LogitQREMixedStrategyProfile] _logit_atlambda "logit_atlambda"(
            c_Game, double, double, double
    ) except +
    c_List[c_LogitQREMixedStrategyProfile] _logit_principal_branch "logit_principal_branch"(
            c_Game, double, double, double
    ) except +
