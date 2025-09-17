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


def build_game_graph(game: Any, player_colors: dict[str, tuple[float, float, float, float]]) -> tuple[nx.DiGraph, list[tuple[float, float, float, float]], dict[int, Any], dict[Any, list[int]]]:
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
        - Dictionary mapping infosets to lists of node IDs
    """
    G = nx.DiGraph()
    node_colors = []
    node_mapping = {}  # Map node_id to actual node object
    infoset_mapping = {}  # Map infoset to list of node_ids
    node_counter = 0  # Use counter instead of object ID for stability

    def add_edges(node: Any, parent: Optional[Any] = None, action_label: Optional[str] = None, parent_counter: Optional[int] = None) -> int:
        """Recursively add edges to the graph."""
        nonlocal node_counter
        current_counter = node_counter
        node_counter += 1
        
        # Always add the node to the graph
        G.add_node(current_counter)
        node_mapping[current_counter] = node
        
        # Track information sets
        if node.infoset is not None:
            infoset = node.infoset
            if infoset not in infoset_mapping:
                infoset_mapping[infoset] = []
            infoset_mapping[infoset].append(current_counter)
        
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
            add_edges(child, node, child.prior_action.label, current_counter)
            
        return current_counter

    # Always add the root node, even if it has no children
    add_edges(game.root)
    
    return G, node_colors, node_mapping, infoset_mapping


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


def adjust_layout_for_infosets(pos: dict[int, tuple[float, float]], 
                              infoset_mapping: dict[Any, list[int]], 
                              node_mapping: dict[int, Any]) -> dict[int, tuple[float, float]]:
    """
    Adjust node positions to group information set nodes spatially close together
    and prevent other nodes from falling inside information set ovals.
    
    Args:
        pos: Original positions from layout algorithm
        infoset_mapping: Dictionary mapping infosets to lists of node IDs
        node_mapping: Dictionary mapping node IDs to PyGambit node objects
        
    Returns:
        Adjusted positions dictionary
    """
    import math
    adjusted_pos = pos.copy()
    
    # Step 1: Group information set nodes
    infoset_ovals = {}  # Store oval parameters for collision detection
    
    for _, node_ids in infoset_mapping.items():
        if len(node_ids) > 1:  # Only adjust if there are multiple nodes in the infoset
            # Calculate the centroid of the original positions
            centroid_x = sum(pos[node_id][0] for node_id in node_ids) / len(node_ids)
            centroid_y = sum(pos[node_id][1] for node_id in node_ids) / len(node_ids)
            
            # Determine layout strategy based on number of nodes
            if len(node_ids) == 2:
                # For 2 nodes, place them horizontally apart to avoid edge overlap
                spacing = 60  # Horizontal spacing
                adjusted_pos[node_ids[0]] = (centroid_x - spacing/2, centroid_y)
                adjusted_pos[node_ids[1]] = (centroid_x + spacing/2, centroid_y)
                
                # Store oval parameters for collision detection
                base_padding = 40
                width = spacing + base_padding
                height = max(base_padding, 60)
                infoset_ovals[tuple(node_ids)] = {
                    "center": (centroid_x, centroid_y),
                    "width": width,
                    "height": height,
                    "nodes": set(node_ids)
                }
            else:
                # For more than 2 nodes, use a larger radius circle
                radius = 40 + (len(node_ids) - 2) * 10  # Adaptive radius based on node count
                for i, node_id in enumerate(node_ids):
                    angle = 2 * math.pi * i / len(node_ids)
                    new_x = centroid_x + radius * math.cos(angle)
                    new_y = centroid_y + radius * math.sin(angle)
                    adjusted_pos[node_id] = (new_x, new_y)
                
                # Store oval parameters for collision detection
                base_padding = 40
                diameter = radius * 2 + base_padding
                infoset_ovals[tuple(node_ids)] = {
                    "center": (centroid_x, centroid_y),
                    "width": diameter,
                    "height": diameter,
                    "nodes": set(node_ids)
                }
    
    # Step 2: Move nodes that fall inside information set ovals they don't belong to
    all_infoset_nodes = set()
    for node_ids in infoset_mapping.values():
        if len(node_ids) > 1:
            all_infoset_nodes.update(node_ids)
    
    def point_in_ellipse(px, py, cx, cy, width, height):
        """Check if point (px, py) is inside ellipse centered at (cx, cy)"""
        return ((px - cx) / (width/2))**2 + ((py - cy) / (height/2))**2 <= 1
    
    def find_safe_position(original_x, original_y, ovals):
        """Find a position that doesn't intersect with any oval"""
        # Try positions around the original point
        safe_distance = 80  # Minimum distance from oval edge
        angles = [0, math.pi/2, math.pi, 3*math.pi/2, math.pi/4, 
                 3*math.pi/4, 5*math.pi/4, 7*math.pi/4]
        for angle in angles:
            test_x = original_x + safe_distance * math.cos(angle)
            test_y = original_y + safe_distance * math.sin(angle)
            
            # Check if this position is safe from all ovals
            is_safe = True
            for oval in ovals.values():
                if point_in_ellipse(test_x, test_y, oval["center"][0], oval["center"][1], 
                                  oval["width"], oval["height"]):
                    is_safe = False
                    break
            
            if is_safe:
                return test_x, test_y
        
        # If no safe position found nearby, move further out
        return original_x + safe_distance * 1.5, original_y
    
    # Check each node that's not part of any multi-node information set
    for node_id, (x, y) in adjusted_pos.items():
        if node_id not in all_infoset_nodes:
            # Check if this node falls inside any information set oval
            for oval in infoset_ovals.values():
                if point_in_ellipse(x, y, oval["center"][0], oval["center"][1], 
                                  oval["width"], oval["height"]):
                    # Node is inside an oval it doesn't belong to - move it
                    new_x, new_y = find_safe_position(x, y, infoset_ovals)
                    adjusted_pos[node_id] = (new_x, new_y)
                    break
    
    return adjusted_pos


