import unittest

import pygambit as gbt


class TestGambitMixedBehavGame(unittest.TestCase):
    def setUp(self):
        self.game = gbt.Game.read_game(
            "test_games/mixed_behavior_game.efg"
        )
        self.profile_double = self.game.mixed_behavior_profile()
        self.profile_rational = self.game.mixed_behavior_profile(True)

        self.game_with_chance = gbt.Game.read_game(
            "test_games/complicated_extensive_game.efg"
        )
        self.profile_double_with_chance = self.game_with_chance.mixed_behavior_profile()
        self.profile_rational_with_chance = self.game_with_chance.mixed_behavior_profile(True)

    def tearDown(self):
        del self.game
        del self.profile_double
        del self.profile_rational

    def test_payoff(self):
        "Test to ensure that payoffs are returned correctly"
        assert self.profile_double.payoff(self.game.players[0]) == 3.0
        assert self.profile_double.payoff(self.game.players[1]) == 3.0
        assert self.profile_double.payoff(self.game.players[2]) == 3.25
        assert (
            self.profile_rational.payoff(self.game.players[0]) ==
            gbt.Rational(3, 1)
        )
        assert (
            self.profile_rational.payoff(self.game.players[1]) ==
            gbt.Rational(3, 1)
        )
        assert (
            self.profile_rational.payoff(self.game.players[2]) ==
            gbt.Rational(13, 4)
        )

    def test_payoff_by_string(self):
        "Test to find payoffs by string values"
        assert self.profile_double.payoff("Player 1") == 3.0
        assert self.profile_double.payoff("Player 2") == 3.0
        assert self.profile_double.payoff("Player 3") == 3.25
        assert (
            self.profile_rational.payoff("Player 1") ==
            gbt.Rational(3, 1)
        )
        assert (
            self.profile_rational.payoff("Player 2") ==
            gbt.Rational(3, 1)
        )
        assert (
            self.profile_rational.payoff("Player 3") ==
            gbt.Rational(13, 4)
        )

    def test_is_defined_at(self):
        "Test to check if an infoset is defined"
        assert self.profile_double.is_defined_at(
            self.game.players[0].infosets[0]
        )
        assert self.profile_double.is_defined_at(
            self.game.players[1].infosets[0]
        )
        assert self.profile_double.is_defined_at(
            self.game.players[2].infosets[0]
        )
        assert self.profile_rational.is_defined_at(
            self.game.players[0].infosets[0]
        )
        assert self.profile_rational.is_defined_at(
            self.game.players[1].infosets[0]
        )
        assert self.profile_rational.is_defined_at(
            self.game.players[2].infosets[0]
        )

    def test_is_defined_at_by_string(self):
        "Test to check if an infoset is defined by string values"
        assert self.profile_double.is_defined_at("Infoset 1:1")
        assert self.profile_double.is_defined_at("Infoset 2:1")
        assert self.profile_double.is_defined_at("Infoset 3:1")
        assert self.profile_rational.is_defined_at("Infoset 1:1")
        assert self.profile_rational.is_defined_at("Infoset 2:1")
        assert self.profile_rational.is_defined_at("Infoset 3:1")

    def test_get_probabilities_action(self):
        "Test to retrieve probabilities from an action"
        assert (
            self.profile_double[self.game.players[0].infosets[0].actions[0]]
            == 0.5
        )
        assert (
            self.profile_double[self.game.players[0].infosets[0].actions[1]]
            == 0.5
        )
        assert (
            self.profile_double[self.game.players[1].infosets[0].actions[0]]
            == 0.5
        )
        assert (
            self.profile_double[self.game.players[1].infosets[0].actions[1]]
            == 0.5
        )
        assert (
            self.profile_double[self.game.players[2].infosets[0].actions[0]]
            == 0.5
        )
        assert (
            self.profile_double[self.game.players[2].infosets[0].actions[1]]
            == 0.5
        )
        assert (
            self.profile_rational[self.game.players[0].infosets[0].actions[0]]
            == gbt.Rational("1/2")
        )
        assert (
            self.profile_rational[self.game.players[0].infosets[0].actions[1]]
            == gbt.Rational("1/2")
        )
        assert (
            self.profile_rational[self.game.players[1].infosets[0].actions[0]]
            == gbt.Rational("1/2")
        )
        assert (
            self.profile_rational[self.game.players[1].infosets[0].actions[1]]
            == gbt.Rational("1/2")
        )
        assert (
            self.profile_rational[self.game.players[2].infosets[0].actions[0]]
            == gbt.Rational("1/2")
        )
        assert (
            self.profile_rational[self.game.players[2].infosets[0].actions[1]]
            == gbt.Rational("1/2")
        )

    def test_get_probabilities_action_by_string(self):
        "Test to retrieve probabilities from an action by string values"
        assert self.profile_double["U1"] == 0.5
        assert self.profile_double["D1"] == 0.5
        assert self.profile_double["U2"] == 0.5
        assert self.profile_double["D2"] == 0.5
        assert self.profile_double["U3"] == 0.5
        assert self.profile_double["D3"] == 0.5
        assert self.profile_rational["U1"] == gbt.Rational("1/2")
        assert self.profile_rational["D1"] == gbt.Rational("1/2")
        assert self.profile_rational["U2"] == gbt.Rational("1/2")
        assert self.profile_rational["D2"] == gbt.Rational("1/2")
        assert self.profile_rational["U3"] == gbt.Rational("1/2")
        assert self.profile_rational["D3"] == gbt.Rational("1/2")

    def test_get_probabilities_infoset(self):
        "Test to retrieve probabilities from an infoset"
        assert (
            self.profile_double[self.game.players[0].infosets[0]] == [0.5, 0.5]
        )
        assert (
            self.profile_double[self.game.players[1].infosets[0]] == [0.5, 0.5]
        )
        assert (
            self.profile_double[self.game.players[2].infosets[0]] ==
            [0.5, 0.5]
        )
        assert (
            self.profile_rational[self.game.players[0].infosets[0]] ==
            [gbt.Rational("1/2"), gbt.Rational("1/2")]
        )
        assert (
            self.profile_rational[self.game.players[1].infosets[0]] ==
            [gbt.Rational("1/2"), gbt.Rational("1/2")]
        )
        assert (
            self.profile_rational[self.game.players[2].infosets[0]] ==
            [gbt.Rational("1/2"), gbt.Rational("1/2")]
        )

    def test_get_probabilities_infoset_by_string(self):
        "Test to retrieve probabilities from an infoset by string values"
        assert (
            self.profile_double[self.game.players[0]]["Infoset 1:1"] ==
            [0.5, 0.5]
        )
        assert self.profile_double["Infoset 1:1"] == [0.5, 0.5]
        assert (
            self.profile_double[self.game.players[1]]["Infoset 2:1"] ==
            [0.5, 0.5]
        )
        assert self.profile_double["Infoset 2:1"] == [0.5, 0.5]
        assert (
            self.profile_double[self.game.players[2]]["Infoset 3:1"] ==
            [0.5, 0.5]
        )
        assert self.profile_double["Infoset 3:1"] == [0.5, 0.5]
        assert (
            self.profile_rational[self.game.players[0]]["Infoset 1:1"] ==
            [gbt.Rational("1/2"), gbt.Rational("1/2")]
        )
        assert (
            self.profile_rational["Infoset 1:1"] ==
            [gbt.Rational("1/2"), gbt.Rational("1/2")]
        )
        assert (
            self.profile_rational[self.game.players[1]]["Infoset 2:1"] ==
            [gbt.Rational("1/2"), gbt.Rational("1/2")]
        )
        assert (
            self.profile_rational["Infoset 2:1"] ==
            [gbt.Rational("1/2"), gbt.Rational("1/2")]
        )
        assert (
            self.profile_rational[self.game.players[2]]["Infoset 3:1"] ==
            [gbt.Rational("1/2"), gbt.Rational("1/2")]
        )
        assert (
            self.profile_rational["Infoset 3:1"] ==
            [gbt.Rational("1/2"), gbt.Rational("1/2")]
        )

    def test_get_probabilities_player(self):
        "Test to retrieve probabilities from a player"
        assert self.profile_double[self.game.players[0]] == [[0.5, 0.5]]
        assert self.profile_double[self.game.players[1]] == [[0.5, 0.5]]
        assert self.profile_double[self.game.players[2]] == [[0.5, 0.5]]
        assert self.profile_rational[self.game.players[0]] == [
            [gbt.Rational("1/2"), gbt.Rational("1/2")]
        ]
        assert self.profile_rational[self.game.players[1]] == [
            [gbt.Rational("1/2"), gbt.Rational("1/2")]
        ]
        assert self.profile_rational[self.game.players[2]] == [
            [gbt.Rational("1/2"), gbt.Rational("1/2")]
        ]

    def test_get_probabilities_player_by_string(self):
        "Test to retrieve probabilities from a player by string values"
        assert self.profile_double["Player 1"] == [[0.5, 0.5]]
        assert self.profile_double["Player 2"] == [[0.5, 0.5]]
        assert self.profile_double["Player 3"] == [[0.5, 0.5]]
        assert self.profile_rational["Player 1"] == [
            [gbt.Rational("1/2"), gbt.Rational("1/2")]
        ]
        assert self.profile_rational["Player 2"] == [
            [gbt.Rational("1/2"), gbt.Rational("1/2")]
        ]
        assert self.profile_rational["Player 3"] == [
            [gbt.Rational("1/2"), gbt.Rational("1/2")]
        ]

    def test_set_probabilities_action(self):
        "Test to set probabilities"
        self.profile_double[self.game.actions[0]] = 0.72
        assert self.profile_double[self.game.actions[0]] == 0.72
        self.profile_double[self.game.actions[1]] = 0.28
        assert self.profile_double[self.game.actions[1]] == 0.28
        self.profile_double[self.game.actions[2]] = 0.42
        assert self.profile_double[self.game.actions[2]] == 0.42
        self.profile_double[self.game.actions[3]] = 0.58
        assert self.profile_double[self.game.actions[3]] == 0.58
        self.profile_double[self.game.actions[4]] = 0.02
        assert self.profile_double[self.game.actions[4]] == 0.02
        self.profile_double[self.game.actions[5]] = 0.98
        assert self.profile_double[self.game.actions[5]] == 0.98
        self.profile_rational[self.game.actions[0]] = gbt.Rational("2/9")
        assert self.profile_rational[self.game.actions[0]] == gbt.Rational("2/9")
        self.profile_rational[self.game.actions[1]] = gbt.Rational("7/9")
        assert self.profile_rational[self.game.actions[1]] == gbt.Rational("7/9")
        self.profile_rational[self.game.actions[2]] = gbt.Rational("4/13")
        assert self.profile_rational[self.game.actions[2]] == gbt.Rational("4/13")
        self.profile_rational[self.game.actions[3]] = gbt.Rational("9/13")
        assert self.profile_rational[self.game.actions[3]] == gbt.Rational("9/13")
        self.profile_rational[self.game.actions[4]] = gbt.Rational("1/98")
        assert self.profile_rational[self.game.actions[4]] == gbt.Rational("1/98")
        self.profile_rational[self.game.actions[5]] = gbt.Rational("97/98")
        assert self.profile_rational[self.game.actions[5]] == gbt.Rational("97/98")

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

        self.profile_rational["U1"] = gbt.Rational("2/9")
        assert self.profile_rational["U1"] == gbt.Rational("2/9")
        self.profile_rational["D1"] = gbt.Rational("7/9")
        assert self.profile_rational["D1"] == gbt.Rational("7/9")
        self.profile_rational["U2"] = gbt.Rational("4/13")
        assert self.profile_rational["U2"] == gbt.Rational("4/13")
        self.profile_rational["D2"] = gbt.Rational("9/13")
        assert self.profile_rational["D2"] == gbt.Rational("9/13")
        self.profile_rational["U3"] = gbt.Rational("1/98")
        assert self.profile_rational["U3"] == gbt.Rational("1/98")
        self.profile_rational["D3"] = gbt.Rational("97/98")
        assert self.profile_rational["D3"] == gbt.Rational("97/98")

    def test_set_probabilities_infoset(self):
        "Test to set probabilities to an infoset"
        self.profile_double[self.game.players[0].infosets[0]] = [0.72, 0.28]
        assert self.profile_double[self.game.players[0].infosets[0]] == [0.72, 0.28]
        self.profile_double[self.game.players[1].infosets[0]] = [0.42, 0.58]
        assert self.profile_double[self.game.players[1].infosets[0]] == [0.42, 0.58]
        self.profile_double[self.game.players[2].infosets[0]] = [0.02, 0.98]
        assert self.profile_double[self.game.players[2].infosets[0]] == [0.02, 0.98]

        self.profile_rational[self.game.players[0].infosets[0]] = [
            gbt.Rational("2/9"), gbt.Rational("7/9")
        ]
        assert (
            self.profile_rational[self.game.players[0].infosets[0]] ==
            [gbt.Rational("2/9"), gbt.Rational("7/9")]
        )
        self.profile_rational[self.game.players[0].infosets[0]] = [
            gbt.Rational("4/13"), gbt.Rational("9/13")
        ]
        assert (
            self.profile_rational[self.game.players[0].infosets[0]] ==
            [gbt.Rational("4/13"), gbt.Rational("9/13")]
        )
        self.profile_rational[self.game.players[0].infosets[0]] = [
            gbt.Rational("1/98"), gbt.Rational("97/98")
        ]
        assert (
            self.profile_rational[self.game.players[0].infosets[0]] ==
            [gbt.Rational("1/98"), gbt.Rational("97/98")]
        )

    def test_set_probabilities_infoset_by_string(self):
        "Test to set probabilities to an infoset by string values"
        self.profile_double["Infoset 1:1"] = [0.72, 0.28]
        assert self.profile_double["Infoset 1:1"] == [0.72, 0.28]
        self.profile_double["Infoset 2:1"] = [0.42, 0.58]
        assert self.profile_double["Infoset 2:1"] == [0.42, 0.58]
        self.profile_double["Infoset 3:1"] = [0.02, 0.98]
        assert self.profile_double["Infoset 3:1"] == [0.02, 0.98]

        self.profile_rational["Infoset 1:1"] = [
            gbt.Rational("2/9"), gbt.Rational("7/9")
        ]
        assert (
            self.profile_rational["Infoset 1:1"] ==
            [gbt.Rational("2/9"), gbt.Rational("7/9")]
        )
        self.profile_rational["Infoset 2:1"] = [
            gbt.Rational("4/13"), gbt.Rational("9/13")
        ]
        assert (
            self.profile_rational["Infoset 2:1"] ==
            [gbt.Rational("4/13"), gbt.Rational("9/13")]
        )
        self.profile_rational["Infoset 3:1"] = [
            gbt.Rational("1/98"), gbt.Rational("97/98")
        ]
        assert (
            self.profile_rational["Infoset 3:1"] ==
            [gbt.Rational("1/98"), gbt.Rational("97/98")]
        )

    def test_set_probabilities_player(self):
        "Test to set probabilities to a player"
        self.profile_double[self.game.players[0]] = [[0.72, 0.28]]
        assert self.profile_double[self.game.players[0]] == [[0.72, 0.28]]
        self.profile_double[self.game.players[1]] = [[0.42, 0.58]]
        assert self.profile_double[self.game.players[1]] == [[0.42, 0.58]]
        self.profile_double[self.game.players[2]] = [[0.02, 0.98]]
        assert self.profile_double[self.game.players[2]] == [[0.02, 0.98]]

        self.profile_rational[self.game.players[0]] = [
            [gbt.Rational("2/9"), gbt.Rational("7/9")]
        ]
        assert (
            self.profile_rational[self.game.players[0]] ==
            [[gbt.Rational("2/9"), gbt.Rational("7/9")]]
        )
        self.profile_rational[self.game.players[1]] = [
            [gbt.Rational("4/13"), gbt.Rational("9/13")]
        ]
        assert (
            self.profile_rational[self.game.players[1]] ==
            [[gbt.Rational("4/13"), gbt.Rational("9/13")]]
        )
        self.profile_rational[self.game.players[2]] = [
            [gbt.Rational("1/98"), gbt.Rational("97/98")]
        ]
        assert (
            self.profile_rational[self.game.players[2]] ==
            [[gbt.Rational("1/98"), gbt.Rational("97/98")]]
        )

    def test_set_probabilities_player_by_string(self):
        "Test to set probabilities to a player by string values"
        self.profile_double["Player 1"] = [[0.72, 0.28]]
        assert self.profile_double["Player 1"] == [[0.72, 0.28]]
        self.profile_double["Player 2"] = [[0.42, 0.58]]
        assert self.profile_double["Player 2"] == [[0.42, 0.58]]
        self.profile_double["Player 3"] = [[0.02, 0.98]]
        assert self.profile_double["Player 3"] == [[0.02, 0.98]]

        self.profile_rational["Player 1"] = [
            [gbt.Rational("2/9"), gbt.Rational("7/9")]
        ]
        assert (
            self.profile_rational["Player 1"] ==
            [[gbt.Rational("2/9"), gbt.Rational("7/9")]]
        )
        self.profile_rational["Player 2"] = [
            [gbt.Rational("4/13"), gbt.Rational("9/13")]
        ]
        assert (
            self.profile_rational["Player 2"] ==
            [[gbt.Rational("4/13"), gbt.Rational("9/13")]]
        )
        self.profile_rational["Player 3"] = [
            [gbt.Rational("1/98"), gbt.Rational("97/98")]
        ]
        assert (
            self.profile_rational["Player 3"] ==
            [[gbt.Rational("1/98"), gbt.Rational("97/98")]]
        )

    def test_infoset_prob(self):
        """Test to retrieve the probability an information set is reached."""
        assert (self.profile_double.infoset_prob(self.game.players[0].infosets[0]) == 1.0)
        assert (self.profile_double.infoset_prob(self.game.players[1].infosets[0]) == 1.0)
        assert (self.profile_double.infoset_prob(self.game.players[2].infosets[0]) == 1.0)
        assert (self.profile_rational.infoset_prob(self.game.players[0].infosets[0]) ==
                gbt.Rational("1/1"))
        assert (self.profile_rational.infoset_prob(self.game.players[1].infosets[0]) ==
                gbt.Rational("1/1"))
        assert (self.profile_rational.infoset_prob(self.game.players[2].infosets[0]) ==
                gbt.Rational("1/1"))

    def test_infoset_prob_by_string(self):
        """Test to retrieve the probability an information set is reached
        using information set labels.
        """
        assert self.profile_double.infoset_prob("Infoset 1:1") == 1.0
        assert self.profile_double.infoset_prob("Infoset 2:1") == 1.0
        assert self.profile_double.infoset_prob("Infoset 3:1") == 1.0
        assert self.profile_rational.infoset_prob("Infoset 1:1") == gbt.Rational("1/1")
        assert self.profile_rational.infoset_prob("Infoset 2:1") == gbt.Rational("1/1")
        assert self.profile_rational.infoset_prob("Infoset 3:1") == gbt.Rational("1/1")

    def test_infoset_payoff(self):
        "Test to retrieve expected payoff associated to an infoset"
        assert self.profile_double.infoset_value(self.game.players[0].infosets[0]) == 3.0
        assert self.profile_double.infoset_value(self.game.players[1].infosets[0]) == 3.0
        assert self.profile_double.infoset_value(self.game.players[2].infosets[0]) == 3.25
        assert self.profile_rational.infoset_value(self.game.players[0].infosets[0]) == 3
        assert self.profile_rational.infoset_value(self.game.players[1].infosets[0]) == 3
        assert (
            self.profile_rational.infoset_value(self.game.players[2].infosets[0]) ==
            gbt.Rational("13/4")
        )

    def test_infoset_payoff_by_string(self):
        """Test to retrieve expected payoff associated to an infose
        by string values"""
        assert self.profile_double.infoset_value("Infoset 1:1") == 3.0
        assert self.profile_double.infoset_value("Infoset 2:1") == 3.0
        assert self.profile_double.infoset_value("Infoset 3:1") == 3.25
        assert self.profile_rational.infoset_value("Infoset 1:1") == 3
        assert self.profile_rational.infoset_value("Infoset 2:1") == 3
        assert self.profile_rational.infoset_value("Infoset 3:1") == gbt.Rational("13/4")

    def test_action_payoff(self):
        "Test to retrieve expected payoff associated to an action"
        assert (
            self.profile_double.action_value(self.game.players[0].infosets[0].actions[0]) == 3.0
        )
        assert (
            self.profile_double.action_value(self.game.players[0].infosets[0].actions[1]) == 3.0
        )
        assert (
            self.profile_double.action_value(self.game.players[1].infosets[0].actions[0]) == 3.0
        )
        assert (
            self.profile_double.action_value(self.game.players[1].infosets[0].actions[1]) == 3.0
        )
        assert (
            self.profile_double.action_value(self.game.players[2].infosets[0].actions[0]) == 3.5
        )
        assert (
            self.profile_double.action_value(self.game.players[2].infosets[0].actions[1]) == 3.0
        )

        assert (
            self.profile_rational.action_value(self.game.players[0].infosets[0].actions[0]) ==
            gbt.Rational("3/1")
        )
        assert (
            self.profile_rational.action_value(self.game.players[0].infosets[0].actions[1]) ==
            gbt.Rational("3/1")
        )
        assert (
            self.profile_rational.action_value(self.game.players[1].infosets[0].actions[0]) ==
            gbt.Rational("3/1")
        )
        assert (
            self.profile_rational.action_value(self.game.players[1].infosets[0].actions[1]) ==
            gbt.Rational("3/1")
        )
        assert (
            self.profile_rational.action_value(self.game.players[2].infosets[0].actions[0]) ==
            gbt.Rational("7/2")
        )
        assert (
            self.profile_rational.action_value(self.game.players[2].infosets[0].actions[1]) ==
            gbt.Rational("3/1")
        )

    def test_action_value_by_string(self):
        """Test to retrieve expected payoff associated to an action
        by string values
        """
        assert self.profile_double.action_value("U1") == 3.0
        assert self.profile_double.action_value("D1") == 3.0
        assert self.profile_double.action_value("U2") == 3.0
        assert self.profile_double.action_value("D2") == 3.0
        assert self.profile_double.action_value("U3") == 3.5
        assert self.profile_double.action_value("D3") == 3.0

        assert self.profile_rational.action_value("U1") == gbt.Rational("3/1")
        assert self.profile_rational.action_value("D1") == gbt.Rational("3/1")
        assert self.profile_rational.action_value("U2") == gbt.Rational("3/1")
        assert self.profile_rational.action_value("D2") == gbt.Rational("3/1")
        assert self.profile_rational.action_value("U3") == gbt.Rational("7/2")
        assert self.profile_rational.action_value("D3") == gbt.Rational("3/1")

    def test_regret(self):
        for profile in [self.profile_double, self.profile_rational]:
            for player in self.game.players:
                for infoset in player.infosets:
                    for action in infoset.actions:
                        assert (
                            profile.regret(action) ==
                            max(profile.action_value(a) for a in infoset.actions) -
                            profile.action_value(action)
                        )

    def test_node_value(self):
        # Another good node_value test (to be written!) is its martingale property: it should
        # be the expected value of its children's node_values, given the probability
        # distribution at the node.
        for profile in [self.profile_double, self.profile_rational]:
            for player in self.game.players:
                assert (
                    profile.node_value(player, self.game.root) == profile.payoff(player)
                )

    def test_liap_values(self):
        "Test to retrieve Lyapunov values"
        assert self.profile_double.liap_value() == 0.0625
        assert self.profile_rational.liap_value() == gbt.Rational("1/16")

    def test_as_strategy(self):
        """Test converting the behavior strategy profile to the equivalent
        mixed strategy profile.
        """
        mixed_double = self.profile_double.as_strategy()
        mixed_rational = self.profile_rational.as_strategy()
        assert (
            [mixed_double[strategy] for strategy in self.game.strategies] ==
            [self.profile_double[action] for action in self.game.actions]
        )
        assert (
            [mixed_rational[strategy] for strategy in self.game.strategies] ==
            [self.profile_rational[action] for action in self.game.actions]
        )

    def test_node_belief(self):
        "Test calculating belief probabilities on a node"
        self.profile_double[self.game.actions[0]] = 0.8
        self.profile_double[self.game.actions[1]] = 0.2
        self.profile_double[self.game.actions[2]] = 1.0
        self.profile_double[self.game.actions[3]] = 1.5
        self.profile_double[self.game.actions[4]] = 0.0
        self.profile_double[self.game.actions[5]] = 0.4
        assert self.profile_double.belief(self.game.root) == 1
        # Comparisons using 1e-13 as an arbitrary epsilon
        assert (
            abs(self.profile_double.belief(self.game.infosets[1].members[0]) -
                0.8) < 1e-13
        )
        assert (
            abs(self.profile_double.belief(self.game.infosets[1].members[1]) -
                0.2) < 1e-13
        )
        assert (
            abs(self.profile_double.belief(self.game.infosets[2].members[0]) -
                0.32) < 1e-13
        )
        assert (
            abs(self.profile_double.belief(self.game.infosets[2].members[1]) -
                0.48) < 1e-13
        )
        assert (
            abs(self.profile_double.belief(self.game.infosets[2].members[2]) -
                0.08) < 1e-13
        )
        assert (
            abs(self.profile_double.belief(self.game.infosets[2].members[3]) -
                0.12) < 1e-13
        )

        self.profile_rational[self.game.actions[0]] = gbt.Rational(4, 5)
        self.profile_rational[self.game.actions[1]] = gbt.Rational(1, 5)
        self.profile_rational[self.game.actions[2]] = gbt.Rational(1, 1)
        self.profile_rational[self.game.actions[3]] = gbt.Rational(3, 2)
        self.profile_rational[self.game.actions[4]] = gbt.Rational(0, 1)
        self.profile_rational[self.game.actions[5]] = gbt.Rational(2, 5)
        assert (
            self.profile_rational.belief(self.game.root) ==
            gbt.Rational(1, 1)
        )
        assert (
            self.profile_rational.belief(self.game.infosets[1].members[0]) ==
            gbt.Rational(4, 5)
        )
        assert (
            self.profile_rational.belief(self.game.infosets[1].members[1]) ==
            gbt.Rational(1, 5)
        )
        assert (
            self.profile_rational.belief(self.game.infosets[2].members[0]) ==
            gbt.Rational(8, 25)
        )
        assert (
            self.profile_rational.belief(self.game.infosets[2].members[1]) ==
            gbt.Rational(12, 25)
        )
        assert (
            self.profile_rational.belief(self.game.infosets[2].members[2]) ==
            gbt.Rational(2, 25)
        )
        assert (
            self.profile_rational.belief(self.game.infosets[2].members[3]) ==
            gbt.Rational(3, 25)
        )

    def test_infoset_belief(self):
        "Test calculating belief probabilities on an infoset"
        self.profile_double[self.game.actions[0]] = 0.8
        self.profile_double[self.game.actions[1]] = 0.2
        self.profile_double[self.game.actions[2]] = 0.4
        self.profile_double[self.game.actions[3]] = 0.6
        self.profile_double[self.game.actions[4]] = 0.0
        self.profile_double[self.game.actions[5]] = 1.0
        assert self.profile_double.belief(self.game.infosets[0].members[0]) == 1.0

        self.profile_rational[self.game.actions[0]] = gbt.Rational(4, 5)
        self.profile_rational[self.game.actions[1]] = gbt.Rational(1, 5)
        self.profile_rational[self.game.actions[2]] = gbt.Rational(2, 5)
        self.profile_rational[self.game.actions[3]] = gbt.Rational(3, 5)
        self.profile_rational[self.game.actions[4]] = gbt.Rational(0, 1)
        self.profile_rational[self.game.actions[5]] = gbt.Rational(1, 1)
        assert (
            self.profile_rational.belief(self.game.infosets[0].members[0]) ==
            gbt.Rational(1, 1)
        )

    def test_payoff_with_chance_player(self):
        """Test to ensure that payoff called with the chance player raises a ValueError"""
        chance_player = self.game_with_chance.players.chance
        self.assertRaises(ValueError, self.profile_double_with_chance.payoff, chance_player)
        self.assertRaises(ValueError, self.profile_rational_with_chance.payoff, chance_player)

    def test_infoset_value_with_chance_player_infoset(self):
        """Test to ensure that infoset_value called with an infoset of the chance player
        raises a ValueError
        """
        chance_infoset = self.game_with_chance.players.chance.infosets[0]
        self.assertRaises(ValueError, self.profile_double_with_chance.infoset_value,
                          chance_infoset)
        self.assertRaises(ValueError, self.profile_rational_with_chance.infoset_value,
                          chance_infoset)

    def test_action_value_with_chance_player_action(self):
        """Test to ensure that action_value called with an action of the chance player
        raises a ValueError
        """
        chance_action = self.game_with_chance.players.chance.infosets[0].actions[0]
        self.assertRaises(ValueError, self.profile_double_with_chance.action_value,
                          chance_action)
        self.assertRaises(ValueError, self.profile_rational_with_chance.action_value,
                          chance_action)
