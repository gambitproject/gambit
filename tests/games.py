"""A utility module to create/load games for the test suite."""

import pathlib

import numpy as np

import pygambit as gbt


def read_from_file(fn: str) -> gbt.Game:
    if fn.endswith(".efg"):
        return gbt.read_efg(pathlib.Path("tests/test_games") / fn)
    elif fn.endswith(".nfg"):
        return gbt.read_nfg(pathlib.Path("tests/test_games") / fn)
    else:
        raise ValueError(f"Unknown file extension in {fn}")


################################################################################################
# Normal-form (aka strategic-form) games (nfg)


def create_2x2_zero_nfg() -> gbt.Game:
    """
    Returns
    -------
    Game
        2x2 all-zero-payoffs bimatrix, with player names and a duplicate label set intentionally
        for testing purposes
    """
    game = gbt.Game.new_table([2, 2])

    game.players[0].label = "Joe"
    game.players["Joe"].strategies[0].label = "cooperate"
    game.players["Joe"].strategies[1].label = "defect"

    game.players[1].label = "Dan"
    game.players["Dan"].strategies[0].label = "defect"
    # intentional duplicate label for player (generates warning):
    game.players["Dan"].strategies[1].label = "defect"

    return game


def create_2x2x2_nfg() -> gbt.Game:
    """
    - This comes from a local max cut instance:
      players {1,2,3} are nodes; edge weight{1,2} = 2; weight{1,3} = -1; weight{2,3} = 2
    - Pure strategies {a,b} encode if respective player is on left or right of the cut
    - The payoff to a player is the sum of their incident edges across the implied cut
    - Pure equilibrium iff local max cuts; in addition, uniform mixture is an equilibrium
    - Equilibrium analysis for pure profiles:
        a a a:  0 0  0 -- Not Nash (2 can deviate and get 4)
        b a a:  1 2 -1 -- Not Nash (3 can deviate and get 2)
        a b a:  2 4  2 -- Nash (global max cut)
        b b a: -1 2  1 -- Not Nash (1 can deviate and get 2)
        a a b: -1 2  1 -- Not Nash (1 can deviate and get 2)
        b a b:  2 4  2 -- Nash (global max cut)
        a b b:  1 2 -1 -- Not Nash (3 can deviate and get 2)
        b b b:  0 0  0 -- Not Nash (2 can deviate and get 4)
    """
    return read_from_file("2x2x2_nfg_with_two_pure_one_mixed_eq.nfg")


def create_coord_4x4_nfg(outcome_version: bool = False) -> gbt.Game:
    """
    Returns
    -------
    Game
        4x4 coordination game, either via reading in a payoff version nfg, or an
        outcome version nfg, which has strategy labels useful for testing
    """
    version = "outcome" if outcome_version else "payoff"
    return read_from_file(f"coordination_4x4_{version}.nfg")


################################################################################################
# Extensive-form games (efg)


def create_mixed_behav_game_efg() -> gbt.Game:
    """
    Returns
    -------
    Game
        Three-player extensive form game: binary tree with 3 infomation sets, one per player,
        with 1, 2, and 4 nodes respectively
    """
    return read_from_file("mixed_behavior_game.efg")


def create_myerson_2_card_poker_efg() -> gbt.Game:
    """
    Returns
    -------
    Game
        Simplied "stripped down" version of Myerson 2-card poker:
        Two-player extensive poker game with a chance move with two moves,
        then player 1 can raise or fold; after raising player 2 is in an infoset with two nodes
        and can choose to meet or pass
    """
    return read_from_file("myerson_2_card_poker.efg")


def create_centipede_game_with_chance_efg() -> gbt.Game:
    """
    Returns
    -------
    Game
        2-player Centipede Game with 3 innings and a probability of altruism
    """
    return read_from_file("cent3.efg")


def create_el_farol_bar_game_efg() -> gbt.Game:
    """
    Returns
    -------
    Game
        5-player El Farol Bar Game
    """
    return read_from_file("el_farol_bar.efg")


def create_selten_horse_game_efg() -> gbt.Game:
    """
    Returns
    -------
    Game
        5-player Selten's Horse Game
    """
    return read_from_file("e01.efg")


