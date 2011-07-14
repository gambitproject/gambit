import gambit.lib.game
import nose.tools

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
		self.game.title = "A prisoner's dilemma game"
		assert str(self.game) == "<Game 'A prisoner's dilemma game'>"
		self.game.players[0].label = "Alphonse"
		self.game.players[1].label = "Gaston"
		p1 = self.game.players[0]
		assert p1.label == "Alphonse"
		p2 = self.game.players[1]
		assert p2.label == "Gaston"

	def test_game_add_strategies(self):
		"Test to add strategies"
		self.game.title = "A prisoner's dilemma game"
		self.game.players[0].label = "Alphonse"
		self.game.players[1].label = "Gaston"
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
	
