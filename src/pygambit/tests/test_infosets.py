import unittest

import pygambit


class TestGambitInfosets(unittest.TestCase):
    def setUp(self):
        self.extensive_game = pygambit.Game.read_game(
            "test_games/basic_extensive_game.efg"
        )
        self.complicated_game = pygambit.Game.read_game(
            "test_games/complicated_extensive_game.efg"
        )

    def tearDown(self):
        del self.extensive_game
        del self.complicated_game

    def test_infoset_set_label(self):
        "Test to ensure infoset labels work"
        assert self.extensive_game.players[0].infosets[0].label == ""
        self.extensive_game.players[0].infosets[0].label = "infoset 1"
        assert self.extensive_game.players[0].infosets[0].label == "infoset 1"

    def test_infoset_player_retrieval(self):
        "Test to ensure infoset returns correct player"
        assert (
            self.extensive_game.players[0] ==
            self.extensive_game.players[0].infosets[0].player
        )

    def test_infoset_player_change(self):
        "Test to ensure infoset player transfer works"
        self.extensive_game.set_player(self.extensive_game.root.infoset,
                                       self.extensive_game.players[1])
        assert (
            self.extensive_game.root.infoset.player ==
            self.extensive_game.players[1]
        )

    def test_infoset_player_mismatch(self):
        """Test to ensure exception raised on setting player
        from different game.
        """
        def foo():
            g2 = pygambit.Game.new_tree()
            p = g2.add_player()
            self.extensive_game.set_player(self.extensive_game.root.infoset, p)
        self.assertRaises(pygambit.MismatchError, foo)

    def test_infoset_node_precedes(self):
        "Test to check if the infoset preceding check works"
        assert not self.extensive_game.players[0].infosets[0].precedes(
            self.extensive_game.root
        )
        assert (
            self.extensive_game.players[1].infosets[0].precedes(
                self.extensive_game.root.children[0]
            )
        )

    def test_infoset_add_action(self):
        assert len(self.extensive_game.infosets[0].actions) == 2
        self.extensive_game.add_action(self.extensive_game.infosets[0])
        assert len(self.extensive_game.infosets[0].actions) == 3
        self.extensive_game.add_action(
            self.extensive_game.infosets[0],
            self.extensive_game.actions[2]
        )
        assert len(self.extensive_game.infosets[0].actions) == 4

    def test_infoset_add_action_error(self):
        self.assertRaises(
            pygambit.MismatchError,
            self.extensive_game.add_action,
            self.extensive_game.infosets[0],
            self.extensive_game.actions[3]
        )
