import gambit
import warnings
import fractions
from nose.tools import assert_raises

class TestGambitPlayers(object):
    def setUp(self):
        self.strategic_game = gambit.new_table([2,2])
        self.strategic_game.players[0].label = "Alphonse"
        self.strategic_game.players[1].label = "Gaston"
        self.extensive_game = gambit.new_tree()
    
    def tearDown(self):
        del self.strategic_game
        
    def test_initial_player_count(self):
        "Test to ensure 0 initial players"
        assert len(self.strategic_game.players) == 2
        
    def test_game_add_players(self):
        "Test to add player"
        p1 = self.strategic_game.players[0]
        assert p1.label == "Alphonse"
        p2 = self.strategic_game.players[1]
        assert p2.label == "Gaston"

    def test_game_add_duplicate_player_names(self):
        "Test to add player with preexisting name"
        with warnings.catch_warnings(True) as w:
                self.strategic_game.players[1].label = "Alphonse"
                assert str(w[0].message) == "Another player with an identical label exists"
        p1 = self.strategic_game.players[0]
        assert p1.label == "Alphonse"
        p2 = self.strategic_game.players[1]
        assert p2.label == "Alphonse"

    def test_game_players_index_by_string(self):
        "Test find a player by providing a label"
        p1 = self.strategic_game.players["Alphonse"]
        assert p1.label == "Alphonse"
        p2 = self.strategic_game.players["Gaston"]
        assert p2.label == "Gaston"

    def test_game_players_index_exception_int(self):
        "Test to verify when an index is out of range"
        assert_raises(IndexError, self.strategic_game.players.__getitem__, 3)

    def test_game_players_index_exception_string(self):
        "Test to verify when a player label is not in the list of players"
        assert_raises(IndexError, self.strategic_game.players.__getitem__, "None")

    def test_game_players_index_exception_player(self):
        "Test to verify when a player object is not in the players"
        self.strategic_game_2 = gambit.new_table([2,2])
        assert_raises(IndexError, self.strategic_game.players.__getitem__, self.strategic_game_2.players[0])

    def test_game_players_index_exception_player(self):
        "Test to verify when attempting to retrieve with invalid input"
        assert_raises(TypeError, self.strategic_game.players.__getitem__, 1.3)

    def test_add_strategic_game_players(self):
        "Test to ensure that players can be added to a strategic game with a single strategy"
        assert len(self.strategic_game.players) == 2
        self.strategic_game.players.add("new player")
        assert len(self.strategic_game.players) == 3
        assert self.strategic_game.players[2].label == "new player"
        assert len(self.strategic_game.players[2].strategies) == 1
        self.strategic_game.players.add()
        assert len(self.strategic_game.players) == 4
        assert self.strategic_game.players[3].label == ""

    def test_add_extensive_game_players(self):
        "Test to ensure that players can be added to an extensive game with no moves"
        assert len(self.extensive_game.players) == 0
        self.extensive_game.players.add("new player")
        assert len(self.extensive_game.players) == 1
        assert len(self.extensive_game.players[0].infosets) == 0
        self.extensive_game.players.add("new player 2")
        assert len(self.extensive_game.players) == 2
        assert len(self.extensive_game.players[1].infosets) == 0
        assert self.extensive_game.players[0].label == "new player"
        assert self.extensive_game.players[1].label == "new player 2"

    def test_game_add_strategies_to_player_strategic_game(self):
        "Test to add a strategy to a player in a strategic game"
        assert len(self.strategic_game.players[0].strategies) == 2
        self.strategic_game.players[0].strategies.add("1st new strategy")
        assert len(self.strategic_game.players[0].strategies) == 3

        with warnings.catch_warnings(True) as w:
            self.strategic_game.players[0].strategies.add("1st new strategy")
            assert str(w[0].message) == "This player has another strategy with an identical label"
        assert len(self.strategic_game.players[0].strategies) == 4

    def test_game_add_strategies_to_player_extensive_game(self):
        "Test to ensure that an exception is raised when attempting to add a strategy to a player in an extensive game"
        self.extensive_game.players.add("Alice")
        assert_raises(TypeError, self.extensive_game.players[0].strategies.add, "Test")

    def test_player_get_min_payoff(self):
        "To test getting the minimum payoff for the players"
        game = gambit.read_game("test_games/payoff_game.nfg")
        assert game.players[0].min_payoff == fractions.Fraction(4,1)
        assert game.players["Player 1"].min_payoff == fractions.Fraction(4,1)
        assert game.players[1].min_payoff == fractions.Fraction(1,1)
        assert game.players["Player 2"].min_payoff == fractions.Fraction(1,1)

    def test_player_get_max_payoff(self):
        "To test getting the maximum payoff for the players"
        game = gambit.read_game("test_games/payoff_game.nfg")
        assert game.players[0].max_payoff == fractions.Fraction(10,1)
        assert game.players["Player 1"].max_payoff == fractions.Fraction(10,1)
        assert game.players[1].max_payoff == fractions.Fraction(8,1)
        assert game.players["Player 2"].max_payoff == fractions.Fraction(8,1)
