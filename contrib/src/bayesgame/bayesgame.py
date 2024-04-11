"""Experimental script to build a two-player Bayesian game in an
explicit extensive representation.
"""
import typing

import pygambit as gbt


def terminal_nodes(game: gbt.Game) -> list:
    return [n for n in game.nodes() if n.is_terminal]


def history(node: gbt.Node) -> list:
    hist = []
    while node.parent:
        hist = [node.prior_action.label] + hist
        node = node.parent
    return hist


def bayesian_game(types: list, actions: list,
                  payoffs: typing.Callable) -> gbt.Game:
    g = gbt.Game.new_tree(players=["Player 1", "Player 2"],
                          title="Untitled Bayesian game")

    g.append_move(g.root, g.players.chance, types)
    g.append_move(g.root.children, g.players.chance, types)
    g.append_move(terminal_nodes(g), g.players["Player 1"], actions)
    g.reveal(g.root.infoset, g.players["Player 1"])
    g.append_move(terminal_nodes(g), g.players["Player 2"], actions)
    g.reveal(g.root.children[0].infoset, g.players["Player 2"])
    for node in terminal_nodes(g):
        g.set_outcome(node, g.add_outcome())
        node.outcome["Player 1"], node.outcome["Player 2"] = payoffs(history(node))
    return g


def bach_stravinsky(hist: list):
    """Bayesian game version of 'Bach or Stravinsky', from
    Osborne and Rubenstein (1994), A course in game theory, exercise 27.1.
    """
    t1, t2, a1, a2 = hist
    if a1 != a2:
        return 0, 0
    return (2 if a1 == t1.lower() else 1,
            2 if a2 == t2.lower() else 1)


def fp_pv_auction(hist: list) -> tuple:
    """First-price private-values auction.
    """
    t1, t2, a1, a2 = [gbt.Decimal(v) for v in hist]
    if a1 > a2:
        return t1 - a1, 0
    if a2 > a1:
        return 0, t2 - a2
    return (t1 - a1) / 2, (t2 - a2) / 2


if __name__ == "__main__":
    low_v = bayesian_game(["0", "2", "4", "6", "8", "11"],
                          ["0", "1", "2", "3", "4", "5", "6"],
                          fp_pv_auction)
    high_v = bayesian_game(["0", "3", "5", "7", "9", "12"],
                           ["0", "1", "2", "3", "4", "5", "6"],
                           fp_pv_auction)
    with open("low_v.efg", "w") as f:
        f.write(low_v.write())
    with open("high_v.efg", "w") as f:
        f.write(high_v.write())
