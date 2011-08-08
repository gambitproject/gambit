import gambit
import fractions

class TestGambitMixedStrategyGame(object):
    def setUp(self):
        self.game = gambit.new_table([2,2])
        self.game.players[0].label = "joe"
        self.game.players["joe"].strategies[0].label = "cooperate"
        self.game.players[1].label = "dan"
        self.game.players["dan"].strategies[1].label = "defect"
        
        self.profile_double = self.game.mixed_profile()
        self.profile_rational = self.game.mixed_profile(True)

        
    def tearDown(self):
        del self.game
        del self.profile_double
        del self.profile_rational
            

    def test_getting_profile(self):
        "Test to ensure that payoffs are returned correctly"
        assert self.profile_double.payoff(self.game.players[0]) == 0.0
        assert self.profile_rational.payoff(self.game.players[0]) == 0.0

    def test_payoffs(self):
        "Test to ensure that payoffs are returned correctly"
        assert self.profile_double.payoff(self.game.players[0]) == 0.0
        assert self.profile_rational.payoff(self.game.players[0]) == 0.0
        
    def test_finding_payoffs_by_string(self):
        "Test to find payoffs by string values"
        assert self.profile_double.payoff("joe") == 0.0 
        assert self.profile_rational.payoff("joe") == 0.0   

    def test_strategy_value(self):
        "Test to retrieve strategy value based on given strategy"
        assert self.profile_double.strategy_value(self.game.players[0].strategies[1]) == 0.0    
        assert self.profile_rational.strategy_value(self.game.players[0].strategies[1]) == 0.0  

    def test_strategy_value_by_string(self):
        "Test expected payoff based on given strategy"
        assert self.profile_double.strategy_value("defect") == 0.0
        assert self.profile_rational.strategy_value("defect") == 0.0

    def test_strategy_values(self):
        "Test retrieving strategy values based on player"
        assert self.profile_double.strategy_values(self.game.players[0]) == [0.0, 0.0]
        assert self.profile_rational.strategy_values(self.game.players[0]) == [0.0, 0.0]

    def test_strategy_values_by_string(self):
        "Test retrieving strategy values based on player"
        assert self.profile_double.strategy_values("joe") == [0.0, 0.0]
        assert self.profile_rational.strategy_values("joe") == [0.0, 0.0]
        
    def test_get_probabilities_strategy(self):
        "Test retrieving probabilities"
        assert self.profile_double[0] == 0.5
        assert self.profile_rational[0] == fractions.Fraction("1/2")


    def test_get_probabilities_player(self):
        "Test retrieving probabilities"
        assert self.profile_double[self.game.players[0]] == [0.5, 0.5]
        assert self.profile_rational[self.game.players[0]] == [fractions.Fraction("1/2"), fractions.Fraction("1/2")]
        
        assert self.profile_double[self.game.players[0]][0] == 0.5
        assert self.profile_rational[self.game.players[0]][0] == fractions.Fraction("1/2")

        assert self.profile_double[self.game.players[0]]["cooperate"] == 0.5
        assert self.profile_rational[self.game.players[0]]["cooperate"] == fractions.Fraction("1/2")


        
    def test_set_probabilities(self):
        "Test setting probabilities"
        self.profile_double[0] = 0.72
        assert self.profile_double[0] == 0.72
        self.profile_rational[0] = fractions.Fraction("2/9")
        assert self.profile_rational[0] == fractions.Fraction("2/9")
        
        self.profile_double[self.game.players[0]][1] = 0.72
        assert self.profile_double[self.game.players[0]][1] == 0.72
        self.profile_rational[self.game.players[0]][1] = fractions.Fraction("2/9")
        assert self.profile_rational[self.game.players[0]][1] == fractions.Fraction("2/9")
        
    def test_liap_values(self):
        "Test retrieving Lyapunov values"
        assert self.profile_double.liap_value() == 0.0
        assert self.profile_rational.liap_value() == fractions.Fraction("0")

