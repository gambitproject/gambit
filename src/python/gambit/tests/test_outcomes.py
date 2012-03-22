import gambit
from nose.tools import assert_raises
import warnings

class TestGambitOutcomes(object):
    def setUp(self):
        self.game = gambit.new_table([2,2])
    
    def tearDown(self):
        del self.game

    def test_outcomes_number_index(self):
        "Test to verify outcome labels and indexing"
        self.game.outcomes[0].label = "trial"
        self.game.outcomes[1].label = "trial 2"
        
        assert self.game.outcomes[0].label == "trial"
        assert self.game.outcomes[1].label == "trial 2"
        
    def test_game_add_outcomes(self):
        "Test to verify outcome indexing"
        self.game.outcomes[0].label = "trial"
        assert self.game.outcomes[0].label == "trial"

    def test_game_add_duplicate_outcome_names(self):
        "Test to verify duplicate outcome names"
        self.game.outcomes[0].label = "trial"
        assert self.game.outcomes[0].label == "trial"
        with warnings.catch_warnings(True) as w:
                self.game.outcomes[1].label = "trial"
                assert str(w[0].message) == "Another outcome with an identical label exists"

    def test_game_outcomes_index_by_string(self):
        "Test to find an outcome by providing a string"
        self.game.outcomes[0].label = "trial"
        o1 = self.game.outcomes["trial"]
        assert o1.label == "trial"

    def test_game_outcome_index_exception_int(self):
        "Test to verify when an index is out of range"
        assert_raises(IndexError, self.game.outcomes.__getitem__, 9)

    def test_game_outcome_index_exception_string(self):
        "Test to verify when an outcome label is not in the list of game outcomes"
        assert_raises(IndexError, self.game.outcomes.__getitem__, "None")

    def test_game_ouctome_index_exception_invalid_input(self):
        "Test to verify when attempting to retrieve an outcome with invalid input"
        assert_raises(TypeError, self.game.outcomes.__getitem__, 1.3)