def draw_infoset_ovals(ax, pos: dict[int, tuple[float, float]], 
                      infoset_mapping: dict[Any, list[int]], 
                      node_mapping: dict[int, Any], 
                      player_colors: dict[str, tuple[float, float, float, float]]) -> None:
    """
    Draw ovals around nodes that belong to the same information set.
    
    Args:
        ax: Matplotlib axes object
        pos: Node positions
        infoset_mapping: Dictionary mapping infosets to lists of node IDs
        node_mapping: Dictionary mapping node IDs to PyGambit node objects
        player_colors: Dictionary mapping player labels to RGBA color tuples
    """
    from matplotlib.patches import Ellipse
    
    for _, node_ids in infoset_mapping.items():
        if len(node_ids) > 1:  # Only draw ovals for multi-node information sets
            # Get positions of all nodes in this information set
            node_positions = [pos[node_id] for node_id in node_ids]
            
            if not node_positions:
                continue
                
            # Calculate bounding box
            xs = [p[0] for p in node_positions]
            ys = [p[1] for p in node_positions]
            
            min_x, max_x = min(xs), max(xs)
            min_y, max_y = min(ys), max(ys)
            
            # Add adaptive padding based on number of nodes and their spread
            base_padding = 40
            spread_x = max_x - min_x
            spread_y = max_y - min_y
            padding_x = max(base_padding, spread_x * 0.3)
            padding_y = max(base_padding, spread_y * 0.3)
            
            center_x = (min_x + max_x) / 2
            center_y = (min_y + max_y) / 2
            width = spread_x + padding_x
            height = max(spread_y + padding_y, 60)  # Minimum height for visibility
            
            # Get player color for the oval
            sample_node = node_mapping[node_ids[0]]
            if sample_node.player:
                if hasattr(sample_node.player, "is_chance") and sample_node.player.is_chance:
                    color = player_colors.get("chance", (1.0, 0.0, 0.0, 1.0))
                else:
                    color = player_colors.get(sample_node.player.label, (0.7, 0.7, 0.7, 1.0))
            else:
                color = (0.7, 0.7, 0.7, 1.0)
            
            # Create ellipse with low alpha for transparency
            ellipse = Ellipse((center_x, center_y), width, height, 
                            facecolor=color[:3], alpha=0.15, 
                            edgecolor=color[:3], linewidth=1.5)
            ax.add_patch(ellipse)


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
            # Decision node - show information set
            if node.player.is_chance:
                node_labels[node_id] = f"Chance: IS {node.infoset.number}"
            else:
                node_labels[node_id] = f"{node.player.label}: IS {node.infoset.number}"
        else:
            # Root or other node
            node_labels[node_id] = "Root"
    return node_labels


def create_legend_elements(
    players: list[str], 
    player_colors: dict[str, tuple[float, float, float, float]]
) -> list[Line2D]:
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
    G, node_colors, node_mapping, infoset_mapping = build_game_graph(game, player_colors)
    
    # If graph is empty (no nodes), nothing to plot
    if len(G.nodes) == 0:
        print("Warning: No nodes to plot in game tree")
        return
    
    # Compute layout and adjust for information sets
    pos = compute_graph_layout(G)
    pos = adjust_layout_for_infosets(pos, infoset_mapping, node_mapping)

    # Create the plot
    fig, ax = plt.subplots(figsize=figsize)
    
    # Draw information set ovals first (so they appear behind nodes)
    draw_infoset_ovals(ax, pos, infoset_mapping, node_mapping, player_colors)
    
    nx.draw(
        G,
        pos,
        with_labels=False,
        arrows=False,
        node_size=node_size,
        edge_color="black",
        node_color=node_colors,
        arrowsize=20,
        width=2,
        ax=ax,
    )

    # Draw edge labels (action labels) only if there are edges and labels are requested
    if show_edge_labels and len(G.edges) > 0:
        try:
            edge_labels = nx.get_edge_attributes(G, "label")
            # Only try to draw labels if they exist
            if edge_labels:
                nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, 
                                            font_size=edge_font_size, font_color="black", ax=ax)
        except Exception as e:
            print(f"Warning: Could not draw edge labels: {e}")

    # Draw node labels
    node_labels = create_node_labels(G, node_mapping, game)
    nx.draw_networkx_labels(G, pos, labels=node_labels, font_size=font_size, ax=ax)

    # Create and display legend
    legend_elements = create_legend_elements(players, player_colors)
    ax.legend(handles=legend_elements, loc="upper right")
    
    ax.set_title(f"Game Tree: {game.title}")
    ax.axis("off")
    fig.tight_layout()
    plt.show()