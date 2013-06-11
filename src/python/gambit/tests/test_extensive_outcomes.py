import gambit
from nose.tools import assert_raises
import warnings

class TestGambitOutcomes(object):
    def setUp(self):
        self.game = gambit.read_game("./test_games/sample_extensive_game.efg")
    
    def tearDown(self):
        del self.game

    def test_player1_outcomes(self):
        assert(self.game[[0,0]][0] == 2)  
        assert(self.game[[0,1]][0] == 2)
        assert(self.game[[1,0]][0] == 4)
        assert(self.game[[1,1]][0] == 6)

    def test_player2_outcomes(self):
        assert(self.game[[0,0]][1] == 3)  
        assert(self.game[[0,1]][1] == 3)
        assert(self.game[[1,0]][1] == 5)
        assert(self.game[[1,1]][1] == 7)

    def test_getting_payoff_by_label_string(self):
        assert(self.game[[0,0]]['Player 1'] == 2)
        assert(self.game[[0,1]]['Player 1'] == 2)
        assert(self.game[[1,0]]['Player 1'] == 4)
        assert(self.game[[1,1]]['Player 1'] == 6)
        assert(self.game[[0,0]]['Player 2'] == 3)
        assert(self.game[[0,1]]['Player 2'] == 3)
        assert(self.game[[1,0]]['Player 2'] == 5)
        assert(self.game[[1,1]]['Player 2'] == 7)

    def test_getting_payoff_by_player(self):
        player1 = self.game.players[0]
        player2 = self.game.players[1]
        assert(self.game[[0,0]][player1] == 2)
        assert(self.game[[0,1]][player1] == 2)
        assert(self.game[[1,0]][player1] == 4)
        assert(self.game[[1,1]][player1] == 6)
        assert(self.game[[0,0]][player2] == 3)
        assert(self.game[[0,1]][player2] == 3)
        assert(self.game[[1,0]][player2] == 5)
        assert(self.game[[1,1]][player2] == 7)

    def test_outcome_index_exception_int(self):
        assert_raises(IndexError, self.game[[0,0]].__getitem__, 3)
