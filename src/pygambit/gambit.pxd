from libcpp cimport bool
from libcpp.string cimport string
from libcpp.memory cimport shared_ptr, unique_ptr
from libcpp.list cimport list as stdlist
from libcpp.vector cimport vector as stdvector
from libcpp.set cimport set as stdset


cdef extern from "gambit.h":
    # We don't wrap anything from the basic header, but it ensures
    # it gets included in the output
    pass


cdef extern from "core/rational.h":
    cdef cppclass c_Rational "Rational":
        c_Rational() except +
    string to_string "lexical_cast<std::string>"(c_Rational) except +
    c_Rational to_rational "lexical_cast<Rational>"(string) except +


cdef extern from "games/number.h":
    cdef cppclass c_Number "Number":
        c_Number() except +
        c_Number(string) except +
        string as_string "operator const string &"() except +


cdef extern from "core/array.h":
    cdef cppclass Array[T]:
        cppclass iterator:
            T operator*()
            iterator operator++()
            bint operator==(iterator)
            bint operator!=(iterator)
        T getitem "operator[]"(int) except +
        int size() except +
        Array() except +
        Array(int) except +
        void push_back(T) except +
        iterator begin() except +
        iterator end() except +


cdef extern from "games/game.h":
    cdef cppclass c_GameRep "GameRep"
    cdef cppclass c_GameStrategyRep "GameStrategyRep"
    cdef cppclass c_GameActionRep "GameActionRep"
    cdef cppclass c_GameInfosetRep "GameInfosetRep"
    cdef cppclass c_GamePlayerRep "GamePlayerRep"
    cdef cppclass c_GameOutcomeRep "GameOutcomeRep"
    cdef cppclass c_GameNodeRep "GameNodeRep"

    cdef cppclass c_Game "Game":
        c_GameRep *deref "operator->"() except +RuntimeError

    cdef cppclass c_GamePlayer "GameObjectPtr<GamePlayerRep>":
        bool operator !=(c_GamePlayer) except +
        c_GamePlayerRep *deref "get"() except +RuntimeError

    cdef cppclass c_GameOutcome "GameObjectPtr<GameOutcomeRep>":
        bool operator==(c_GameOutcome) except +
        bool operator !=(c_GameOutcome) except +
        c_GameOutcomeRep *deref "get"() except +RuntimeError

    cdef cppclass c_GameNode "GameObjectPtr<GameNodeRep>":
        bool operator !=(c_GameNode) except +
        c_GameNodeRep *deref "get"() except +RuntimeError

    cdef cppclass c_GameAction "GameObjectPtr<GameActionRep>":
        bool operator !() except +
        bool operator !=(c_GameAction) except +
        c_GameActionRep *deref "get"() except +RuntimeError

    cdef cppclass c_GameInfoset "GameObjectPtr<GameInfosetRep>":
        bool operator ==(c_GameInfoset) except +
        bool operator !=(c_GameInfoset) except +
        c_GameInfosetRep *deref "get"() except +RuntimeError

    cdef cppclass c_GameStrategy "GameObjectPtr<GameStrategyRep>":
        c_GameStrategyRep *deref "get"() except +RuntimeError

    cdef cppclass c_PureStrategyProfile "PureStrategyProfile":
        shared_ptr[c_PureStrategyProfileRep] deref "operator->"() except +
        c_PureStrategyProfile(c_PureStrategyProfile) except +

    cdef cppclass c_PureBehaviorProfile "PureBehaviorProfile":
        c_PureBehaviorProfile(c_Game) except +

    cdef cppclass c_GameStrategyRep "GameStrategyRep":
        int GetNumber() except +
        int GetId() except +
        c_GamePlayer GetPlayer() except +
        string GetLabel() except +
        void SetLabel(string) except +
        c_GameAction GetAction(c_GameInfoset) except +

    cdef cppclass c_GameActionRep "GameActionRep":
        int GetNumber() except +
        c_GameInfoset GetInfoset() except +
        bint Precedes(c_GameNode) except +

        string GetLabel() except +
        void SetLabel(string) except +

    cdef cppclass c_GameInfosetRep "GameInfosetRep":
        cppclass Actions:
            cppclass iterator:
                c_GameAction operator *()
                iterator operator++()
                bint operator ==(iterator)
                bint operator !=(iterator)
            int size() except +
            iterator begin() except +
            iterator end() except +

        cppclass Members:
            cppclass iterator:
                c_GameNode operator *()
                iterator operator++()
                bint operator ==(iterator)
                bint operator !=(iterator)
            int size() except +
            iterator begin() except +
            iterator end() except +

        int GetNumber() except +
        c_Game GetGame() except +
        c_GamePlayer GetPlayer() except +

        string GetLabel() except +
        void SetLabel(string) except +

        c_GameAction GetAction(int) except +IndexError
        Actions GetActions() except +
        c_Number GetActionProb(c_GameAction) except +IndexError

        c_GameNode GetMember(int) except +IndexError
        Members GetMembers() except +

        bint IsChanceInfoset() except +
        bint Precedes(c_GameNode) except +
        stdset[c_GameAction] GetOwnPriorActions() except +

    cdef cppclass c_GamePlayerRep "GamePlayerRep":
        cppclass Infosets:
            cppclass iterator:
                c_GameInfoset operator *()
                iterator operator++()
                bint operator ==(iterator)
                bint operator !=(iterator)
            int size() except +
            iterator begin() except +
            iterator end() except +

        cppclass Strategies:
            cppclass iterator:
                c_GameStrategy operator *()
                iterator operator++()
                bint operator ==(iterator)
                bint operator !=(iterator)
            int size() except +
            iterator begin() except +
            iterator end() except +

        c_Game GetGame() except +
        int GetNumber() except +
        int IsChance() except +

        string GetLabel() except +
        void SetLabel(string) except +

        c_GameStrategy GetStrategy(int) except +IndexError
        Strategies GetStrategies() except +

        c_GameInfoset GetInfoset(int) except +IndexError
        Infosets GetInfosets() except +

    cdef cppclass c_GameOutcomeRep "GameOutcomeRep":
        c_Game GetGame() except +
        int GetNumber() except +

        string GetLabel() except +
        void SetLabel(string) except +

        T GetPayoff[T](c_GamePlayer) except +IndexError
        void SetPayoff(c_GamePlayer, c_Number) except +IndexError

    cdef cppclass c_GameNodeRep "GameNodeRep":
        cppclass Children:
            cppclass iterator:
                c_GameNode operator *()
                iterator operator++()
                bint operator ==(iterator)
                bint operator !=(iterator)
            int size() except +
            iterator begin() except +
            iterator end() except +

        c_Game GetGame() except +
        int GetNumber() except +

        string GetLabel() except +
        void SetLabel(string) except +

        c_GameInfoset GetInfoset() except +
        c_GamePlayer GetPlayer() except +
        c_GameNode GetParent() except +
        Children GetChildren() except +
        c_GameNode GetChild(c_GameAction) except +IndexError
        c_GameOutcome GetOutcome() except +
        c_GameNode GetPriorSibling() except +
        c_GameNode GetNextSibling() except +
        bint IsTerminal() except +
        bint IsSuccessorOf(c_GameNode) except +
        bint IsSubgameRoot() except +
        bint IsStrategyReachable() except +
        c_GameAction GetPriorAction() except +
        c_GameAction GetOwnPriorAction() except +

    cdef cppclass c_GameRep "GameRep":
        cppclass Players:
            cppclass iterator:
                c_GamePlayer operator *()
                iterator operator++()
                bint operator ==(iterator)
                bint operator !=(iterator)
            int size() except +
            iterator begin() except +
            iterator end() except +

        cppclass Outcomes:
            cppclass iterator:
                c_GameOutcome operator *()
                iterator operator++()
                bint operator ==(iterator)
                bint operator !=(iterator)
            int size() except +
            iterator begin() except +
            iterator end() except +

        cppclass Nodes:
            cppclass iterator:
                bint operator ==(iterator)
                bint operator !=(iterator)
                c_GameNode operator *()
                iterator operator++()
            iterator begin() except +
            iterator end() except +

        int IsTree() except +

        string GetTitle() except +
        void SetTitle(string) except +

        string GetComment() except +
        void SetComment(string) except +

        int NumPlayers() except +
        c_GamePlayer GetPlayer(int) except +IndexError
        Players GetPlayers() except +
        c_GamePlayer GetChance() except +
        c_GamePlayer NewPlayer() except +

        int NumOutcomes() except +
        c_GameOutcome GetOutcome(int) except +IndexError
        Outcomes GetOutcomes() except +
        c_GameOutcome NewOutcome() except +
        void DeleteOutcome(c_GameOutcome) except +

        int NumNodes() except +
        int NumNonterminalNodes() except +
        c_GameNode GetRoot() except +
        Nodes GetNodes() except +

        c_GameStrategy GetStrategy(int) except +IndexError
        c_GameStrategy NewStrategy(c_GamePlayer, string) except +
        void DeleteStrategy(c_GameStrategy) except +
        int MixedProfileLength() except +

        c_GameInfoset GetInfoset(int) except +IndexError
        Array[int] NumInfosets() except +

        c_GameAction GetAction(int) except +IndexError
        int BehavProfileLength() except +

        bool IsConstSum() except +
        c_Rational GetMinPayoff() except +
        c_Rational GetPlayerMinPayoff(c_GamePlayer) except +
        c_Rational GetMaxPayoff() except +
        c_Rational GetPlayerMaxPayoff(c_GamePlayer) except +
        stdvector[c_GameNode] GetPlays(c_GameNode) except +
        stdvector[c_GameNode] GetPlays(c_GameInfoset) except +
        stdvector[c_GameNode] GetPlays(c_GameAction) except +
        bool IsPerfectRecall() except +
        bool IsAbsentMinded(c_GameInfoset) except +

        c_GameInfoset AppendMove(c_GameNode, c_GamePlayer, int) except +ValueError
        c_GameInfoset AppendMove(c_GameNode, c_GameInfoset) except +ValueError
        c_GameInfoset InsertMove(c_GameNode, c_GamePlayer, int) except +ValueError
        c_GameInfoset InsertMove(c_GameNode, c_GameInfoset) except +ValueError
        void CopyTree(c_GameNode dest, c_GameNode src) except +ValueError
        void MoveTree(c_GameNode dest, c_GameNode src) except +ValueError
        void DeleteParent(c_GameNode) except +
        void DeleteTree(c_GameNode) except +
        void SetPlayer(c_GameInfoset, c_GamePlayer) except +
        void Reveal(c_GameInfoset, c_GamePlayer) except +
        void SetInfoset(c_GameNode, c_GameInfoset) except +ValueError
        c_GameInfoset LeaveInfoset(c_GameNode) except +
        c_GameAction InsertAction(c_GameInfoset, c_GameAction) except +ValueError
        void DeleteAction(c_GameAction) except +ValueError
        void SetOutcome(c_GameNode, c_GameOutcome) except +
        c_Game SetChanceProbs(c_GameInfoset, Array[c_Number]) except +

        c_PureStrategyProfile NewPureStrategyProfile()  # except + doesn't compile
        c_MixedStrategyProfile[T] NewMixedStrategyProfile[T](T)  # except + doesn't compile

    c_Game NewTree() except +
    c_Game NewTable(stdvector[int]) except +


