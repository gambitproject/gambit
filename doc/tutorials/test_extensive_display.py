"""
Test suite for extensive_display.py module.

This module contains unit tests for all functions in the extensive_display module
that handle PyGambit game tree visualization.
"""

import unittest
from unittest.mock import patch, MagicMock
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
import networkx as nx
import pygambit as gbt

# Import the module under test
from extensive_display import (
    create_player_color_map,
    build_game_graph,
    compute_graph_layout,
    create_node_labels,
    create_legend_elements,
    plot_gambit_tree
)


class TestExtensiveDisplay(unittest.TestCase):
    """Test class for extensive form game display functions."""
    
    def setUp(self):
        """Set up test fixtures before each test method."""
        # Load the trust game for testing
        self.trust_game = gbt.read_efg("games/trust_game.efg")
        
        # Create a simple test game programmatically
        self.simple_game = gbt.Game.new_tree(
            players=["Player1", "Player2"], 
            title="Test Game"
        )
        self.simple_game.append_move(
            self.simple_game.root,
            player="Player1",
            actions=["A", "B"]
        )
        # Add outcomes
        outcome1 = self.simple_game.add_outcome(payoffs=[1, 0], label="Outcome1")
        outcome2 = self.simple_game.add_outcome(payoffs=[0, 1], label="Outcome2")
        self.simple_game.set_outcome(self.simple_game.root.children[0], outcome1)
        self.simple_game.set_outcome(self.simple_game.root.children[1], outcome2)

    def test_create_player_color_map(self):
        """Test player color mapping function."""
        players = ["Player1", "Player2"]
        color_map = create_player_color_map(players)
        
        # Check that all players have colors
        self.assertIn("Player1", color_map)
        self.assertIn("Player2", color_map)
        self.assertIn("chance", color_map)
        
        # Check color format (RGBA tuples)
        for color in color_map.values():
            self.assertIsInstance(color, tuple)
            self.assertEqual(len(color), 4)  # RGBA
            for component in color:
                self.assertIsInstance(component, float)
                self.assertGreaterEqual(component, 0.0)
                self.assertLessEqual(component, 1.0)
        
        # Check that chance is gray
        self.assertEqual(color_map["chance"], (0.5, 0.5, 0.5, 1.0))

    def test_build_game_graph_simple(self):
        """Test graph building with simple game."""
        players = [p.label for p in self.simple_game.players]
        color_map = create_player_color_map(players)
        
        G, node_colors, node_mapping = build_game_graph(self.simple_game, color_map)
        
        # Check graph properties
        self.assertIsInstance(G, nx.DiGraph)
        self.assertEqual(len(G.nodes), 3)  # Root + 2 children
        self.assertEqual(len(G.edges), 2)  # 2 edges from root
        
        # Check node colors
        self.assertEqual(len(node_colors), 3)  # One color per node
        self.assertIsInstance(node_colors[0], tuple)
        
        # Check node mapping - use the first node ID from the mapping
        self.assertEqual(len(node_mapping), 3)
        # Check that the root node is mapped (should be counter 0)
        self.assertIn(0, node_mapping)
        self.assertEqual(node_mapping[0], self.simple_game.root)

    def test_build_game_graph_trust(self):
        """Test graph building with trust game."""
        players = [p.label for p in self.trust_game.players]
        color_map = create_player_color_map(players)
        
        G, node_colors, node_mapping = build_game_graph(self.trust_game, color_map)
        
        # Check graph properties for trust game structure
        self.assertIsInstance(G, nx.DiGraph)
        self.assertGreater(len(G.nodes), 3)  # Should have multiple nodes
        self.assertGreater(len(G.edges), 2)  # Should have multiple edges
        
        # Check that all nodes have colors and are mapped
        self.assertEqual(len(node_colors), len(G.nodes))
        for node_id in G.nodes:
            self.assertIn(node_id, node_mapping)

    @patch("extensive_display.nx.nx_agraph.graphviz_layout")
    def test_compute_graph_layout_graphviz_success(self, mock_graphviz):
        """Test graph layout computation with graphviz."""
        # Create test graph
        G = nx.DiGraph()
        G.add_edges_from([(1, 2), (2, 3)])
        
        # Mock successful graphviz layout
        mock_positions = {1: (0, 0), 2: (1, 1), 3: (2, 2)}
        mock_graphviz.return_value = mock_positions
        
        result = compute_graph_layout(G)
        
        self.assertEqual(result, mock_positions)
        mock_graphviz.assert_called_once_with(G, prog="dot")

    def test_create_node_labels_simple(self):
        """Test node label creation with simple game."""
        players = [p.label for p in self.simple_game.players]
        color_map = create_player_color_map(players)
        G, _, node_mapping = build_game_graph(self.simple_game, color_map)
        
        labels = create_node_labels(G, node_mapping, self.simple_game)
        
        # Check that all nodes have labels
        self.assertEqual(len(labels), len(G.nodes))
        
        # Check label content - root should be at counter 0
        self.assertEqual(labels[0], "Player1")
        
        # Check terminal node labels contain outcomes
        for node_id, label in labels.items():
            node = node_mapping[node_id]
            if node.outcome is not None:
                self.assertIn(node.outcome.label, label)
                self.assertIn("(", label)  # Should contain payoffs

    def test_create_legend_elements(self):
        """Test legend element creation."""
        players = ["Player1", "Player2"]
        color_map = create_player_color_map(players)
        
        legend_elements = create_legend_elements(players, color_map)
        
        # Check number of elements
        expected_count = len(players) + 1  # +1 for chance
        self.assertEqual(len(legend_elements), expected_count)
        
        # Check all elements are Line2D objects
        for element in legend_elements:
            self.assertIsInstance(element, Line2D)
        
        # Check labels
        labels = [element.get_label() for element in legend_elements]
        self.assertIn("Player1", labels)
        self.assertIn("Player2", labels)
        self.assertIn("Chance", labels)

    def test_create_legend_elements_no_chance(self):
        """Test legend element creation without chance player."""
        players = ["Player1", "Player2"]
        color_map = create_player_color_map(players)
        # Remove chance from color map
        del color_map["chance"]
        
        legend_elements = create_legend_elements(players, color_map)
        
        # Check number of elements (no chance)
        self.assertEqual(len(legend_elements), len(players))
        
        # Check labels don't include chance
        labels = [element.get_label() for element in legend_elements]
        self.assertNotIn("Chance", labels)

    @patch("extensive_display.plt.show")
    @patch("extensive_display.plt.figure")
    def test_plot_gambit_tree_integration(self, mock_figure, mock_show):
        """Test full plotting function integration."""
        # Test with simple game - disable edge labels to avoid drawing issues
        plot_gambit_tree(self.simple_game, figsize=(10, 6), show_edge_labels=False)
        
        # Just verify that the plot functions were called, not specific arguments
        mock_figure.assert_called()
        mock_show.assert_called_once()

    @patch("extensive_display.plt.show")
    @patch("extensive_display.plt.figure")
    def test_plot_gambit_tree_trust_game(self, mock_figure, mock_show):
        """Test plotting function with trust game."""
        plot_gambit_tree(
            self.trust_game, 
            figsize=(14, 10),
            node_size=600,
            font_size=10,
            edge_font_size=12,
            show_edge_labels=False  # Disable edge labels to avoid compatibility issues
        )
        
        # Just verify that the plot functions were called
        mock_figure.assert_called()
        mock_show.assert_called_once()

    def test_empty_game_handling(self):
        """Test handling of edge cases."""
        # Test with minimal game (just root)
        minimal_game = gbt.Game.new_tree(players=["P1"], title="Minimal")
        
        players = [p.label for p in minimal_game.players]
        color_map = create_player_color_map(players)
        G, node_colors, node_mapping = build_game_graph(minimal_game, color_map)
        
        # Should have one node, no edges
        self.assertEqual(len(G.nodes), 1)
        self.assertEqual(len(G.edges), 0)
        self.assertEqual(len(node_colors), 1)  # One node color for the root
        
        # Should still create labels
        labels = create_node_labels(G, node_mapping, minimal_game)
        self.assertEqual(len(labels), 1)

    def test_node_coloring_by_player(self):
        """Test that nodes are properly colored based on their controlling player."""
        # Create a more complex game to test node coloring
        game = gbt.Game.new_tree(players=["Alice", "Bob"], title="Node Coloring Test")
        
        # Add some moves to create a meaningful tree structure
        root = game.root
        game.append_move(root, player="Alice", actions=["A1", "A2"])
        alice_a1 = root.children[0]
        game.append_move(alice_a1, player="Bob", actions=["B1", "B2"])
        
        # Add outcomes to terminal nodes
        outcome1 = game.add_outcome(payoffs=[2, 1], label="Outcome1")
        outcome2 = game.add_outcome(payoffs=[1, 2], label="Outcome2")
        outcome3 = game.add_outcome(payoffs=[0, 0], label="Outcome3")
        game.set_outcome(alice_a1.children[0], outcome1)
        game.set_outcome(alice_a1.children[1], outcome2) 
        game.set_outcome(root.children[1], outcome3)
        
        # Create color map and build graph
        players = [p.label for p in game.players]
        color_map = create_player_color_map(players)
        G, node_colors, node_mapping = build_game_graph(game, color_map)
        
        # Verify we have colors for all nodes
        self.assertEqual(len(node_colors), len(G.nodes))
        
        # Check that nodes are colored based on their controlling player
        for node_id, color in enumerate(node_colors):
            if node_id in node_mapping:
                node = node_mapping[node_id]
                if node.infoset is not None:
                    # Decision node - should have player's color
                    player_label = node.infoset.player.label
                    expected_color = color_map[player_label]
                    self.assertEqual(color, expected_color, 
                                   f"Node {node_id} controlled by {player_label} should have "
                                   f"color {expected_color}")
                else:
                    # Terminal or root node - should have neutral light gray color
                    expected_color = (0.9, 0.9, 0.9, 1.0)
                    self.assertEqual(color, expected_color,
                                   f"Terminal/root node {node_id} should have neutral gray color")


if __name__ == '__main__':
    # Run the tests
    unittest.main()