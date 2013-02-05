import gambit
import fractions

class TestGambitMixedBehavGame(object):
    def setUp(self):
        self.game = gambit.read_game("test_games/mixed_behavior_game.efg")
        
        self.profile_double = self.game.behav_profile()
        self.profile_rational = self.game.behav_profile(True)
        
    def tearDown(self):
        del self.game
        del self.profile_double
        del self.profile_rational
            

    def test_payoff(self):
        "Test to ensure that payoffs are returned correctly"        
        assert self.profile_double.payoff(self.game.players[0]) == 3.0
        assert self.profile_double.payoff(self.game.players[1]) == 3.0
        assert self.profile_double.payoff(self.game.players[2]) == 3.25
        assert self.profile_rational.payoff(self.game.players[0]) == fractions.Fraction(3,1)
        assert self.profile_rational.payoff(self.game.players[1]) == fractions.Fraction(3,1)
        assert self.profile_rational.payoff(self.game.players[2]) == fractions.Fraction(13,4)

        
    def test_payoff_by_string(self):
        "Test to find payoffs by string values"
        assert self.profile_double.payoff("Player 1") == 3.0
        assert self.profile_double.payoff("Player 2") == 3.0
        assert self.profile_double.payoff("Player 3") == 3.25
        assert self.profile_rational.payoff("Player 1") == fractions.Fraction(3,1)
        assert self.profile_rational.payoff("Player 2") == fractions.Fraction(3,1)
        assert self.profile_rational.payoff("Player 3") == fractions.Fraction(13,4)

    def test_is_defined_at(self):
        "Test to check if a infoset is defined"
        assert self.profile_double.is_defined_at(self.game.players[0].infosets[0])
        assert self.profile_double.is_defined_at(self.game.players[1].infosets[0])
        assert self.profile_double.is_defined_at(self.game.players[2].infosets[0])
        assert self.profile_rational.is_defined_at(self.game.players[0].infosets[0])
        assert self.profile_rational.is_defined_at(self.game.players[1].infosets[0])
        assert self.profile_rational.is_defined_at(self.game.players[2].infosets[0])

    def test_is_defined_at_by_string(self):
        "Test to check if a infoset is defined by string values"
        assert self.profile_double.is_defined_at("Infoset 1:1")
        assert self.profile_double.is_defined_at("Infoset 2:1")
        assert self.profile_double.is_defined_at("Infoset 3:1")
        assert self.profile_rational.is_defined_at("Infoset 1:1")
        assert self.profile_rational.is_defined_at("Infoset 2:1")
        assert self.profile_rational.is_defined_at("Infoset 3:1")

    def test_get_probabilities_action(self):
        "Test to retrieve probabilities from an action"
        assert self.profile_double[self.game.players[0].infosets[0].actions[0]] == 0.5
        assert self.profile_double[self.game.players[0].infosets[0].actions[1]] == 0.5
        assert self.profile_double[self.game.players[1].infosets[0].actions[0]] == 0.5
        assert self.profile_double[self.game.players[1].infosets[0].actions[1]] == 0.5
        assert self.profile_double[self.game.players[2].infosets[0].actions[0]] == 0.5
        assert self.profile_double[self.game.players[2].infosets[0].actions[1]] == 0.5
        assert self.profile_rational[self.game.players[0].infosets[0].actions[0]] == fractions.Fraction("1/2")
        assert self.profile_rational[self.game.players[0].infosets[0].actions[1]] == fractions.Fraction("1/2")
        assert self.profile_rational[self.game.players[1].infosets[0].actions[0]] == fractions.Fraction("1/2")
        assert self.profile_rational[self.game.players[1].infosets[0].actions[1]] == fractions.Fraction("1/2")
        assert self.profile_rational[self.game.players[2].infosets[0].actions[0]] == fractions.Fraction("1/2")
        assert self.profile_rational[self.game.players[2].infosets[0].actions[1]] == fractions.Fraction("1/2")

    def test_get_probabilities_action_by_string(self):
        "Test to retrieve probabilities from an action by string values"
        assert self.profile_double["U1"] == 0.5
        assert self.profile_double["D1"] == 0.5
        assert self.profile_double["U2"] == 0.5
        assert self.profile_double["D2"] == 0.5
        assert self.profile_double["U3"] == 0.5
        assert self.profile_double["D3"] == 0.5
        assert self.profile_rational["U1"] == fractions.Fraction("1/2")
        assert self.profile_rational["D1"] == fractions.Fraction("1/2")
        assert self.profile_rational["U2"] == fractions.Fraction("1/2")
        assert self.profile_rational["D2"] == fractions.Fraction("1/2")
        assert self.profile_rational["U3"] == fractions.Fraction("1/2")
        assert self.profile_rational["D3"] == fractions.Fraction("1/2")

    def test_get_probabilities_infoset(self):
        "Test to retrieve probabilities from an infoset"
        assert self.profile_double[self.game.players[0].infosets[0]] == [0.5, 0.5]
        assert self.profile_double[self.game.players[0].infosets[0]] == self.profile_double[self.game.players[0]][0]
        assert self.profile_double[self.game.players[1].infosets[0]] == [0.5, 0.5]
        assert self.profile_double[self.game.players[1].infosets[0]] == self.profile_double[self.game.players[1]][0]
        assert self.profile_double[self.game.players[2].infosets[0]] == [0.5, 0.5]
        assert self.profile_double[self.game.players[2].infosets[0]] == self.profile_double[self.game.players[2]][0]
        assert self.profile_rational[self.game.players[0].infosets[0]] == [fractions.Fraction("1/2"), fractions.Fraction("1/2")]
        assert self.profile_rational[self.game.players[0].infosets[0]] == self.profile_rational[self.game.players[0]][0]
        assert self.profile_rational[self.game.players[1].infosets[0]] == [fractions.Fraction("1/2"), fractions.Fraction("1/2")]
        assert self.profile_rational[self.game.players[1].infosets[0]] == self.profile_rational[self.game.players[1]][0]
        assert self.profile_rational[self.game.players[2].infosets[0]] == [fractions.Fraction("1/2"), fractions.Fraction("1/2")]
        assert self.profile_rational[self.game.players[2].infosets[0]] == self.profile_rational[self.game.players[2]][0]

    def test_get_probabilities_infoset_by_string(self):
        "Test to retrieve probabilities from an infoset by string values"
        assert self.profile_double[self.game.players[0]]["Infoset 1:1"] == [0.5, 0.5]
        assert self.profile_double["Infoset 1:1"] == [0.5, 0.5]
        assert self.profile_double[self.game.players[1]]["Infoset 2:1"] == [0.5, 0.5]
        assert self.profile_double["Infoset 2:1"] == [0.5, 0.5]
        assert self.profile_double[self.game.players[2]]["Infoset 3:1"] == [0.5, 0.5]
        assert self.profile_double["Infoset 3:1"] == [0.5, 0.5]
        assert self.profile_rational[self.game.players[0]]["Infoset 1:1"] == [fractions.Fraction("1/2"), fractions.Fraction("1/2")]
        assert self.profile_rational["Infoset 1:1"] == [fractions.Fraction("1/2"), fractions.Fraction("1/2")]
        assert self.profile_rational[self.game.players[1]]["Infoset 2:1"] == [fractions.Fraction("1/2"), fractions.Fraction("1/2")]
        assert self.profile_rational["Infoset 2:1"] == [fractions.Fraction("1/2"), fractions.Fraction("1/2")]
        assert self.profile_rational[self.game.players[2]]["Infoset 3:1"] == [fractions.Fraction("1/2"), fractions.Fraction("1/2")]
        assert self.profile_rational["Infoset 3:1"] == [fractions.Fraction("1/2"), fractions.Fraction("1/2")]

    def test_get_probabilities_player(self):
        "Test to retrieve probabilities from a player"
        assert self.profile_double[self.game.players[0]] == [[0.5, 0.5]]
        assert self.profile_double[self.game.players[1]] == [[0.5, 0.5]]
        assert self.profile_double[self.game.players[2]] == [[0.5, 0.5]]
        assert self.profile_rational[self.game.players[0]] == [[fractions.Fraction("1/2"), fractions.Fraction("1/2")]]
        assert self.profile_rational[self.game.players[1]] == [[fractions.Fraction("1/2"), fractions.Fraction("1/2")]]
        assert self.profile_rational[self.game.players[2]] == [[fractions.Fraction("1/2"), fractions.Fraction("1/2")]]

    def test_get_probabilities_player_by_string(self):
        "Test to retrieve probabilities from a player by string values"
        assert self.profile_double["Player 1"] == [[0.5, 0.5]]
        assert self.profile_double["Player 2"] == [[0.5, 0.5]]
        assert self.profile_double["Player 3"] == [[0.5, 0.5]]
        assert self.profile_rational["Player 1"] == [[fractions.Fraction("1/2"), fractions.Fraction("1/2")]]
        assert self.profile_rational["Player 2"] == [[fractions.Fraction("1/2"), fractions.Fraction("1/2")]]
        assert self.profile_rational["Player 3"] == [[fractions.Fraction("1/2"), fractions.Fraction("1/2")]]
        
    def test_set_probabilities_action(self):
        "Test to set probabilities"
        self.profile_double[0] = 0.72
        assert self.profile_double[0] == 0.72
        self.profile_double[1] = 0.28
        assert self.profile_double[1] == 0.28
        self.profile_double[2] = 0.42
        assert self.profile_double[2] == 0.42
        self.profile_double[3] = 0.58
        assert self.profile_double[3] == 0.58
        self.profile_double[4] = 0.02
        assert self.profile_double[4] == 0.02
        self.profile_double[5] = 0.98
        assert self.profile_double[5] == 0.98
        self.profile_rational[0] = fractions.Fraction("2/9")
        assert self.profile_rational[0] == fractions.Fraction("2/9")
        self.profile_rational[1] = fractions.Fraction("7/9")
        assert self.profile_rational[1] == fractions.Fraction("7/9")
        self.profile_rational[2] = fractions.Fraction("4/13")
        assert self.profile_rational[2] == fractions.Fraction("4/13")
        self.profile_rational[3] = fractions.Fraction("9/13")
        assert self.profile_rational[3] == fractions.Fraction("9/13")
        self.profile_rational[4] = fractions.Fraction("1/98")
        assert self.profile_rational[4] == fractions.Fraction("1/98")
        self.profile_rational[5] = fractions.Fraction("97/98")
        assert self.profile_rational[5] == fractions.Fraction("97/98")

    def test_set_probabilities_action_by_string(self):
        "Test to set probabilities by string values"
        self.profile_double["U1"] = 0.72
        assert self.profile_double["U1"] == 0.72
        self.profile_double["D1"] = 0.28
        assert self.profile_double["D1"] == 0.28
        self.profile_double["U2"] = 0.42
        assert self.profile_double["U2"] == 0.42
        self.profile_double["D2"] = 0.58
        assert self.profile_double["D2"] == 0.58
        self.profile_double["U3"] = 0.02
        assert self.profile_double["U3"] == 0.02
        self.profile_double["D3"] = 0.98
        assert self.profile_double["D3"] == 0.98
        assert self.profile_double == [0.72, 0.28, 0.42, 0.58, 0.02, 0.98]

        self.profile_rational["U1"] = fractions.Fraction("2/9")
        assert self.profile_rational["U1"] == fractions.Fraction("2/9")
        self.profile_rational["D1"] = fractions.Fraction("7/9")
        assert self.profile_rational["D1"] == fractions.Fraction("7/9")
        self.profile_rational["U2"] = fractions.Fraction("4/13")
        assert self.profile_rational["U2"] == fractions.Fraction("4/13")
        self.profile_rational["D2"] = fractions.Fraction("9/13")
        assert self.profile_rational["D2"] == fractions.Fraction("9/13")
        self.profile_rational["U3"] = fractions.Fraction("1/98")
        assert self.profile_rational["U3"] == fractions.Fraction("1/98")
        self.profile_rational["D3"] = fractions.Fraction("97/98")
        assert self.profile_rational["D3"] == fractions.Fraction("97/98")
        print self.profile_rational
        assert self.profile_rational == [fractions.Fraction("2/9"), 
                                        fractions.Fraction("7/9"), 
                                        fractions.Fraction("4/13"), 
                                        fractions.Fraction("9/13"), 
                                        fractions.Fraction("1/98"), 
                                        fractions.Fraction("97/98")]
    
    def test_set_probabilities_infoset(self):
        "Test to set probabilities to an infoset"
        self.profile_double[self.game.players[0]][0] = [0.72, 0.28]
        assert self.profile_double[self.game.players[0]][0] == [0.72, 0.28]
        self.profile_double[self.game.players[1]][0] = [0.42, 0.58]
        assert self.profile_double[self.game.players[1]][0] == [0.42, 0.58]
        self.profile_double[self.game.players[2]][0] = [0.02, 0.98]
        assert self.profile_double[self.game.players[2]][0] == [0.02, 0.98]

        self.profile_rational[self.game.players[0]][0] = [fractions.Fraction("2/9"), fractions.Fraction("7/9")]
        assert self.profile_rational[self.game.players[0]][0] == [fractions.Fraction("2/9"), fractions.Fraction("7/9")]
        self.profile_rational[self.game.players[0]][0] = [fractions.Fraction("4/13"), fractions.Fraction("9/13")]
        assert self.profile_rational[self.game.players[0]][0] == [fractions.Fraction("4/13"), fractions.Fraction("9/13")]
        self.profile_rational[self.game.players[0]][0] = [fractions.Fraction("1/98"), fractions.Fraction("97/98")]
        assert self.profile_rational[self.game.players[0]][0] == [fractions.Fraction("1/98"), fractions.Fraction("97/98")]

    def test_set_probabilities_infoset_by_string(self):
        "Test to set probabilities to an infoset by string values"
        self.profile_double["Infoset 1:1"] = [0.72, 0.28]
        assert self.profile_double["Infoset 1:1"] == [0.72, 0.28]
        self.profile_double["Infoset 2:1"] = [0.42, 0.58]
        assert self.profile_double["Infoset 2:1"] == [0.42, 0.58]
        self.profile_double["Infoset 3:1"] = [0.02, 0.98]
        assert self.profile_double["Infoset 3:1"] == [0.02, 0.98]

        self.profile_rational["Infoset 1:1"] = [fractions.Fraction("2/9"), fractions.Fraction("7/9")]
        assert self.profile_rational["Infoset 1:1"] == [fractions.Fraction("2/9"), fractions.Fraction("7/9")]
        self.profile_rational["Infoset 2:1"] = [fractions.Fraction("4/13"), fractions.Fraction("9/13")]
        assert self.profile_rational["Infoset 2:1"] == [fractions.Fraction("4/13"), fractions.Fraction("9/13")]
        self.profile_rational["Infoset 3:1"] = [fractions.Fraction("1/98"), fractions.Fraction("97/98")]
        assert self.profile_rational["Infoset 3:1"] == [fractions.Fraction("1/98"), fractions.Fraction("97/98")]

    def test_set_probabilities_player(self):
        "Test to set probabilities to a player"
        self.profile_double[self.game.players[0]] = [[0.72, 0.28]]
        assert self.profile_double[self.game.players[0]] == [[0.72, 0.28]]
        self.profile_double[self.game.players[1]] = [[0.42, 0.58]]
        assert self.profile_double[self.game.players[1]] == [[0.42, 0.58]]
        self.profile_double[self.game.players[2]] = [[0.02, 0.98]]
        assert self.profile_double[self.game.players[2]] == [[0.02, 0.98]]

        self.profile_rational[self.game.players[0]] = [[fractions.Fraction("2/9"), fractions.Fraction("7/9")]]
        assert self.profile_rational[self.game.players[0]] == [[fractions.Fraction("2/9"), fractions.Fraction("7/9")]]
        self.profile_rational[self.game.players[1]] = [[fractions.Fraction("4/13"), fractions.Fraction("9/13")]]
        assert self.profile_rational[self.game.players[1]] == [[fractions.Fraction("4/13"), fractions.Fraction("9/13")]]
        self.profile_rational[self.game.players[2]] = [[fractions.Fraction("1/98"), fractions.Fraction("97/98")]]
        assert self.profile_rational[self.game.players[2]] == [[fractions.Fraction("1/98"), fractions.Fraction("97/98")]]

    def test_set_probabilities_player_by_string(self):
        "Test to set probabilities to a player by string values"
        self.profile_double["Player 1"] = [[0.72, 0.28]]
        assert self.profile_double["Player 1"] == [[0.72, 0.28]]
        self.profile_double["Player 2"] = [[0.42, 0.58]]
        assert self.profile_double["Player 2"] == [[0.42, 0.58]]
        self.profile_double["Player 3"] = [[0.02, 0.98]]
        assert self.profile_double["Player 3"] == [[0.02, 0.98]]

        self.profile_rational["Player 1"] = [[fractions.Fraction("2/9"), fractions.Fraction("7/9")]]
        assert self.profile_rational["Player 1"] == [[fractions.Fraction("2/9"), fractions.Fraction("7/9")]]
        self.profile_rational["Player 2"] = [[fractions.Fraction("4/13"), fractions.Fraction("9/13")]]
        assert self.profile_rational["Player 2"] == [[fractions.Fraction("4/13"), fractions.Fraction("9/13")]]
        self.profile_rational["Player 3"] = [[fractions.Fraction("1/98"), fractions.Fraction("97/98")]]
        assert self.profile_rational["Player 3"] == [[fractions.Fraction("1/98"), fractions.Fraction("97/98")]]

    def test_infoset_prob(self):
        "Test to retrieve the probability associated to an infoset"
        assert self.profile_double.realiz_prob(self.game.players[0].infosets[0]) == 1.0
        assert self.profile_double.realiz_prob(self.game.players[1].infosets[0]) == 1.0
        assert self.profile_double.realiz_prob(self.game.players[2].infosets[0]) == 1.0
        assert self.profile_rational.realiz_prob(self.game.players[0].infosets[0]) == fractions.Fraction("1/1")
        assert self.profile_rational.realiz_prob(self.game.players[1].infosets[0]) == fractions.Fraction("1/1")
        assert self.profile_rational.realiz_prob(self.game.players[2].infosets[0]) == fractions.Fraction("1/1")

    def test_infoset_prob_by_string(self):
        "Test to retrieve the probability associated to an infoset by string values"
        assert self.profile_double.realiz_prob("Infoset 1:1") == 1.0
        assert self.profile_double.realiz_prob("Infoset 2:1") == 1.0
        assert self.profile_double.realiz_prob("Infoset 3:1") == 1.0
        assert self.profile_rational.realiz_prob("Infoset 1:1") == fractions.Fraction("1/1")
        assert self.profile_rational.realiz_prob("Infoset 2:1") == fractions.Fraction("1/1")
        assert self.profile_rational.realiz_prob("Infoset 3:1") == fractions.Fraction("1/1")

    def test_infoset_payoff(self):
        "Test to retrieve expected payoff associated to an infoset"
        assert self.profile_double.payoff(self.game.players[0].infosets[0]) == 3.0
        assert self.profile_double.payoff(self.game.players[1].infosets[0]) == 3.0
        assert self.profile_double.payoff(self.game.players[2].infosets[0]) == 3.25
        assert self.profile_rational.payoff(self.game.players[0].infosets[0]) == fractions.Fraction("3/1")
        assert self.profile_rational.payoff(self.game.players[1].infosets[0]) == fractions.Fraction("3/1")
        assert self.profile_rational.payoff(self.game.players[2].infosets[0]) == fractions.Fraction("13/4")

    def test_infoset_payoff_by_string(self):
        "Test to retrieve expected payoff associated to an infoset by string values"
        assert self.profile_double.payoff("Infoset 1:1") == 3.0
        assert self.profile_double.payoff("Infoset 2:1") == 3.0
        assert self.profile_double.payoff("Infoset 3:1") == 3.25
        assert self.profile_rational.payoff("Infoset 1:1") == fractions.Fraction("3/1")
        assert self.profile_rational.payoff("Infoset 2:1") == fractions.Fraction("3/1")
        assert self.profile_rational.payoff("Infoset 3:1") == fractions.Fraction("13/4")

    def test_action_payoff(self):
        "Test to retrieve expected payoff associated to an action"
        assert self.profile_double.payoff(self.game.players[0].infosets[0].actions[0]) == 3.0
        assert self.profile_double.payoff(self.game.players[0].infosets[0].actions[1]) == 3.0
        assert self.profile_double.payoff(self.game.players[1].infosets[0].actions[0]) == 3.0
        assert self.profile_double.payoff(self.game.players[1].infosets[0].actions[1]) == 3.0
        assert self.profile_double.payoff(self.game.players[2].infosets[0].actions[0]) == 3.5
        assert self.profile_double.payoff(self.game.players[2].infosets[0].actions[1]) == 3.0

        assert self.profile_rational.payoff(self.game.players[0].infosets[0].actions[0]) == fractions.Fraction("3/1")
        assert self.profile_rational.payoff(self.game.players[0].infosets[0].actions[1]) == fractions.Fraction("3/1")
        assert self.profile_rational.payoff(self.game.players[1].infosets[0].actions[0]) == fractions.Fraction("3/1")
        assert self.profile_rational.payoff(self.game.players[1].infosets[0].actions[1]) == fractions.Fraction("3/1")
        assert self.profile_rational.payoff(self.game.players[2].infosets[0].actions[0]) == fractions.Fraction("7/2")
        assert self.profile_rational.payoff(self.game.players[2].infosets[0].actions[1]) == fractions.Fraction("3/1")

    def test_action_value_by_string(self):
        "Test to retrieve expected payoff associated to an action by string values"
        assert self.profile_double.payoff("U1") == 3.0
        assert self.profile_double.payoff("D1") == 3.0
        assert self.profile_double.payoff("U2") == 3.0
        assert self.profile_double.payoff("D2") == 3.0
        assert self.profile_double.payoff("U3") == 3.5
        assert self.profile_double.payoff("D3") == 3.0

        assert self.profile_rational.payoff("U1") == fractions.Fraction("3/1")
        assert self.profile_rational.payoff("D1") == fractions.Fraction("3/1")
        assert self.profile_rational.payoff("U2") == fractions.Fraction("3/1")
        assert self.profile_rational.payoff("D2") == fractions.Fraction("3/1")
        assert self.profile_rational.payoff("U3") == fractions.Fraction("7/2")
        assert self.profile_rational.payoff("D3") == fractions.Fraction("3/1")

    def test_regret(self):
        "Test to retrieve regret value associated to an action"
        assert self.profile_double.regret(self.game.players[0].infosets[0].actions[0]) == 0.0
        assert self.profile_double.regret(self.game.players[0].infosets[0].actions[1]) == 0.0
        assert self.profile_double.regret(self.game.players[1].infosets[0].actions[0]) == 0.0
        assert self.profile_double.regret(self.game.players[1].infosets[0].actions[1]) == 0.0
        assert self.profile_double.regret(self.game.players[2].infosets[0].actions[0]) == 0.25
        assert self.profile_double.regret(self.game.players[2].infosets[0].actions[1]) == -0.25

        assert self.profile_rational.regret(self.game.players[0].infosets[0].actions[0]) == fractions.Fraction("0/1")
        assert self.profile_rational.regret(self.game.players[0].infosets[0].actions[1]) == fractions.Fraction("0/1")
        assert self.profile_rational.regret(self.game.players[1].infosets[0].actions[0]) == fractions.Fraction("0/1")
        assert self.profile_rational.regret(self.game.players[1].infosets[0].actions[1]) == fractions.Fraction("0/1")
        assert self.profile_rational.regret(self.game.players[2].infosets[0].actions[0]) == fractions.Fraction("1/4")
        assert self.profile_rational.regret(self.game.players[2].infosets[0].actions[1]) == fractions.Fraction(-1, 4)

    def test_regret_by_string(self):
        "Test to retrieve regret value associated to an action by string values"
        assert self.profile_double.regret("U1") == 0.0
        assert self.profile_double.regret("D1") == 0.0
        assert self.profile_double.regret("U2") == 0.0
        assert self.profile_double.regret("D2") == 0.0
        assert self.profile_double.regret("U3") == 0.25
        assert self.profile_double.regret("D3") == -0.25

        assert self.profile_rational.regret("U1") == fractions.Fraction("0/1")
        assert self.profile_rational.regret("D1") == fractions.Fraction("0/1")
        assert self.profile_rational.regret("U2") == fractions.Fraction("0/1")
        assert self.profile_rational.regret("D2") == fractions.Fraction("0/1")
        assert self.profile_rational.regret("U3") == fractions.Fraction(1, 4)
        assert self.profile_rational.regret("D3") == fractions.Fraction(-1, 4)
        
    def test_liap_values(self):
        "Test to retrieve Lyapunov values"
        assert self.profile_double.liap_value() == 0.0625
        assert self.profile_rational.liap_value() == fractions.Fraction("1/16")

    def test_as_mixed(self):
        "Test converting the profile to a mixed strategy one"
        mixed_double = self.profile_double.as_mixed()
        mixed_rational = self.profile_rational.as_mixed()

        assert mixed_double[0] == self.profile_double[0]
        assert mixed_double[1] == self.profile_double[1]
        assert mixed_double[2] == self.profile_double[2]
        assert mixed_double[3] == self.profile_double[3]
        assert mixed_double[4] == self.profile_double[4]
        assert mixed_double[5] == self.profile_double[5]

        assert mixed_rational[0] == self.profile_rational[0]
        assert mixed_rational[1] == self.profile_rational[1]
        assert mixed_rational[2] == self.profile_rational[2]
        assert mixed_rational[3] == self.profile_rational[3]
        assert mixed_rational[4] == self.profile_rational[4]
        assert mixed_rational[5] == self.profile_rational[5]

    def test_node_belief(self):
        "Test calculating belief probabilities on a node"
        self.profile_double[0] = 0.8
        self.profile_double[1] = 0.2
        self.profile_double[2] = 1.0
        self.profile_double[3] = 1.5
        self.profile_double[4] = 0.0
        self.profile_double[5] = 0.4
        assert self.profile_double.belief(self.game.root) == 1
        # Comparisons using 1e-13 as an arbitrary epsilon due to errors
        assert abs(self.profile_double.belief(self.game.infosets[1].members[0]) - 0.8) < 1e-13
        assert abs(self.profile_double.belief(self.game.infosets[1].members[1]) - 0.2) < 1e-13
        assert abs(self.profile_double.belief(self.game.infosets[2].members[0]) - 0.32) < 1e-13
        assert abs(self.profile_double.belief(self.game.infosets[2].members[1]) - 0.48) < 1e-13
        assert abs(self.profile_double.belief(self.game.infosets[2].members[2]) - 0.08) < 1e-13
        assert abs(self.profile_double.belief(self.game.infosets[2].members[3]) - 0.12) < 1e-13

        self.profile_rational[0] = fractions.Fraction(4,5)
        self.profile_rational[1] = fractions.Fraction(1,5)
        self.profile_rational[2] = fractions.Fraction(1,1)
        self.profile_rational[3] = fractions.Fraction(3,2)
        self.profile_rational[4] = fractions.Fraction(0,1)
        self.profile_rational[5] = fractions.Fraction(2,5)
        assert self.profile_rational.belief(self.game.root) == fractions.Fraction(1,1)
        assert self.profile_rational.belief(self.game.infosets[1].members[0]) == fractions.Fraction(4,5)
        assert self.profile_rational.belief(self.game.infosets[1].members[1]) == fractions.Fraction(1,5)
        assert self.profile_rational.belief(self.game.infosets[2].members[0]) == fractions.Fraction(8,25)
        assert self.profile_rational.belief(self.game.infosets[2].members[1]) == fractions.Fraction(12,25)
        assert self.profile_rational.belief(self.game.infosets[2].members[2]) == fractions.Fraction(2,25)
        assert self.profile_rational.belief(self.game.infosets[2].members[3]) == fractions.Fraction(3,25)

    def test_infoset_belief(self):
        "Test calculating belief probabilities on an infoset"
        self.profile_double[0] = 0.8
        self.profile_double[1] = 0.2
        self.profile_double[2] = 0.4
        self.profile_double[3] = 0.6
        self.profile_double[4] = 0.0
        self.profile_double[5] = 1.0
        assert self.profile_double.belief(self.game.infosets[0])[0] == 1.0
        for i in self.game.infosets:
            belief = self.profile_double.belief(i)
            for n in xrange(0, len(i.members)):
                assert self.profile_double.belief(i.members[n]) == belief[n]
            assert abs(sum(belief) - 1.0) < 1e-13

        self.profile_rational[0] = fractions.Fraction(4,5)
        self.profile_rational[1] = fractions.Fraction(1,5)
        self.profile_rational[2] = fractions.Fraction(2,5)
        self.profile_rational[3] = fractions.Fraction(3,5)
        self.profile_rational[4] = fractions.Fraction(0,1)
        self.profile_rational[5] = fractions.Fraction(1,1)
        assert self.profile_rational.belief(self.game.infosets[0])[0] == fractions.Fraction(1,1)
        for i in self.game.infosets:
            belief = self.profile_rational.belief(i)
            for n in xrange(0, len(i.members)):
                assert self.profile_rational.belief(i.members[n]) == belief[n]
            assert sum(belief) == fractions.Fraction(1,1)
