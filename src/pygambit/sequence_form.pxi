from libcpp.map cimport map as cpp_map
from libcpp.memory cimport shared_ptr as cpp_shared_ptr
from libcpp.vector cimport vector as cpp_vector

@cython.cfunc
def rat_to_py_new(r: c_Rational):
    s = to_string(r).decode("ascii")
    if not s:
        return Rational(0)
    return Rational(s)


cdef extern from "../games/behavspt.h" namespace "Gambit":
    cdef cppclass c_BehaviorSupportProfile "BehaviorSupportProfile":
        c_BehaviourSupportProfile(const c_Game&) except +


cdef extern from "../solvers/enumpoly/gameseq.h" namespace "Gambit":
    cdef cppclass c_GameSequenceRep "GameSequenceRep":
            pass


ctypedef cpp_shared_ptr[c_GameSequenceRep] c_GameSequence


cdef extern from "../solvers/enumpoly/gameseq.h" namespace "Gambit":
    cdef cppclass c_PlayerSequences "GameSequenceForm::PlayerSequences":
        cpp_vector[c_GameSequence].const_iterator begin()


cdef extern from "../solvers/enumpoly/gameseq.h" namespace "Gambit":
    cdef cppclass c_GameSequenceForm "GameSequenceForm":
        c_GameSequenceForm(const c_BehaviorSupportProfile&) except +
        c_Rational PayoffFromActions(cpp_map[c_GamePlayer, c_GameAction] action_profile, c_GamePlayer p_player)
        c_GameSequence GetCorrespondingSequence(c_GameAction action)
        c_PlayerSequences GetSequences(const c_GamePlayer &p_player) const
        const c_Rational &GetPayoff(const cpp_map[c_GamePlayer, c_GameSequence] &p_profile, const c_GamePlayer &p_player) const
        int GetConstraintEntry(const c_GameInfoset &p_infoset, const c_GameAction &p_action) const


cdef class GameSequenceForm:
    cdef c_GameSequenceForm* seq_form
    cdef c_BehaviorSupportProfile* support

    def __cinit__(self, py_game):
        cdef Game game = cython.cast(Game, py_game)
        cdef c_Game cpp_game = game.game
        self.support = new c_BehaviorSupportProfile(cpp_game)
        self.seq_form = new c_GameSequenceForm(deref(self.support))

    def __dealloc__(self):
        del self.seq_form
        del self.support

    def get_payoff(self, action_dict, py_player):
        cdef Player player = cython.cast(Player, py_player)
        cdef c_GamePlayer cpp_player = player.player
        cdef cpp_map[c_GamePlayer, c_GameSequence] profile
        cdef c_GamePlayer temp_player
        cdef c_GameAction temp_action
        cdef Player key
        cdef Action value
        for py_key, py_value in action_dict.items():
            key = cython.cast(Player, py_key)
            temp_player = key.player
            if py_value is None:
                profile[temp_player] = deref(self.seq_form.GetSequences(temp_player).begin())
            else:
                value = cython.cast(Action, py_value)
                temp_action = value.action
                profile[temp_player] = self.seq_form.GetCorrespondingSequence(temp_action)
        cdef c_Rational payoff = self.seq_form.GetPayoff(profile, cpp_player)
        return rat_to_py_new(payoff)

    def get_constraint_entry(self, py_infoset, py_action):
        cdef Infoset infoset = cython.cast(Infoset, py_infoset)
        cdef c_GameInfoset cpp_infoset = infoset.infoset
        cdef Action action = cython.cast(Action, py_action)
        cdef c_GameAction cpp_action = action.action
        return self.seq_form.GetConstraintEntry(cpp_infoset, cpp_action)