cdef extern from "games/stratpure.h":
    cdef cppclass c_PureStrategyProfileRep "PureStrategyProfileRep":
        c_GameStrategy GetStrategy(c_GamePlayer) except +
        void SetStrategy(c_GameStrategy) except +

        c_GameOutcome GetOutcome() except +
        void SetOutcome(c_GameOutcome) except +

        c_Rational GetPayoff(c_GamePlayer) except +


cdef extern from "games/stratmixed.h" namespace "Gambit":
    cdef cppclass c_MixedStrategyProfile "MixedStrategyProfile"[T]:
        bool operator==(c_MixedStrategyProfile[T]) except +
        bool operator!=(c_MixedStrategyProfile[T]) except +
        c_Game GetGame() except +
        bool HasOutdatedGameVersion()
        int MixedProfileLength() except +
        c_StrategySupportProfile GetSupport() except +
        c_MixedStrategyProfile[T] Normalize()  # except + doesn't compile
        T getitem_strategy "operator[]"(c_GameStrategy) except +IndexError
        T GetPayoff(c_GamePlayer) except +
        T GetPayoff(c_GameStrategy) except +
        T GetRegret(c_GameStrategy) except +
        T GetRegret(c_GamePlayer) except +
        T GetMaxRegret() except +
        T GetPayoffDeriv(int, c_GameStrategy, c_GameStrategy) except +
        T GetLiapValue() except +
        c_MixedStrategyProfile[T] ToFullSupport() except +
        c_MixedStrategyProfile(c_MixedStrategyProfile[T]) except +