def create_reduction_generic_payoffs_efg() -> gbt.Game:
    # tree with only root
    g = gbt.Game.new_tree(
        players=["1", "2"], title="2 player reduction generic payoffs"
    )

    # add four children
    g.append_move(g.root, "2", ["a", "b", "c", "d"])

    # add L and R after a
    g.append_move(g.root.children[0], "1", ["L", "R"])

    # add C and D to single infoset after b and c
    nodes = [g.root.children[1], g.root.children[2]]
    g.append_move(nodes, "1", ["C", "D"])

    # add s and t from single infoset after rightmost C and D
    g.append_move(g.root.children[2].children, "2", ["s", "t"])

    # add p and q
    g.append_move(g.root.children[0].children[1], "2", ["p", "q"])

    # add U and V in a single infoset after p and q
    g.append_move(g.root.children[0].children[1].children, "1", ["U", "V"])

    # Set outcomes

    g.set_outcome(g.root.children[0].children[0], g.add_outcome([1, -1], label="aL"))
    g.set_outcome(
        g.root.children[0].children[1].children[0].children[0],
        g.add_outcome([2, -2], label="aRpU"),
    )
    g.set_outcome(
        g.root.children[0].children[1].children[0].children[1],
        g.add_outcome([3, -3], label="aRpV"),
    )
    g.set_outcome(
        g.root.children[0].children[1].children[1].children[0],
        g.add_outcome([4, -4], label="aRqU"),
    )
    g.set_outcome(
        g.root.children[0].children[1].children[1].children[1],
        g.add_outcome([5, -5], label="aRqV"),
    )

    g.set_outcome(g.root.children[1].children[0], g.add_outcome([6, -6], label="bC"))
    g.set_outcome(g.root.children[1].children[1], g.add_outcome([7, -7], label="bD"))

    g.set_outcome(
        g.root.children[2].children[0].children[0], g.add_outcome([8, -8], label="cCs")
    )
    g.set_outcome(
        g.root.children[2].children[0].children[1], g.add_outcome([9, -9], label="cCt")
    )
    g.set_outcome(
        g.root.children[2].children[1].children[0],
        g.add_outcome([10, -10], label="cDs"),
    )
    g.set_outcome(
        g.root.children[2].children[1].children[1],
        g.add_outcome([11, -11], label="cDt"),
    )

    g.set_outcome(g.root.children[3], g.add_outcome([12, -12], label="d"))

    return g


def create_reduction_one_player_generic_payoffs_efg() -> gbt.Game:
    g = gbt.Game.new_tree(players=["1"], title="One player reduction generic payoffs")
    g.append_move(g.root, "1", ["a", "b", "c", "d"])
    g.append_move(g.root.children[0], "1", ["e", "f"])
    g.set_outcome(g.root.children[0].children[0], g.add_outcome([1]))
    g.set_outcome(g.root.children[0].children[1], g.add_outcome([2]))
    g.set_outcome(g.root.children[1], g.add_outcome([3]))
    g.set_outcome(g.root.children[2], g.add_outcome([4]))
    g.set_outcome(g.root.children[3], g.add_outcome([5]))
    return g


def create_reduction_both_players_payoff_ties_efg() -> gbt.Game:
    g = gbt.Game.new_tree(players=["1", "2"], title="From GTE survey")
    g.append_move(g.root, "1", ["A", "B", "C", "D"])
    g.append_move(g.root.children[0], "2", ["a", "b"])
    g.append_move(g.root.children[1], "2", ["c", "d"])
    g.append_move(g.root.children[2], "2", ["e", "f"])
    g.append_move(g.root.children[0].children[1], "2", ["g", "h"])
    g.append_move(g.root.children[2].children, "1", ["E", "F"])

    g.set_outcome(g.root.children[0].children[0], g.add_outcome([2, 8]))
    g.set_outcome(g.root.children[0].children[1].children[0], g.add_outcome([0, 1]))
    g.set_outcome(g.root.children[0].children[1].children[1], g.add_outcome([5, 2]))
    g.set_outcome(g.root.children[1].children[0], g.add_outcome([7, 6]))
    g.set_outcome(g.root.children[1].children[1], g.add_outcome([4, 2]))
    g.set_outcome(g.root.children[2].children[0].children[0], g.add_outcome([3, 7]))
    g.set_outcome(g.root.children[2].children[0].children[1], g.add_outcome([8, 3]))
    g.set_outcome(g.root.children[2].children[1].children[0], g.add_outcome([7, 8]))
    g.set_outcome(g.root.children[2].children[1].children[1], g.add_outcome([2, 2]))
    g.set_outcome(g.root.children[3], g.add_outcome([6, 4]))
    return g


def make_rational(input: str):
    return gbt.Rational(input)


vectorized_make_rational = np.vectorize(make_rational)
