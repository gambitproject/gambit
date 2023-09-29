import unittest

import pygambit


class TestGambitOutcomes(unittest.TestCase):
    def setUp(self):
        self.game = pygambit.Game.read_game(
            "./test_games/sample_extensive_game.efg"
        )

    def tearDown(self):
        del self.game

    def test_player1_outcomes(self):
        assert self.game[[0, 0]]["Player 1"] == 2
        assert self.game[[0, 1]]["Player 1"] == 2
        assert self.game[[1, 0]]["Player 1"] == 4
        assert self.game[[1, 1]]["Player 1"] == 6

    def test_player2_outcomes(self):
        assert self.game[[0, 0]]["Player 2"] == 3
        assert self.game[[0, 1]]["Player 2"] == 3
        assert self.game[[1, 0]]["Player 2"] == 5
        assert self.game[[1, 1]]["Player 2"] == 7

    def test_getting_payoff_by_label_string(self):
        assert self.game[[0, 0]]['Player 1'] == 2
        assert self.game[[0, 1]]['Player 1'] == 2
        assert self.game[[1, 0]]['Player 1'] == 4
        assert self.game[[1, 1]]['Player 1'] == 6
        assert self.game[[0, 0]]['Player 2'] == 3
        assert self.game[[0, 1]]['Player 2'] == 3
        assert self.game[[1, 0]]['Player 2'] == 5
        assert self.game[[1, 1]]['Player 2'] == 7

    def test_getting_payoff_by_player(self):
        player1 = self.game.players[0]
        player2 = self.game.players[1]
        assert self.game[[0, 0]][player1] == 2
        assert self.game[[0, 1]][player1] == 2
        assert self.game[[1, 0]][player1] == 4
        assert self.game[[1, 1]][player1] == 6
        assert self.game[[0, 0]][player2] == 3
        assert self.game[[0, 1]][player2] == 3
        assert self.game[[1, 0]][player2] == 5
        assert self.game[[1, 1]][player2] == 7

    def test_outcome_index_exception_int(self):
        self.assertRaises(KeyError, self.game[[0, 0]].__getitem__, "Not a player")