cdef extern from "games/behavmixed.h" namespace "Gambit":
    cdef cppclass c_MixedBehaviorProfile "MixedBehaviorProfile"[T]:
        bool operator==(c_MixedBehaviorProfile[T]) except +
        bool operator!=(c_MixedBehaviorProfile[T]) except +
        c_Game GetGame() except +
        bool IsInvalidated()
        int BehaviorProfileLength() except +
        bool IsDefinedAt(c_GameInfoset) except +
        c_MixedBehaviorProfile[T] Normalize()  # except + doesn't compile
        T getitem "operator[]"(int) except +IndexError
        T getaction "operator[]"(c_GameAction) except +IndexError
        T GetPayoff(c_GamePlayer) except +
        T GetBeliefProb(c_GameNode) except +
        T GetRealizProb(c_GameNode) except +
        T GetInfosetProb(c_GameInfoset) except +
        T GetPayoff(c_GameInfoset) except +
        T GetPayoff(c_GamePlayer, c_GameNode) except +
        T GetPayoff(c_GameAction) except +
        T GetRegret(c_GameAction) except +
        T GetRegret(c_GameInfoset) except +
        T GetAgentMaxRegret() except +
        T GetAgentLiapValue() except +
        T GetMaxRegret() except +
        T GetLiapValue() except +
        c_MixedStrategyProfile[T] ToMixedProfile()  # except + doesn't compile
        c_MixedBehaviorProfile(c_MixedStrategyProfile[T]) except +NotImplementedError
        c_MixedBehaviorProfile(c_Game) except +
        c_MixedBehaviorProfile(c_MixedBehaviorProfile[T]) except +

