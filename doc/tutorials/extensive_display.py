# TODO: move these functions into the Game class
import matplotlib.pyplot as plt
import networkx as nx


def plot_gambit_tree(game):
    # Assign a unique color to each player
    players = [p.label for p in game.players]
    color_map = plt.cm.get_cmap("tab10")
    player_colors = {p: color_map(i) for i, p in enumerate(players)}
    player_colors["chance"] = (0.5, 0.5, 0.5, 1.0)  # gray for chance

    G = nx.DiGraph()
    edge_colors = []
    node_mapping = {}  # Map node_id to actual node object

    def add_edges(node, parent=None, action_label=None):
        node_id = id(node)
        node_mapping[node_id] = node

        if parent is not None:
            parent_id = id(parent)
            # Color edge based on the player who made the move leading to this node
            player_label = parent.player.label if parent.player else "chance"
            G.add_edge(parent_id, node_id, label=action_label, player=player_label)
            edge_colors.append(player_colors.get(player_label, (0, 0, 0, 1)))

        for child in node.children:
            add_edges(child, node, child.label)

    add_edges(game.root)

    # Use spring layout as fallback if graphviz layout fails
    try:
        pos = nx.nx_agraph.graphviz_layout(G, prog="dot")
        print("Graphviz layout succeeded")
    except:
        pos = nx.spring_layout(G, k=2, iterations=50)
        print("Graphviz layout failed, using spring layout instead")

    plt.figure(figsize=(12, 8))
    nx.draw(
        G,
        pos,
        with_labels=False,
        arrows=True,
        node_size=500,
        edge_color=edge_colors,
        node_color="lightblue",
        arrowsize=20,
        width=2,
    )

    # Draw edge labels (action labels)
    edge_labels = nx.get_edge_attributes(G, "label")
    nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, font_size=10, font_color="black")

    # Draw node labels (player names and outcomes)
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

    nx.draw_networkx_labels(G, pos, labels=node_labels, font_size=8)

    # Create legend for player colors
    legend_elements = []
    for player in players:
        legend_elements.append(
            plt.Line2D([0], [0], color=player_colors[player], lw=3, label=player)
        )
    if "chance" in player_colors:
        legend_elements.append(
            plt.Line2D([0], [0], color=player_colors["chance"], lw=3, label="Chance")
        )

    plt.legend(handles=legend_elements, loc="upper right")
    plt.title(f"Game Tree: {game.title}")
    plt.axis("off")
    plt.tight_layout()
    plt.show()