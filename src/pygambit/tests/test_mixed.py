import fractions
import unittest

import pygambit


class TestGambitMixedStrategyGame(unittest.TestCase):
    def setUp(self):
        self.game = pygambit.Game.new_table([2, 2])
        self.game.players[0].label = "joe"
        self.game.players["joe"].strategies[0].label = "cooperate"
        self.game.players[1].label = "dan"
        self.game.players["dan"].strategies[1].label = "defect"

        self.profile_double = self.game.mixed_strategy_profile()
        self.profile_rational = self.game.mixed_strategy_profile(rational=True)

        self.tree_game = (
            pygambit.Game.read_game("test_games/mixed_behavior_game.efg")
        )

        self.tree_profile_double = self.tree_game.mixed_strategy_profile()
        self.tree_profile_rational = (
            self.tree_game.mixed_strategy_profile(rational=True)
        )

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
        assert (
            self.profile_double.strategy_value(
                self.game.players[0].strategies[1]
            ) == 0.0
        )
        assert (
            self.profile_rational.strategy_value(
                self.game.players[0].strategies[1]
            ) == 0.0
        )

    def test_strategy_value_by_string(self):
        "Test expected payoff based on given strategy"
        assert self.profile_double.strategy_value("defect") == 0.0
        assert self.profile_rational.strategy_value("defect") == 0.0

    def test_get_probabilities_strategy(self):
        """Test retrieving probabilities"""
        assert self.profile_double[self.game.strategies[0]] == 0.5
        assert self.profile_rational[self.game.strategies[0]] == fractions.Fraction("1/2")

    def test_get_probabilities_player(self):
        "Test retrieving probabilities"
        assert self.profile_double[self.game.players[0]] == [0.5, 0.5]
        assert self.profile_rational[self.game.players[0]] == (
            [fractions.Fraction("1/2"), fractions.Fraction("1/2")]
        )

        assert self.profile_double[self.game.players[0].strategies[0]] == 0.5
        assert self.profile_rational[self.game.players[0].strategies[0]] == (
            fractions.Fraction("1/2")
        )

        assert self.profile_double[self.game.players[0]]["cooperate"] == 0.5
        assert self.profile_rational[self.game.players[0]]["cooperate"] == (
            fractions.Fraction("1/2")
        )

    def test_set_probabilities(self):
        """Test setting probabilities"""
        self.profile_double[self.game.strategies[0]] = 0.72
        assert self.profile_double[self.game.strategies[0]] == 0.72
        self.profile_rational[self.game.strategies[0]] = fractions.Fraction("2/9")
        assert self.profile_rational[self.game.strategies[0]] == fractions.Fraction("2/9")

        self.profile_double[self.game.players[0].strategies[1]] = 0.72
        assert self.profile_double[self.game.players[0].strategies[1]] == 0.72

        self.profile_double[self.game.players[0]] = [0.72, 0.28]
        assert self.profile_double[self.game.players[0]] == [0.72, 0.28]

        self.profile_rational[self.game.players[0].strategies[1]] = fractions.Fraction("2/9")
        assert (
            self.profile_rational[self.game.players[0].strategies[1]] ==
            fractions.Fraction("2/9")
        )
        self.profile_rational[self.game.players[0]] = (
            [fractions.Fraction("2/9"), fractions.Fraction("7/9")]
        )
        assert (
            self.profile_rational[self.game.players[0]] ==
            [fractions.Fraction("2/9"), fractions.Fraction("7/9")]
        )

    def test_liap_values(self):
        "Test retrieving Lyapunov values"
        assert self.profile_double.liap_value() == 0.0
        assert self.profile_rational.liap_value() == fractions.Fraction("0")

    def test_as_behav_tree(self):
        "Test converting the profile to a behavior one"
        behav_double = self.tree_profile_double.as_behavior()
        behav_rational = self.tree_profile_rational.as_behavior()
        assert (
            [behav_double[action] for action in self.tree_game.actions] ==
            [self.tree_profile_double[strategy] for strategy in self.tree_game.strategies]
        )
        assert (
            [behav_rational[action] for action in self.tree_game.actions] ==
            [self.tree_profile_rational[strategy] for strategy in self.tree_game.strategies]
        )

    def test_as_behav_error(self):
        """Test raising an error when trying to convert a profile
        from a strategic game
        """
        self.assertRaises(
            pygambit.UndefinedOperationError, self.profile_double.as_behavior
        )
        self.assertRaises(
            pygambit.UndefinedOperationError, self.profile_rational.as_behavior
        )
