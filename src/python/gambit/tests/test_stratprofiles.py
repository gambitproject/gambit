import gambit
import fractions
import nose.tools
from nose.tools import assert_raises
from gambit.lib.error import UndefinedOperationError

class TestGambitStrategySupportProfile(object):
    def setUp(self):
        self.game = gambit.read_game("test_games/mixed_strategy.nfg")
        self.support_profile = self.game.support_profile()
        self.restriction = self.support_profile.restrict()
        
    def tearDown(self):
        del self.game
        del self.support_profile
        del self.restriction
            

    def test_num_strategies(self):
        "Ensure the support profile of the full game still has all strategies"
        assert len(self.support_profile) == len(self.game.strategies)
        assert len(self.support_profile) == len(self.restriction.strategies)

    def test_getitem(self):
        "Test retrieving strategies from a support profile"
        for strategy in self.support_profile:
            assert strategy in self.game.strategies
            assert strategy in self.restriction.strategies

    def test_remove(self):
        "Test removing strategies from a support profile"
        strategy = self.support_profile[0]
        new_profile = self.support_profile.remove(strategy)
        assert len(self.support_profile) == new_profile + 1
        assert strategy not in new_profile

    def test_remove_restriction(self):
        "Test generating an incomplete strategic restriction from a support profile"
        strategy = self.support_profile[0]
        new_profile = self.support_profile.remove(strategy)
        new_restriction = new_profile.restrict()
        assert len(new_restriction.strategies) == len(self.restriction) + 1
        assert strategy not in new_restriction.strategies
