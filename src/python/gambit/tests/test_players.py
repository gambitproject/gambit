import gambit
import warnings
from nose.tools import assert_raises

class TestGambitPlayers(object):
    def setUp(self):
        self.game = gambit.new_table([2,2])
        self.game.players[0].label = "Alphonse"
        self.game.players[1].label = "Gaston"
    
    def tearDown(self):
        del self.game
        
    def test_initial_player_count(self):
        "Test to ensure 0 initial players"
        assert len(self.game.players) == 2
        
    def test_game_add_players(self):
        "Test to add player"
        p1 = self.game.players[0]
        assert p1.label == "Alphonse"
        p2 = self.game.players[1]
        assert p2.label == "Gaston"

    def test_game_add_duplicate_player_names(self):
        "Test to add player with preexisting name"
        with warnings.catch_warnings(True) as w:
                self.game.players[1].label = "Alphonse"
                assert str(w[0].message) == "Another player with an identical label exists"
        p1 = self.game.players[0]
        assert p1.label == "Alphonse"
        p2 = self.game.players[1]
        assert p2.label == "Alphonse"

    def test_game_players_index_by_string(self):
        "Test find a player by providing a label"
        p1 = self.game.players["Alphonse"]
        assert p1.label == "Alphonse"
        p2 = self.game.players["Gaston"]
        assert p2.label == "Gaston"

    def test_game_players_index_exception_int(self):
        "Test to verify when an index is out of range"
        assert_raises(IndexError, self.game.players.__getitem__, 3)

    def test_game_players_index_exception_string(self):
        "Test to verify when a player label is not in the list of players"
        assert_raises(IndexError, self.game.players.__getitem__, "None")

    def test_game_players_index_exception_player(self):
        "Test to verify when a player object is not in the players"
        self.game_2 = gambit.new_table([2,2])
        assert_raises(IndexError, self.game.players.__getitem__, self.game_2.players[0])

    def test_game_players_index_exception_player(self):
        "Test to verify when attempting to retrieve with invalid input"
        assert_raises(TypeError, self.game.players.__getitem__, 1.3)

