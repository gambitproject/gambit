import unittest

import pygambit

from . import games


class TestGambitNode(unittest.TestCase):
    def setUp(self):
        self.game = pygambit.Game.new_tree()
        self.extensive_game = (
            games.read_from_file("basic_extensive_game.efg")
        )

    def tearDown(self):
        del self.game
        del self.extensive_game

    def test_get_infoset(self):
        "Test to ensure that we can retrieve an infoset for a given node"
        assert self.extensive_game.root.infoset is not None
        assert self.extensive_game.root.children[0].infoset is not None
        assert (
            self.extensive_game.root.children[0].children[1].children[0]
            .infoset is None
        )

    def test_get_outcome(self):
        "Test to ensure that we can retrieve an outcome for a given node"
        assert (
            self.extensive_game.root.children[0].children[1].children[0]
            .outcome == self.extensive_game.outcomes[1]
        )
        assert self.extensive_game.root.outcome is None

    def test_get_player(self):
        "Test to ensure that we can retrieve a player for a given node"
        assert (
            self.extensive_game.root.player ==
            self.extensive_game.players[0]
        )
        assert (
            self.extensive_game.root.children[0].children[1].children[0]
            .player is None
        )

    def test_get_game(self):
        "Test to ensure that we can retrieve the game object from a given node"
        assert self.extensive_game == self.extensive_game.root.game

    def test_get_parent(self):
        "Test to ensure that we can retrieve a parent node for a given node"
        assert (
            self.extensive_game.root.children[0].parent ==
            self.extensive_game.root
        )
        assert self.extensive_game.root.parent is None

    def test_get_prior_action(self):
        "Test to ensure that we can retrieve the prior action for a given node"
        assert (
            self.extensive_game.root.children[0].prior_action ==
            self.extensive_game.root.infoset.actions[0]
        )
        assert self.extensive_game.root.prior_action is None

    def test_get_prior_sibling(self):
        "Test to ensure that we can retrieve a prior sibling of a given node"
        assert (
            self.extensive_game.root.children[1].prior_sibling ==
            self.extensive_game.root.children[0]
        )
        assert self.extensive_game.root.children[0].prior_sibling is None

    def test_get_next_sibling(self):
        "Test to ensure that we can retrieve a next sibling of a given node"
        assert (
            self.extensive_game.root.children[0].next_sibling ==
            self.extensive_game.root.children[1]
        )
        assert self.extensive_game.root.children[1].next_sibling is None

    def test_is_terminal(self):
        "Test to ensure that we can check if a given node is a terminal node"
        assert self.extensive_game.root.is_terminal is False
        assert (
            self.extensive_game.root.children[0].children[0].children[0]
            .is_terminal is True
        )

    def test_is_successor_of(self):
        """Test to ensure that we can check if a given node is a
        successor of a supplied node
        """
        assert (
            self.extensive_game.root.children[0].is_successor_of(
                self.extensive_game.root
            ) is True
        )
        assert (
            self.extensive_game.root.is_successor_of(
                self.extensive_game.root.children[0]
            ) is False
        )
        self.assertRaises(
            TypeError, self.extensive_game.root.is_successor_of, 9
        )
        self.assertRaises(
            TypeError, self.extensive_game.root.is_successor_of, "Test"
        )
        self.assertRaises(
            TypeError,
            self.extensive_game.root.is_successor_of,
            self.extensive_game.players[0]
        )

    def test_is_subgame_root(self):
        """Test whether nodes are correctly labeled as roots of proper subgames."""
        assert self.extensive_game.root.is_subgame_root is True
        assert self.extensive_game.root.children[0].is_subgame_root is False

    def test_append_move_error_player_actions(self):
        "Test to ensure there are actions when appending with a player"
        self.assertRaises(
            pygambit.UndefinedOperationError,
            self.extensive_game.append_move,
            self.extensive_game.root,
            self.extensive_game.players[0],
            []
        )

    def test_append_move_error_player_mismatch(self):
        "Test to ensure the node and the player are from the same game"
        self.assertRaises(
            pygambit.pygambit.MismatchError,
            self.game.append_move,
            self.game.root,
            self.extensive_game.players[0],
            ["a"]
        )

    def test_append_move_error_infoset_mismatch(self):
        "Test to ensure the node and the player are from the same game"
        self.assertRaises(pygambit.MismatchError,
                          self.game.append_infoset,
                          self.game.root,
                          self.extensive_game.players[0].infosets[0])

    def test_insert_move_error_player_actions(self):
        "Test to ensure there are actions when inserting with a player"
        self.assertRaises(
            pygambit.UndefinedOperationError,
            self.extensive_game.insert_move,
            self.extensive_game.root,
            self.extensive_game.players[0],
            0
        )

    def test_insert_move_error_player_mismatch(self):
        "Test to ensure the node and the player are from the same game"
        self.assertRaises(pygambit.MismatchError,
                          self.game.insert_move,
                          self.game.root,
                          self.extensive_game.players[0],
                          1)

    def test_node_leave_infoset(self):
        "Test to ensure it's possible to remove a node from an infoset"
        assert len(self.extensive_game.infosets[1].members) == 2
        self.extensive_game.leave_infoset(self.extensive_game.root.children[0])
        assert len(self.extensive_game.infosets[1].members) == 1

    def test_node_delete_parent(self):
        "Test to ensure deleting a parent node works"
        node = self.extensive_game.root.children[0]
        node.game.delete_parent(node)
        assert self.extensive_game.root == node

    def test_node_delete_tree(self):
        "Test to ensure deleting every children of a node works"
        node = self.extensive_game.root.children[0]
        node.game.delete_tree(node)
        assert len(node.children) == 0

    def test_node_copy_nonterminal(self):
        """Test on copying to a nonterminal node."""
        self.assertRaises(pygambit.UndefinedOperationError, self.extensive_game.copy_tree,
                          self.extensive_game.root, self.extensive_game.root)

    def test_node_copy_across_games(self):
        """Test to ensure a pygambit.MismatchError is raised when trying to copy a tree
        from a different game.
        """
        self.assertRaises(pygambit.MismatchError, self.game.copy_tree,
                          self.game.root, self.extensive_game.root)
        self.assertRaises(pygambit.MismatchError, self.game.copy_tree,
                          self.extensive_game.root, self.game.root)

    def test_node_move_nonterminal(self):
        """Test on moving to a nonterminal node."""
        self.assertRaises(pygambit.UndefinedOperationError, self.extensive_game.move_tree,
                          self.extensive_game.root, self.extensive_game.root)

    def test_node_move_successor(self):
        """Test on moving a node to one of its successors."""
        self.assertRaises(pygambit.UndefinedOperationError, self.extensive_game.move_tree,
                          self.extensive_game.root,
                          self.extensive_game.root.children[0].children[0].children[0])

    def test_node_move_across_games(self):
        """Test to ensure a pygambit.MismatchError is raised when trying to move a tree
        between different games.
        """
        self.assertRaises(pygambit.MismatchError, self.game.move_tree,
                          self.game.root, self.extensive_game.root)
        self.assertRaises(pygambit.MismatchError, self.game.move_tree,
                          self.extensive_game.root, self.game.root)
