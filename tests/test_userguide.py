"""Tests inspired by examples in the user guide."""
import pygambit as gbt


def test_trust_game():
    """Build the one-shot trust game from Kreps (1990)"""
    g = gbt.Game.new_tree(players=["Buyer", "Seller"],
                          title="One-shot trust game, after Kreps (1990)")
    g.append_move(g.root, "Buyer", ["Trust", "Not trust"])
    g.append_move(g.root.children[0], "Seller", ["Honor", "Abuse"])
    g.set_outcome(g.root.children[0].children[0],
                  g.add_outcome([1, 1], label="Trustworthy"))
    g.set_outcome(g.root.children[0].children[1],
                  g.add_outcome([-1, 2], label="Untrustworthy"))
    g.set_outcome(g.root.children[1],
                  g.add_outcome([0, 0], label="Opt-out"))


def test_myerson_poker():
    """Build the one-card poker example adapted from Myerson (1991)"""
    g = gbt.Game.new_tree(players=["Alice", "Bob"],
                          title="One card poker game, after Myerson (1991)")
    g.append_move(g.root, g.players.chance, ["King", "Queen"])
    for node in g.root.children:
        g.append_move(node, "Alice", ["Raise", "Fold"])
    g.append_move(g.root.children[0].children[0], "Bob", ["Meet", "Pass"])
    g.append_infoset(g.root.children[1].children[0],
                     g.root.children[0].children[0].infoset)
    alice_winsbig = g.add_outcome([2, -2], label="Alice wins big")
    alice_wins = g.add_outcome([1, -1], label="Alice wins")
    bob_winsbig = g.add_outcome([-2, 2], label="Bob wins big")
    bob_wins = g.add_outcome([-1, 1], label="Bob wins")
    g.set_outcome(g.root.children[0].children[0].children[0], alice_winsbig)
    g.set_outcome(g.root.children[0].children[0].children[1], alice_wins)
    g.set_outcome(g.root.children[0].children[1], bob_wins)
    g.set_outcome(g.root.children[1].children[0].children[0], bob_winsbig)
    g.set_outcome(g.root.children[1].children[0].children[1], alice_wins)
    g.set_outcome(g.root.children[1].children[1], bob_wins)
