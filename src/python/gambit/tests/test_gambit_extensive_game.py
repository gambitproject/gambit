import gambit.lib.game

class TestGambitExtensiveGame(object):
	def setUp(self):
		self.game = gambit.new_tree()
	
	def tearDown(self):
		del self.game
		
	def test_initial_player_count(self):
		"Test to ensure 0 initial players"
		assert len(self.game.players) == 0
		
	def test_initial_game_title(self):
		"Test to ensure correct basic title"
		assert str(self.game) == "<Game ''>"
		
	def test_game_title_assignment(self):
		"Test to check title assignment"
		self.game.title = "A simple poker example"
		assert str(self.game) == "<Game 'A simple poker example'>"

	def test_game_add_players(self):
		"Test to add player"
		self.game.title = "A simple poker example"
		p = self.game.players.add("Alice")
		assert len(self.game.players) == 1
		assert str(self.game.players[0]) == "<Player [0] 'Alice' in game 'A simple poker example'>"
		assert str(p.label) == "Alice"
		