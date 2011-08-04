import gambit

class TestGambitMixedStrategyGame(object):
	def setUp(self):
		self.game = gambit.read_game("test_games/mixed_strategy.nfg")
		self.p = self.game.mixed_profile()
		
	def tearDown(self):
		del self.game
		
	def test_initial_player_count(self):
		"Test to ensure 2 initial players"
		assert len(self.game.players) == 2
	
	def test_initial_game_title(self):
		"Test to ensure correct basic title"
		assert str(self.game) == "<Game ''>"
		
	def test_payoffs(self):
		"Test to ensure that payoffs are returned correctly"
		assert self.p.payoff(self.game.players[0]) == 0.66666666666666663
		
	def test_payoff_labels(self):
		"Test to ensure that payoffs are correctly labeled"
		#print self.p.payoff(self.game.players[0])
		#print self.p
		assert self.p.payoff(self.game.players[0]) == 0.66666666666666663

	def test_strategy_value(self):
		"Test expected payoff based on given strategy"
		assert self.p.strategy_value(self.game.players[0].strategies[2]) == 1.0	
