"""
Visualization functions for PyGambit extensive form games.

This module provides functions to create and display game tree visualizations
using NetworkX and Matplotlib.
"""

from typing import Any, Optional
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
from matplotlib.lines import Line2D
import networkx as nx
import pygambit as gbt


def create_player_color_map(players: list[str]) -> dict[str, tuple[float, float, float, float]]:
    """
    Create a color mapping for players in the game.
    
    Args:
        players: List of player label strings
        
    Returns:
        Dictionary mapping player labels to RGBA color tuples
    """
    color_map = plt.colormaps.get_cmap("tab10")
    player_colors = {p: color_map(i) for i, p in enumerate(players)}
    player_colors["chance"] = (1.0, 0.0, 0.0, 1.0)  # red for chance
    return player_colors


def build_game_graph(game: Any, player_colors: dict[str, tuple[float, float, float, float]]) -> tuple[nx.DiGraph, list[tuple[float, float, float, float]], dict[int, Any]]:
    """
    Build a NetworkX directed graph from a PyGambit game tree.
    
    Args:
        game: PyGambit Game object
        player_colors: Dictionary mapping player labels to RGBA color tuples
        
    Returns:
        Tuple containing:
        - NetworkX DiGraph representing the game tree
        - List of node colors corresponding to players who control each node
        - Dictionary mapping node IDs to PyGambit node objects
    """
    G = nx.DiGraph()
    node_colors = []
    node_mapping = {}  # Map node_id to actual node object
    node_counter = 0  # Use counter instead of object ID for stability

    def add_edges(node: Any, parent: Optional[Any] = None, action_label: Optional[str] = None, parent_counter: Optional[int] = None) -> int:
        """Recursively add edges to the graph."""
        nonlocal node_counter
        current_counter = node_counter
        node_counter += 1
        
        # Always add the node to the graph
        G.add_node(current_counter)
        node_mapping[current_counter] = node
        
        # Assign node color based on the player who controls this node
        if node.player:
            player_label = node.player.label
            # Special handling for chance player
            if hasattr(node.player, "is_chance") and node.player.is_chance:
                node_colors.append(player_colors.get("chance", (1.0, 0.0, 0.0, 1.0)))
            else:
                node_colors.append(player_colors.get(player_label, (0.7, 0.7, 0.7, 1.0)))
        elif node.outcome is not None:
            # Terminal nodes should be invisible (transparent)
            node_colors.append((0.0, 0.0, 0.0, 0.0))  # Fully transparent
        else:
            # Root or other nodes get a neutral color
            node_colors.append((0.9, 0.9, 0.9, 1.0))  # Light gray

        if parent is not None and parent_counter is not None:
            # Add edge without color information (edges will be black)
            G.add_edge(parent_counter, current_counter, label=action_label, player=parent.player.label if parent.player else "chance")

        for child in node.children:
            add_edges(child, node, child.label, current_counter)
            
        return current_counter

    # Always add the root node, even if it has no children
    add_edges(game.root)
    
    return G, node_colors, node_mapping


def compute_graph_layout(G: nx.DiGraph) -> dict[int, tuple[float, float]]:
    """
    Compute node positions for the graph layout.
    
    Args:
        G: NetworkX DiGraph
        
    Returns:
        Dictionary mapping node IDs to (x, y) position tuples
    """
    pos = nx.nx_agraph.graphviz_layout(G, prog="dot")
    return pos


def create_node_labels(G: nx.DiGraph, node_mapping: dict[int, Any], game: Any) -> dict[int, str]:
    """
    Create labels for nodes in the graph.
    
    Args:
        G: NetworkX DiGraph
        node_mapping: Dictionary mapping node IDs to PyGambit node objects
        game: PyGambit Game object
        
    Returns:
        Dictionary mapping node IDs to label strings
    """
    node_labels = {}
    for node_id in G.nodes:
        node = node_mapping[node_id]
        if node.outcome is not None:
            # Terminal node - show outcome and payoffs
            payoffs = [str(node.outcome[player]) for player in game.players]
            node_labels[node_id] = f"{node.outcome.label}\n({', '.join(payoffs)})"
        elif node.player:
            # Decision node - show player name
            node_labels[node_id] = node.player.label
        else:
            # Root or other node
            node_labels[node_id] = "Root"
    return node_labels


def create_legend_elements(players: list[str], player_colors: dict[str, tuple[float, float, float, float]]) -> list[Line2D]:
    """
    Create legend elements for the plot.
    
    Args:
        players: List of player label strings
        player_colors: Dictionary mapping player labels to RGBA color tuples
        
    Returns:
        List of matplotlib Line2D objects for the legend
    """
    legend_elements = []
    for player in players:
        legend_elements.append(
            Line2D([0], [0], color=player_colors[player], lw=3, label=player)
        )
    if "chance" in player_colors:
        legend_elements.append(
            Line2D([0], [0], color=player_colors["chance"], lw=3, label="Chance")
        )
    return legend_elements


def plot_gambit_tree(game: Any, 
                    figsize: tuple[int, int] = (6, 6),
                    node_size: int = 500,
                    font_size: int = 8,
                    edge_font_size: int = 10,
                    show_edge_labels: bool = True) -> None:
    """
    Plot a PyGambit extensive form game as a tree visualization.
    
    Args:
        game: PyGambit Game object to visualize
        figsize: Figure size as (width, height) tuple
        node_size: Size of nodes in the graph
        font_size: Font size for node labels
        edge_font_size: Font size for edge labels
        show_edge_labels: Whether to show edge labels (action names)
    """
    # Get player list and create color mapping
    players = [p.label for p in game.players]
    player_colors = create_player_color_map(players)
    
    # Build the graph
    G, node_colors, node_mapping = build_game_graph(game, player_colors)
    
    # If graph is empty (no nodes), nothing to plot
    if len(G.nodes) == 0:
        print("Warning: No nodes to plot in game tree")
        return
    
    # Compute layout
    pos = compute_graph_layout(G)

    # Create the plot
    plt.figure(figsize=figsize)
    nx.draw(
        G,
        pos,
        with_labels=False,
        arrows=True,
        node_size=node_size,
        edge_color="black",
        node_color=node_colors,
        arrowsize=20,
        width=2,
    )

    # Draw edge labels (action labels) only if there are edges and labels are requested
    if show_edge_labels and len(G.edges) > 0:
        try:
            edge_labels = nx.get_edge_attributes(G, "label")
            # Only try to draw labels if they exist
            if edge_labels:
                nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, 
                                            font_size=edge_font_size, font_color="black")
        except Exception as e:
            print(f"Warning: Could not draw edge labels: {e}")

    # Draw node labels
    node_labels = create_node_labels(G, node_mapping, game)
    nx.draw_networkx_labels(G, pos, labels=node_labels, font_size=font_size)

    # Create and display legend
    legend_elements = create_legend_elements(players, player_colors)
    plt.legend(handles=legend_elements, loc="upper right")
    
    plt.title(f"Game Tree: {game.title}")
    plt.axis("off")
    plt.tight_layout()
    plt.show()