from libcpp.map cimport map as cpp_map
from libcpp.memory cimport shared_ptr as cpp_shared_ptr

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
    cdef cppclass c_GameSequenceForm "GameSequenceForm":
        c_GameSequenceForm(const c_BehaviorSupportProfile&) except +
        c_Rational PayoffFromActions(cpp_map[c_GamePlayer, c_GameAction] action_profile,
                                                c_GamePlayer p_player)
        c_GameSequence GetEmptySequence(c_GamePlayer player)
        c_GameSequence GetCorrespondingSequence(c_GameAction action)
        c_Rational GetPayoff(cpp_map[c_GamePlayer, c_GameSequence] p_profile, c_GamePlayer p_player)

cdef class GameSequenceForm:
    cdef c_GameSequenceForm* cpp_form
    cdef c_BehaviorSupportProfile* _bsp  # internal

    def __cinit__(self, py_game):
        cdef Game game = cython.cast(Game, py_game)
        cdef c_Game cpp_game = game.game
        self._bsp = new c_BehaviorSupportProfile(cpp_game)
        self.cpp_form = new c_GameSequenceForm(deref(self._bsp))

    def __dealloc__(self):
        del self.cpp_form
        del self._bsp

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
            temp_player = <c_GamePlayer>key.player
            if py_value is None:
                profile[temp_player] = self.cpp_form.GetEmptySequence(temp_player)
            else:
                value = cython.cast(Action, py_value)
                temp_action = <c_GameAction>value.action
                profile[temp_player] = self.cpp_form.GetCorrespondingSequence(temp_action)
        cdef c_Rational payoff_obj = self.cpp_form.GetPayoff(profile, cpp_player)
        return rat_to_py_new(payoff_obj)


