import gambit
import warnings

class TestGambitStrategicGame(object):
    def setUp(self):
        self.game = gambit.new_table([2,2])
    
    def tearDown(self):
        del self.game
        
    def test_initial_player_count(self):
        "Test to ensure 0 initial players"
        assert len(self.game.players) == 2
        
    def test_initial_game_title(self):
        "Test to ensure correct basic title"
        assert str(self.game) == "<Game ''>"
        
    def test_game_title_assignment(self):
        "Test to check title assignment"
        self.game.title = "A prisoner's dilemma game"
        assert str(self.game) == "<Game 'A prisoner's dilemma game'>"

    def test_game_add_players(self):
        "Test to add player"
        self.game.players[0].label = "Alphonse"
        self.game.players[1].label = "Gaston"
        p1 = self.game.players[0]
        assert p1.label == "Alphonse"
        p2 = self.game.players[1]
        assert p2.label == "Gaston"

    def test_game_add_duplicate_player_names(self):
        "Test to add player with preexisting name"
        self.game.players[0].label = "Alphonse"
        with warnings.catch_warnings(True) as w:
                self.game.players[1].label = "Alphonse"
                assert str(w[0].message) == "Another player with an identical label exists"
        p1 = self.game.players[0]
        assert p1.label == "Alphonse"
        p2 = self.game.players[1]
        assert p2.label == "Alphonse"

    def test_game_add_strategies(self):
        "Test to add strategies"
        assert len(self.game.players[0].strategies) == 2
        assert len(self.game.players[1].strategies) == 2
        
        self.game.players[0].strategies[0].label = "Cooperate"
        self.game.players[0].strategies[1].label = "Defect"
        assert self.game.players[0].strategies[0].label == "Cooperate"
        assert self.game.players[0].strategies[1].label == "Defect"
        self.game.players[1].strategies[0].label = "Cooperate"
        self.game.players[1].strategies[1].label = "Defect"
        assert self.game.players[1].strategies[0].label == "Cooperate"
        assert self.game.players[1].strategies[1].label == "Defect"

    def test_game_add_duplicate_strategy_names(self):
        "Test to add strategies with preexisting names"
        assert len(self.game.players[0].strategies) == 2
        assert len(self.game.players[1].strategies) == 2

        self.game.players[0].strategies[0].label = "Cooperate"
        with warnings.catch_warnings(True) as w:
                self.game.players[0].strategies[1].label = "Cooperate"
                assert str(w[0].message) == "This player has another strategy with an identical label"
        assert self.game.players[0].strategies[0].label == "Cooperate"
        assert self.game.players[0].strategies[1].label == "Cooperate"
        self.game.players[1].strategies[0].label = "Cooperate"
        self.game.players[1].strategies[1].label = "Defect"
        assert self.game.players[1].strategies[0].label == "Cooperate"
        assert self.game.players[1].strategies[1].label == "Defect"

    