cdef extern from "games/stratspt.h":
    cdef cppclass c_StrategySupportProfile "StrategySupportProfile":
        cppclass Support:
            cppclass const_iterator:
                c_GameStrategy operator *()
                const_iterator operator++()
                bint operator ==(const_iterator)
                bint operator !=(const_iterator)
            Support()
            int size()
            const_iterator begin() except +
            const_iterator end() except +

        c_StrategySupportProfile(c_Game) except +
        c_StrategySupportProfile(c_StrategySupportProfile) except +
        bool operator ==(c_StrategySupportProfile) except +
        bool operator !=(c_StrategySupportProfile) except +
        c_Game GetGame() except +
        Array[int] NumStrategies() except +
        int MixedProfileLength() except +
        int GetIndex(c_GameStrategy) except +
        bool IsSubsetOf(c_StrategySupportProfile) except +
        bool RemoveStrategy(c_GameStrategy) except +
        Support GetStrategies(c_GamePlayer) except +
        bool Contains(c_GameStrategy) except +
        bool IsDominated(c_GameStrategy, bool, bool) except +
        c_StrategySupportProfile Undominated(bool, bool)  # except + doesn't compile
        c_MixedStrategyProfile[double] NewMixedStrategyProfileDouble \
            "NewMixedStrategyProfile<double>"() except +
        c_MixedStrategyProfile[c_Rational] NewMixedStrategyProfileRational \
            "NewMixedStrategyProfile<Rational>"() except +


cdef extern from "games/behavspt.h":
    cdef cppclass c_BehaviorSupportProfile "BehaviorSupportProfile":
        c_BehaviorSupportProfile(c_Game) except +


cdef extern from "games/layout.h":
    cdef cppclass c_Layout "Layout":
        int GetNodeLevel(c_GameNode) except +
        int GetNodeSublevel(c_GameNode) except +
        double GetNodeOffset(c_GameNode) except +
    shared_ptr[c_Layout] CreateLayout(c_Game) except +


