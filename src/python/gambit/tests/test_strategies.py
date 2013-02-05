import gambit
import warnings
from nose.tools import assert_raises

class TestGambitStrategies(object):
    def setUp(self):
        self.game = gambit.new_table([2,2])
        self.game.players[0].label = "Alphonse"
        self.game.players[1].label = "Gaston"
    
    def tearDown(self):
        del self.game

    def test_game_add_strategies(self):
        "Test to add strategies"
        assert len(self.game.players[0].strategies) == 2
        assert len(self.game.players[1].strategies) == 2

    def test_game_add_duplicate_strategy_names(self):
        "Test to add strategies with preexisting names"
        self.game.players[0].strategies[0].label = "Cooperate"
        with warnings.catch_warnings(True) as w:
                self.game.players[0].strategies[1].label = "Cooperate"
                assert str(w[0].message) == "This player has another strategy with an identical label"

    def test_game_strategies_index_by_string(self):
        "Test to find a strategy by providing a string"
        self.game.players[0].strategies[0].label = "Cooperate"
        s1 = self.game.players[0].strategies["Cooperate"]
        assert s1.label == "Cooperate"

    def test_game_strategies_index_exception_int(self):
        "Test to verify when an index is out of range"
        assert_raises(IndexError, self.game.players[0].strategies.__getitem__, 3)

    def test_game_strategies_index_exception_string(self):
        "Test to verify when a strategy label is not in the list of player's strategies"
        assert_raises(IndexError, self.game.players[0].strategies.__getitem__, "None")

    def test_game_strategies_index_exception_player(self):
        "Test to verify when a strategy object is not in a player's list of strategies"
        self.game_2 = gambit.new_table([2,2])
        assert_raises(IndexError, self.game.players[0].strategies.__getitem__, self.game_2.players[0].strategies[0])

    def test_game_strategies_index_exception_player(self):
        "Test to verify when attempting to retrieve strategy with invalid input"
        assert_raises(TypeError, self.game.players[0].strategies.__getitem__, 1.3)
