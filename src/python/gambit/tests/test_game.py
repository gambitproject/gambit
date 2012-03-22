import gambit
from nose.tools import assert_raises
import warnings

class TestGambitGame(object):
    def setUp(self):
        self.game = gambit.new_table([2,2])
    
    def tearDown(self):
        del self.game

    def test_game_get_outcome_with_ints(self):
        "To test getting the first outcome"
        assert self.game.outcomes[0] == self.game[0,0]

    def test_game_get_outcome_with_incorrect_tuple_size(self):
        "To test getting an outcome with a bad tuple size"
        assert_raises(KeyError, self.game.__getitem__, (0,0,0))


    def test_game_get_outcome_with_bad_ints(self):
        "To test getting an index error with ints"
        assert_raises(IndexError,self.game.__getitem__,(0,3))

    def test_game_get_outcome_with_strings(self):
        "To test getting the first outcome with strategy labels"
        self.game.players[0].strategies[0].label = "defect"
        self.game.players[1].strategies[0].label = "cooperate"
        assert self.game.outcomes[0] == self.game["defect","cooperate"]

    def test_game_get_outcome_with_bad_strings(self):
        "To test getting the first outcome with strategy labels"
        self.game.players[0].strategies[0].label = "defect"
        self.game.players[1].strategies[0].label = "cooperate"
        assert_raises(IndexError,self.game.__getitem__,("defect","defect"))


    def test_game_get_outcome_with_strategies(self):
        "To test getting the first outcome with strategies"
        assert self.game.outcomes[0] == self.game[self.game.players[0].strategies[0], self.game.players[1].strategies[0]]

    def test_game_get_outcome_with_bad_strategies(self):
        "To test getting the first outcome with incorrect strategies"
        assert_raises(IndexError, self.game.__getitem__, (self.game.players[0].strategies[0], self.game.players[0].strategies[0]))


    def test_game_outcomes_non_tuple(self):
        "To test when attempting to find outcome with a non-tuple-like object"
        assert_raises(TypeError, self.game.__getitem__, 42)
        
    def test_game_outcomes_type_exception(self):
        "To test when attempting to find outcome with invalid input"
        assert_raises(TypeError, self.game.__getitem__, (1.23,1))

    def test_game_is_const_sum(self):
        "To test checking if the game is constant sum"
        self.game_2 = gambit.read_game("test_games/const_sum_game.nfg")
        assert self.game_2.is_const_sum == True

    def test_game_is_not_const_sum(self):
        "To test checking if the game is not constant sum"
        self.game_2 = gambit.read_game("test_games/non_const_sum_game.nfg")
        assert self.game_2.is_const_sum == False