cdef extern from "util.h":
    c_Game ParseGbtGame(string, bint) except +IOError
    c_Game ParseEfgGame(string, bint) except +IOError
    c_Game ParseNfgGame(string, bint) except +IOError
    c_Game ParseAggGame(string, bint) except +IOError
    string WriteEfgFile(c_Game)
    string WriteNfgFile(c_Game)
    string WriteNfgFileSupport(c_StrategySupportProfile) except +IOError
    string WriteLaTeXFile(c_Game)
    string WriteHTMLFile(c_Game)

    stdlist[shared_ptr[T]] make_list_of_pointer[T](stdlist[T]) except +

    void setitem_array_int "setitem"(Array[int] *, int, int) except +
    void setitem_array_number "setitem"(Array[c_Number], int, c_Number) except +
    void setitem_array_int "setitem"(Array[int] *, int, int) except +
    void setitem_array_number "setitem"(Array[c_Number], int, c_Number) except +

    void setitem_mspd_int "setitem"(c_MixedStrategyProfile[double], int, double) except +
    void setitem_mspd_strategy "setitem"(c_MixedStrategyProfile[double],
                                         c_GameStrategy, double) except +
    void setitem_mspr_int "setitem"(c_MixedStrategyProfile[c_Rational], int, c_Rational) except +
    void setitem_mspr_strategy "setitem"(c_MixedStrategyProfile[c_Rational],
                                         c_GameStrategy, c_Rational) except +

    void setitem_mbpd_int "setitem"(c_MixedBehaviorProfile[double], int, double) except +
    void setitem_mbpd_action "setitem"(c_MixedBehaviorProfile[double],
                                       c_GameAction, double) except +
    void setitem_mbpr_int "setitem"(c_MixedBehaviorProfile[c_Rational], int, c_Rational) except +
    void setitem_mbpr_action "setitem"(c_MixedBehaviorProfile[c_Rational],
                                       c_GameAction, c_Rational) except +

    shared_ptr[c_MixedStrategyProfile[double]] copyitem_list_mspd "sharedcopyitem"(
            stdlist[c_MixedStrategyProfile[double]], int
    ) except +
    shared_ptr[c_MixedStrategyProfile[c_Rational]] copyitem_list_mspr "sharedcopyitem"(
            stdlist[c_MixedStrategyProfile[c_Rational]], int
    ) except +
    shared_ptr[c_MixedBehaviorProfile[double]] copyitem_list_mbpd "sharedcopyitem"(
            stdlist[c_MixedBehaviorProfile[double]], int
    ) except +
    shared_ptr[c_MixedBehaviorProfile[c_Rational]] copyitem_list_mbpr "sharedcopyitem"(
            stdlist[c_MixedBehaviorProfile[c_Rational]], int
    ) except +
    shared_ptr[c_LogitQREMixedStrategyProfile] copyitem_list_qrem "sharedcopyitem"(
            stdlist[c_LogitQREMixedStrategyProfile], int
    ) except +
    shared_ptr[c_LogitQREMixedBehaviorProfile] copyitem_list_qreb "sharedcopyitem"(
            stdlist[c_LogitQREMixedBehaviorProfile], int
    ) except +


cdef extern from "solvers/enumpure/enumpure.h":
    stdlist[c_MixedStrategyProfile[c_Rational]] EnumPureStrategySolve(c_Game) except +RuntimeError
    stdlist[c_MixedBehaviorProfile[c_Rational]] EnumPureAgentSolve(c_Game) except +RuntimeError

cdef extern from "solvers/enummixed/enummixed.h":
    stdlist[c_MixedStrategyProfile[T]] EnumMixedStrategySolve[T](c_Game) except +RuntimeError

cdef extern from "solvers/lcp/lcp.h":
    stdlist[c_MixedStrategyProfile[T]] LcpStrategySolve[T](
            c_Game, int p_stopAfter, int p_maxDepth
    ) except +RuntimeError
    stdlist[c_MixedBehaviorProfile[T]] LcpBehaviorSolve[T](
            c_Game, int p_stopAfter, int p_maxDepth
    ) except +RuntimeError

cdef extern from "solvers/lp/lp.h":
    stdlist[c_MixedStrategyProfile[T]] LpStrategySolve[T](c_Game) except +RuntimeError
    stdlist[c_MixedBehaviorProfile[T]] LpBehaviorSolve[T](c_Game) except +RuntimeError

cdef extern from "solvers/liap/liap.h":
    stdlist[c_MixedStrategyProfile[double]] LiapStrategySolve(
            c_MixedStrategyProfile[double], double p_maxregret, int p_maxitsN
    ) except +RuntimeError
    stdlist[c_MixedBehaviorProfile[double]] LiapAgentSolve(
            c_MixedBehaviorProfile[double], double p_maxregret, int p_maxitsN
    ) except +RuntimeError

