import gambit
import fractions
import nose.tools
from nose.tools import assert_raises
from gambit.lib.error import UndefinedOperationError

class TestGambitStrategicRestriction(object):
    def setUp(self):
        self.game = gambit.read_game("test_games/mixed_strategy.nfg")
        
        self.profile_double = self.game.mixed_profile()
        self.profile_rational = self.game.mixed_profile(True)
        self.restriction = self.profile_double.restriction()

        self.tree_game = gambit.read_game("test_games/mixed_behavior_game.efg")

        self.tree_profile_double = self.tree_game.mixed_profile()
        self.tree_profile_rational = self.tree_game.mixed_profile(True)
        self.tree_restriction = self.tree_profile_double.restriction()

        
    def tearDown(self):
        del self.game
        del self.tree_game
        del self.profile_double
        del self.profile_rational
        del self.tree_profile_double
        del self.tree_profile_rational
        del self.restriction
        del self.tree_restriction
            

    def test_both_restrictions(self):
        "Test to ensure generated restrictions from double and rational-valued\
        profiles are equal"
        assert self.restriction == self.profile_rational.restriction()
        assert self.tree_restriction == self.tree_profile_rational.restriction()

    def test_unrestrict(self):
        "Test retrieving the game object from a restriction"
        assert self.restriction.unrestrict() == self.game
        assert self.tree_restriction.unrestrict() == self.tree_game

    def test_getting_strategies_directly(self):
        "Test retrieving strategies from a restriction"
        assert self.restriction.strategies[0] == self.game.players[0].strategies[0]
        assert self.restriction.strategies[1] == self.game.players[0].strategies[1]

    def test_getting_players(self):
        "Test retrieving players from a restriction"
        assert self.restriction.players[0] == self.game.players[0]
        assert self.restriction.players[1] == self.game.players[1]
        assert self.tree_restriction.players[0] == self.tree_game.players[0]
        assert self.tree_restriction.players[1] == self.tree_game.players[1]
        assert self.tree_restriction.players[2] == self.tree_game.players[2]

    def test_getting_outcomes(self):
        "Test retrieving outcomes from a restriction"
        for i in range(0, len(self.restriction.outcomes)-1):
            assert self.restriction.outcomes[i][0] == self.game.outcomes[i][0]
            assert self.restriction.outcomes[i][1] == self.game.outcomes[i][1]
            assert self.restriction.outcomes[i] == self.game.outcomes[i]

    def test_unrestricting_outcomes(self):
        "Test retrieving the original outcomes from a restriction"
        for i in range(0, len(self.restriction.outcomes)-1):
            assert self.restriction.outcomes[i].unrestrict() == self.game.outcomes[i]

    def test_unrestricting_strategies(self):
        "Test retrieving the original strategies from a restriction"
        for i in range(0, len(self.restriction.strategies)-1):
            assert self.restriction.strategies[i].unrestrict() == self.game.strategies[i]

    def test_unrestricting_players(self):
        "Test retrieving the original players from a restriction"
        for i in range(0, len(self.restriction.players)-1):
            assert self.restriction.players[i].unrestrict() == self.game.players[i]

    @nose.tools.raises(UndefinedOperationError)
    def test_restriction_add_outcome_error(self):
        "Test to ensure an error is raised when trying to add an outcome"\
        "to a restriction"
        self.restriction.outcomes.add("")

    @nose.tools.raises(UndefinedOperationError)
    def test_restriction_outcome_value_error(self):
        "Test to ensure an error is raised when trying to change the value"\
        "of an outcome in a restriction"
        self.restriction.outcomes[0][0] = 2

    @nose.tools.raises(UndefinedOperationError)
    def test_restriction_outcome_label_error(self):
        "Test to ensure an error is raised when trying to change the label"\
        "of an outcome in a restriction"
        self.restriction.outcomes[0].label = ""

    @nose.tools.raises(UndefinedOperationError)
    def test_restriction_outcome_delete_error(self):
        "Test to ensure an error is raised when trying to delete an outcome"\
        "in a restriction"
        self.restriction.outcomes[0].delete()

    @nose.tools.raises(UndefinedOperationError)
    def test_restriction_add_player_error(self):
        "Test to ensure an error is raised when trying to add a player"\
        "to a restriction"
        self.restriction.players.add("")
        
    @nose.tools.raises(UndefinedOperationError)
    def test_restriction_player_label_error(self):
        "Test to ensure an error is raised when trying to change the label"\
        "of a player in a restriction"
        self.restriction.players[0].label = ""

    @nose.tools.raises(UndefinedOperationError)
    def test_restriction_add_strategy_error(self):
        "Test to ensure an error is raised when trying to add a strategy"\
        "to a restriction"
        self.restriction.players[0].strategies.add("")

    @nose.tools.raises(UndefinedOperationError)
    def test_restriction_strategy_label_error(self):
        "Test to ensure an error is raised when trying to change the label"\
        "of a player in a restriction"
        self.restriction.strategies[0].label = ""

    @nose.tools.raises(UndefinedOperationError)
    def test_restriction_player_strategy_label_error(self):
        "Test to ensure an error is raised when trying to change the label"\
        "of a player's strategy in a restriction"
        self.restriction.players[0].strategies[0].label = ""
