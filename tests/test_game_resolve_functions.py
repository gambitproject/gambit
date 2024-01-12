import unittest

from . import games


class TestGambitResolveFunctions(unittest.TestCase):
    def setUp(self):
        self.game1 = games.read_from_file("sample_extensive_game.efg")

        # has named outcomes
        self.game2 = games.read_from_file("basic_extensive_game.efg")

        # has named infosets
        self.game3 = games.read_from_file("mixed_behavior_game.efg")

    def tearDown(self):
        del self.game1
        del self.game2
        del self.game3

    def test_resolve_player_empty_strings(self):
        """Test _resolve_player with the empty string or strings of all spaces"""
        self.assertRaises(ValueError, self.game1._resolve_player, player="", funcname="test")
        self.assertRaises(ValueError, self.game1._resolve_player, player=" ", funcname="test")
        self.assertRaises(ValueError, self.game2._resolve_player, player="", funcname="test")
        self.assertRaises(ValueError, self.game2._resolve_player, player=" ", funcname="test")

    def test_resolve_player_nonempty_strings(self):
        """Test _resolve_player with non-empty strings, some that resolve some that don't"""
        assert self.game1._resolve_player(player="Player 1", funcname="test")
        assert self.game1._resolve_player(player="Player 2", funcname="test")
        self.assertRaises(KeyError, self.game1._resolve_player, player="Player 3",
                          funcname="test")

    def test_resolve_outcome_empty_strings(self):
        """Test _resolve_outcome with empty string or strings of all spaces"""
        self.assertRaises(ValueError, self.game1._resolve_outcome, outcome="", funcname="test")
        self.assertRaises(ValueError, self.game1._resolve_outcome, outcome="  ", funcname="test")
        self.assertRaises(ValueError, self.game2._resolve_outcome, outcome="", funcname="test")
        self.assertRaises(ValueError, self.game2._resolve_outcome, outcome="  ", funcname="test")

    def test_resolve_outcome_nonempty_strings(self):
        """Test _resolve_outcome with non-empty strings, some that resolve some that don't"""
        assert self.game2._resolve_outcome(outcome="Outcome 1", funcname="test")
        assert self.game2._resolve_outcome(outcome="Outcome 2", funcname="test")
        self.assertRaises(KeyError, self.game2._resolve_outcome, outcome="Outcome 5",
                          funcname="test")

    def test_resolve_strategy_empty_strings(self):
        """Test _resolve_strategy with empty string or strings of all spaces"""
        self.assertRaises(ValueError, self.game1._resolve_strategy, strategy="", funcname="test")
        self.assertRaises(ValueError, self.game1._resolve_strategy, strategy="  ",
                          funcname="test")
        self.assertRaises(ValueError, self.game2._resolve_strategy, strategy="", funcname="test")
        self.assertRaises(ValueError, self.game2._resolve_strategy, strategy="  ",
                          funcname="test")

    def test_resolve_strategy_nonempty_strings(self):
        """Test _resolve_strategy with non-empty strings, some that resolve some that don't"""
        assert self.game1._resolve_strategy(strategy="11", funcname="test")
        assert self.game1._resolve_strategy(strategy="12", funcname="test")
        self.assertRaises(KeyError, self.game1._resolve_strategy, strategy="13", funcname="test")
        self.assertRaises(ValueError, self.game2._resolve_strategy, strategy="1", funcname="test")
        self.assertRaises(ValueError, self.game2._resolve_strategy, strategy="2", funcname="test")
        self.assertRaises(KeyError, self.game2._resolve_strategy, strategy="3", funcname="test")

    def test_resolve_node_empty_strings(self):
        """Test _resolve_node with empty string or strings of all spaces"""
        self.assertRaises(ValueError, self.game1._resolve_node, node="", funcname="test")
        self.assertRaises(ValueError, self.game1._resolve_node, node="  ", funcname="test")
        self.assertRaises(ValueError, self.game2._resolve_node, node="", funcname="test")
        self.assertRaises(ValueError, self.game2._resolve_node, node="  ", funcname="test")

    def test_resolve_node_nonempty_strings(self):
        """Test _resolve_node with non-empty strings, some that resolve some that don't"""
        assert self.game1._resolve_node(node="1", funcname="test")
        assert self.game1._resolve_node(node="2", funcname="test")
        self.assertRaises(KeyError, self.game1._resolve_node, node="4", funcname="test")
        self.assertRaises(KeyError, self.game2._resolve_node, node="1", funcname="test")

    def test_resolve_infoset_empty_strings(self):
        """Test _resolve_infoset with empty string or strings of all spaces"""
        self.assertRaises(ValueError, self.game1._resolve_infoset, infoset="", funcname="test")
        self.assertRaises(ValueError, self.game1._resolve_infoset, infoset="  ", funcname="test")
        self.assertRaises(ValueError, self.game2._resolve_infoset, infoset="", funcname="test")
        self.assertRaises(ValueError, self.game2._resolve_infoset, infoset="  ", funcname="test")

    def test_resolve_infoset_nonempty_strings(self):
        """Test _resolve_node with non-empty strings, some that resolve some that don't"""
        assert self.game3._resolve_infoset(infoset="Infoset 1:1", funcname="test")
        assert self.game3._resolve_infoset(infoset="Infoset 2:1", funcname="test")
        self.assertRaises(KeyError, self.game3._resolve_infoset, infoset="Infoset 4:1",
                          funcname="test")

    def test_resolve_action_empty_strings(self):
        """Test _resolve_action with empty string or strings of all spaces"""
        self.assertRaises(ValueError, self.game1._resolve_action, action="", funcname="test")
        self.assertRaises(ValueError, self.game1._resolve_action, action="  ", funcname="test")
        self.assertRaises(ValueError, self.game2._resolve_action, action="", funcname="test")
        self.assertRaises(ValueError, self.game2._resolve_action, action="  ", funcname="test")

    def test_resolve_action_nonempty_strings(self):
        """Test _resolve_action with non-empty strings, some that resolve some that don't"""
        self.assertRaises(ValueError, self.game1._resolve_action, action="1", funcname="test")
        self.assertRaises(ValueError, self.game1._resolve_action, action="2", funcname="test")
        self.assertRaises(KeyError, self.game1._resolve_action, action="3", funcname="test")
        assert self.game2._resolve_action(action="U1", funcname="test")
        assert self.game2._resolve_action(action="D1", funcname="test")
        self.assertRaises(KeyError, self.game2._resolve_action, action="D4", funcname="test")