cdef extern from "solvers/simpdiv/simpdiv.h":
    stdlist[c_MixedStrategyProfile[c_Rational]] SimpdivStrategySolve(
            c_MixedStrategyProfile[c_Rational] start, c_Rational p_maxregret, int p_gridResize,
            int p_leashLength
    ) except +RuntimeError

cdef extern from "solvers/ipa/ipa.h":
    stdlist[c_MixedStrategyProfile[double]] IPAStrategySolve(
            c_MixedStrategyProfile[double]
    ) except +RuntimeError

cdef extern from "solvers/gnm/gnm.h":
    stdlist[c_MixedStrategyProfile[double]] GNMStrategySolve(
            c_MixedStrategyProfile[double], double p_endLambda, int p_steps,
            int p_localNewtonInterval, int p_localNewtonMaxits
    ) except +RuntimeError

cdef extern from "solvers/nashsupport/nashsupport.h":
    cdef cppclass c_PossibleNashStrategySupportsResult "PossibleNashStrategySupportsResult":
        stdlist[c_StrategySupportProfile] m_supports
    shared_ptr[c_PossibleNashStrategySupportsResult] PossibleNashStrategySupports(
            c_Game
    ) except +RuntimeError

cdef extern from "solvers/enumpoly/enumpoly.h":
    stdlist[c_MixedStrategyProfile[double]] EnumPolyStrategySolve(
            c_Game, int, float
    ) except +RuntimeError
    stdlist[c_MixedBehaviorProfile[double]] EnumPolyBehaviorSolve(
            c_Game, int, float
    ) except +RuntimeError

cdef extern from "solvers/logit/logit.h":
    cdef cppclass c_LogitQREMixedBehaviorProfile "LogitQREMixedBehaviorProfile":
        c_LogitQREMixedBehaviorProfile(c_Game) except +
        c_LogitQREMixedBehaviorProfile(c_LogitQREMixedBehaviorProfile) except +
        c_Game GetGame() except +
        c_MixedBehaviorProfile[double] GetProfile()  # except + doesn't compile
        double GetLambda() except +
        double GetLogLike() except +
        int size() except +
        double getitem "operator[]"(int) except +IndexError

    cdef cppclass c_LogitQREMixedStrategyProfile "LogitQREMixedStrategyProfile":
        c_LogitQREMixedStrategyProfile(c_Game) except +
        c_LogitQREMixedStrategyProfile(c_LogitQREMixedStrategyProfile) except +
        c_Game GetGame() except +
        c_MixedStrategyProfile[double] GetProfile()  # except + doesn't compile
        double GetLambda() except +
        double GetLogLike() except +
        int size() except +
        double getitem "operator[]"(int) except +IndexError


cdef extern from "nash.h":
    stdlist[c_MixedBehaviorProfile[double]] LogitBehaviorSolveWrapper(
            c_Game, double, double, double
    ) except +
    stdlist[c_LogitQREMixedBehaviorProfile] LogitBehaviorPrincipalBranchWrapper(
            c_Game, double, double, double
    ) except +
    stdlist[shared_ptr[c_LogitQREMixedBehaviorProfile]] LogitBehaviorAtLambdaWrapper(
            c_Game, stdlist[double], double, double
    ) except +
    shared_ptr[c_LogitQREMixedBehaviorProfile] LogitBehaviorEstimateWrapper(
            shared_ptr[c_MixedBehaviorProfile[double]], bool, double, double
    ) except +
    stdlist[c_MixedStrategyProfile[double]] LogitStrategySolveWrapper(
            c_Game, double, double, double
    ) except +
    stdlist[c_LogitQREMixedStrategyProfile] LogitStrategyPrincipalBranchWrapper(
            c_Game, double, double, double
    ) except +
    stdlist[shared_ptr[c_LogitQREMixedStrategyProfile]] LogitStrategyAtLambdaWrapper(
            c_Game, stdlist[double], double, double
    ) except +
    shared_ptr[c_LogitQREMixedStrategyProfile] LogitStrategyEstimateWrapper(
            shared_ptr[c_MixedStrategyProfile[double]], bool, double, double
    ) except +
