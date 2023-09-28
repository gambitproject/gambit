import pygambit
import fractions
import unittest


class TestGambitActions(unittest.TestCase):
    def setUp(self):
        self.extensive_game = pygambit.Game.read_game(
            "test_games/complicated_extensive_game.efg"
        )

    def tearDown(self):
        del self.extensive_game

    def test_action_set_label(self):
        """Test to ensure action labels work"""
        assert self.extensive_game.root.infoset.actions[0].label == "RED"
        self.extensive_game.root.infoset.actions[0].label = "action label"
        assert (
            self.extensive_game.root.infoset.actions[0].label ==
            "action label"
        )

    def test_action_probability_chance(self):
        """Test setting action probabilities at chance information sets"""
        assert (self.extensive_game.root.infoset.actions[0].prob == 0.5)

        self.extensive_game.set_chance_probs(self.extensive_game.root.infoset, [0.75, 0.25])
        assert (self.extensive_game.root.infoset.actions[0].prob == 0.75)
        assert (self.extensive_game.root.infoset.actions[1].prob == 0.25)

        self.extensive_game.set_chance_probs(self.extensive_game.root.infoset, ["1/17", "16/17"])
        assert (self.extensive_game.root.infoset.actions[0].prob == fractions.Fraction("1/17"))
        assert (self.extensive_game.root.infoset.actions[1].prob == fractions.Fraction("16/17"))

        self.assertRaises(
            ValueError, self.extensive_game.set_chance_probs,
            self.extensive_game.root.infoset, [0.75, 0.10]
        )
        self.assertRaises(
            ValueError, self.extensive_game.set_chance_probs,
            self.extensive_game.root.infoset, [1.1, -0.1]
        )
        self.assertRaises(
            IndexError, self.extensive_game.set_chance_probs,
            self.extensive_game.root.infoset, [0.75, 0.10, 0.15]
        )
        self.assertRaises(
            ValueError, setattr, self.extensive_game.root.infoset.actions[0], "prob", "test"
        )

        self.extensive_game.set_chance_probs(self.extensive_game.root.infoset, [0.50, 0.50])

    def test_action_probability_nonchance(self):
        """Test attempts to set action probabilities at non-chance information sets."""
        self.assertRaises(
            pygambit.UndefinedOperationError, self.extensive_game.set_chance_probs,
            self.extensive_game.players[0].infosets[0], [0.75, 0.25]
        )

    def test_action_precedes(self):
        """Test to ensure precedes is working"""
        assert not self.extensive_game.actions[0].precedes(
            self.extensive_game.root
        )
        assert self.extensive_game.actions[0].precedes(
            self.extensive_game.root.children[0].children[0]
        )

    def test_action_precedes_error(self):
        """Test to ensure a TypeError is raised when precedes is called
        without a node
        """
        self.assertRaises(
            TypeError, self.extensive_game.actions[0].precedes, 0
        )

    def test_action_delete(self):
        """Test to ensure it is possible to delete an action"""
        assert len(self.extensive_game.actions) == 6
        self.extensive_game.delete_action(self.extensive_game.actions[0])
        assert len(self.extensive_game.actions) == 5

    def test_action_delete_error(self):
        """Test to ensure deleting the last action of an infoset
        raises an error
        """
        assert len(self.extensive_game.infosets[0].actions) == 2
        self.extensive_game.delete_action(self.extensive_game.actions[0])
        self.assertRaises(
            pygambit.UndefinedOperationError,
            self.extensive_game.delete_action,
            self.extensive_game.actions[0]
        )
