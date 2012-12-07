import gambit
import decimal
import fractions
from nose.tools import assert_raises
import unittest
from gambit.lib.error import UndefinedOperationError
#from nose.tools import failUnlessRaises

class TestGambitActions(unittest.TestCase):
    def setUp(self):
        self.extensive_game = gambit.read_game("test_games/complicated_extensive_game.efg")
        
    def tearDown(self):
        del self.extensive_game
        
    def test_action_set_label(self):
        "Test to ensure action labels work"
        assert self.extensive_game.root.infoset.actions[0].label == "RED"
        self.extensive_game.root.infoset.actions[0].label = "action label"
        assert self.extensive_game.root.infoset.actions[0].label == "action label"

    def test_action_probability(self):
        "Test to ensure action probabilities work"
        assert self.extensive_game.root.infoset.actions[0].prob == decimal.Decimal('0.500000')
        
        self.extensive_game.root.infoset.actions[0].prob = decimal.Decimal('0.97300')
        assert self.extensive_game.root.infoset.actions[0].prob == decimal.Decimal('0.97300')
        
        self.extensive_game.root.infoset.actions[0].prob = fractions.Fraction('1/17')
        assert self.extensive_game.root.infoset.actions[0].prob == fractions.Fraction('1/17')
        
        self.extensive_game.root.infoset.actions[0].prob = 2
        assert self.extensive_game.root.infoset.actions[0].prob == 2

        self.failUnlessRaises(TypeError, setattr, self.extensive_game.root.infoset.actions[0], "prob", 2.0)
        self.failUnlessRaises(TypeError, setattr, self.extensive_game.root.infoset.actions[0], "prob", "test")
        self.failUnlessRaises(TypeError, setattr, self.extensive_game.root.infoset.actions[0], "prob", "1/7")
        self.failUnlessRaises(TypeError, setattr, self.extensive_game.root.infoset.actions[0], "prob", "2.7")

    def test_action_precedes(self):
        "Test to ensure precedes is working"
        assert not self.extensive_game.actions[0].precedes(self.extensive_game.root)
        assert self.extensive_game.actions[0].precedes(self.extensive_game.root.children[0].children[0])

    def test_action_precedes_error(self):
        "Test to ensure a TypeError is raised when precedes is called without a node"
        assert_raises(TypeError, self.extensive_game.actions[0].precedes, 0)

    def test_action_delete(self):
        "Test to ensure it is possible to delete an action"
        assert len(self.extensive_game.actions) == 6
        self.extensive_game.actions[0].delete()
        assert len(self.extensive_game.actions) == 5

    def test_action_delete_error(self):
        "Test to ensure deleting the last action of an infoset raises an error"
        assert len(self.extensive_game.infosets[0].actions) == 2
        self.extensive_game.actions[0].delete()
        assert_raises(UndefinedOperationError, self.extensive_game.actions[0].delete)
