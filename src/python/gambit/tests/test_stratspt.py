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
        self.support = self.profile_double.support()

        self.tree_game = gambit.read_game("test_games/mixed_behavior_game.efg")

        self.tree_profile_double = self.tree_game.mixed_profile()
        self.tree_profile_rational = self.tree_game.mixed_profile(True)
        self.tree_support = self.tree_profile_double.support()

        
    def tearDown(self):
        del self.game
        del self.tree_game
        del self.profile_double
        del self.profile_rational
        del self.tree_profile_double
        del self.tree_profile_rational
        del self.support
        del self.tree_support
            

    def test_both_supports(self):
        "Test to ensure generated supports from double and rational-valued\
        profiles are equal"
        assert self.support == self.profile_rational.support()
        assert self.tree_support == self.tree_profile_rational.support()

    def test_unrestrict(self):
        "Test retrieving the game object from a support"
        assert self.support.unrestrict() == self.game
        assert self.tree_support.unrestrict() == self.tree_game

    def test_getting_strategies_directly(self):
        "Test retrieving strategies from a support"
        assert self.support.strategies[0] == self.game.players[0].strategies[0]
        assert self.support.strategies[1] == self.game.players[0].strategies[1]

    def test_getting_players(self):
        "Test retrieving players from a support"
        assert self.support.players[0] == self.game.players[0]
        assert self.support.players[1] == self.game.players[1]
        assert self.tree_support.players[0] == self.tree_game.players[0]
        assert self.tree_support.players[1] == self.tree_game.players[1]
        assert self.tree_support.players[2] == self.tree_game.players[2]

    def test_getting_outcomes(self):
        "Test retrieving outcomes from a support"
        for i in range(0, len(self.support.outcomes)-1):
            assert self.support.outcomes[i][0] == self.game.outcomes[i][0]
            assert self.support.outcomes[i][1] == self.game.outcomes[i][1]
            assert self.support.outcomes[i] == self.game.outcomes[i]

    def test_unrestricting_outcomes(self):
        "Test retrieving the original outcomes from a support"
        for i in range(0, len(self.support.outcomes)-1):
            assert self.support.outcomes[i].unrestrict() == self.game.outcomes[i]

    def test_unrestricting_strategies(self):
        "Test retrieving the original strategies from a support"
        for i in range(0, len(self.support.strategies)-1):
            assert self.support.strategies[i].unrestrict() == self.game.strategies[i]

    def test_unrestricting_players(self):
        "Test retrieving the original players from a support"
        for i in range(0, len(self.support.players)-1):
            assert self.support.players[i].unrestrict() == self.game.players[i]

    @nose.tools.raises(UndefinedOperationError)
    def test_support_add_outcome_error(self):
        "Test to ensure an error is raised when trying to add an outcome"\
        "to a support"
        self.support.outcomes.add("")

    @nose.tools.raises(UndefinedOperationError)
    def test_support_outcome_value_error(self):
        "Test to ensure an error is raised when trying to change the value"\
        "of an outcome in a support"
        self.support.outcomes[0][0] = 2

    @nose.tools.raises(UndefinedOperationError)
    def test_support_outcome_label_error(self):
        "Test to ensure an error is raised when trying to change the label"\
        "of an outcome in a support"
        self.support.outcomes[0].label = ""

    @nose.tools.raises(UndefinedOperationError)
    def test_support_outcome_delete_error(self):
        "Test to ensure an error is raised when trying to delete an outcome"\
        "in a support"
        self.support.outcomes[0].delete()

    @nose.tools.raises(UndefinedOperationError)
    def test_support_add_player_error(self):
        "Test to ensure an error is raised when trying to add a player"\
        "to a support"
        self.support.players.add("")
        
    @nose.tools.raises(UndefinedOperationError)
    def test_support_player_label_error(self):
        "Test to ensure an error is raised when trying to change the label"\
        "of a player in a support"
        self.support.players[0].label = ""

    @nose.tools.raises(UndefinedOperationError)
    def test_support_add_strategy_error(self):
        "Test to ensure an error is raised when trying to add a strategy"\
        "to a support"
        self.support.players[0].strategies.add("")

    @nose.tools.raises(UndefinedOperationError)
    def test_support_strategy_label_error(self):
        "Test to ensure an error is raised when trying to change the label"\
        "of a player in a support"
        self.support.strategies[0].label = ""

    @nose.tools.raises(UndefinedOperationError)
    def test_support_player_strategy_label_error(self):
        "Test to ensure an error is raised when trying to change the label"\
        "of a player's strategy in a support"
        self.support.players[0].strategies[0].label = ""
