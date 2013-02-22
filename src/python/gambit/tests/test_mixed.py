import gambit
import fractions
from nose.tools import assert_raises
from gambit.lib.error import UndefinedOperationError

class TestGambitMixedStrategyGame(object):
    def setUp(self):
        self.game = gambit.new_table([2,2])
        self.game.players[0].label = "joe"
        self.game.players["joe"].strategies[0].label = "cooperate"
        self.game.players[1].label = "dan"
        self.game.players["dan"].strategies[1].label = "defect"
        
        self.profile_double = self.game.mixed_profile()
        self.profile_rational = self.game.mixed_profile(True)

        self.tree_game = gambit.read_game("test_games/mixed_behavior_game.efg")

        self.tree_profile_double = self.tree_game.mixed_profile()
        self.tree_profile_rational = self.tree_game.mixed_profile(True)

        
    def tearDown(self):
        del self.game
        del self.tree_game
        del self.profile_double
        del self.profile_rational
        del self.tree_profile_double
        del self.tree_profile_rational
            
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

    def test_as_behav_tree(self):
        "Test converting the profile to a behavior one"
        behav_double = self.tree_profile_double.as_behav()
        behav_rational = self.tree_profile_rational.as_behav()

        assert behav_double[0] == self.tree_profile_double[0]
        assert behav_double[1] == self.tree_profile_double[1]
        assert behav_double[2] == self.tree_profile_double[2]
        assert behav_double[3] == self.tree_profile_double[3]
        assert behav_double[4] == self.tree_profile_double[4]
        assert behav_double[5] == self.tree_profile_double[5]

        assert behav_rational[0] == self.tree_profile_rational[0]
        assert behav_rational[1] == self.tree_profile_rational[1]
        assert behav_rational[2] == self.tree_profile_rational[2]
        assert behav_rational[3] == self.tree_profile_rational[3]
        assert behav_rational[4] == self.tree_profile_rational[4]
        assert behav_rational[5] == self.tree_profile_rational[5]

    def test_as_behav_error(self):  
        "Test raising an error when trying to convert a profile from a strategic game"
        assert_raises(UndefinedOperationError, self.profile_double.as_behav)
        assert_raises(UndefinedOperationError, self.profile_rational.as_behav)

