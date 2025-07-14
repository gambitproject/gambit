"""A utility module to create/load games for the test suite."""

import pathlib
from itertools import product

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


class Centipede:
    """
    Helper class for creating EFG and corresponding reduced strategy sets and payoff arrays
    for the two-player centipede game (with the number of rounds as a parameter, and two payoff
    parameters, one for each player)
    """

    def create_game(N, m0, m1):
        # Create two-player centipede game with N rounds and payoff params m0 and m1
        g = gbt.Game.new_tree(
            players=["1", "2"], title=f"Centipede Game with {N} rounds"
        )
        current_node = g.root
        current_player = "1"
        for t in range(N):
            g.append_move(current_node, current_player, ["Take", "Push"])
            payoffs = [2**t * m0, 2**t * m1]  # take payoffs
            if current_player == "2":
                payoffs.reverse()
            g.set_outcome(current_node.children[0], g.add_outcome(payoffs))
            if t == N - 1:  # for last round, push payoffs
                payoffs = [2 ** (t + 1) * m1, 2 ** (t + 1) * m0]
                if current_player == "2":
                    payoffs.reverse()
                g.set_outcome(current_node.children[1], g.add_outcome(payoffs))
            current_node = current_node.children[1]
            current_player = "2" if current_player == "1" else "1"
        return g

    def redu_strats(N):

        if N % 2 == 0:
            p1_n_moves = p2_n_moves = int(N / 2)
        else:
            p1_n_moves = int((N + 1) / 2)
            p2_n_moves = int((N - 1) / 2)

        def get_rss(n):
            # Given n (which should be I and J for players 1 and 2 respectively)
            # creates the set of strategies that:
            # - have n + 1 positions
            # - have all *s after any 1
            # - have prefixes 1, 21, 221, 2221, etc. ending with all 2s
            ret = ["2" * (i) + "1" * 1 + "*" * (n - i - 1) for i in range(n)]
            ret.append("2" * n)
            return ret

        return [get_rss(n) for n in [p1_n_moves, p2_n_moves]]

    def get_size_of_RSF(N):
        return [len(x) for x in Centipede.redu_strats(N)]

    def create_rsf(N, m0, m1):
        m, n = Centipede.get_size_of_RSF(N)
        p1_payoffs = np.zeros((m, n), dtype=int)
        p2_payoffs = np.zeros((m, n), dtype=int)
        row1_1 = [m0] * n
        row1_2 = [m1] * n
        p1_payoffs[0, :] = row1_1
        p2_payoffs[0, :] = row1_2

        for j in range(n - 1 if N % 2 == 0 else n):
            max_in_col_p1 = 2 ** (2 * j + 1) * m1
            max_in_col_p2 = 2 ** (2 * j + 1) * m0
            base1 = [max_in_col_p1] * (m - 1)
            base2 = [max_in_col_p2] * (m - 1)
            for i in range(1, (j + 1)):
                base1[i - 1] = 2 ** (2 * i) * m0
                base2[i - 1] = 2 ** (2 * i) * m1
            p1_payoffs[1:, j] = base1
            p2_payoffs[1:, j] = base2
        if N % 2 == 0:
            # final col
            p1_payoffs[:, n - 1] = p1_payoffs[:, n - 2]
            p2_payoffs[:, n - 1] = p2_payoffs[:, n - 2]
            p1_extra_pay = 2 ** (2 * (n - 1)) * m0
            p2_extra_pay = 2 ** (2 * (n - 1)) * m1
        else:
            # final row
            p1_payoffs[m - 1, :] = p1_payoffs[m - 2, :]
            p2_payoffs[m - 1, :] = p2_payoffs[m - 2, :]
            p1_extra_pay = 2 ** (2 * (n) - 1) * m1
            p2_extra_pay = 2 ** (2 * (n) - 1) * m0
        p1_payoffs[m - 1, n - 1] = p1_extra_pay
        p2_payoffs[m - 1, n - 1] = p2_extra_pay
        return p1_payoffs, p2_payoffs

    def test_parametrization(N, m0, m1):
        return (
            Centipede.create_game(N, m0, m1),
            Centipede.redu_strats(N),
            Centipede.create_rsf(N, m0, m1),
        )


class BinEFGExpRSF:
    """
    Helper class for creating EFG and corresponding reduced strategy sets and payoff arrays
    for two-player games on a binary tree with exponentially-many (~ 2^root(level))
    reduced strategies

    Games taken from:

    B. von Stengel, A. van den Elzen, and A. J. J. Talman (2002)
    Computing normal form perfect equilibria for extensive two-person games
    Econometrica 70(2), 693-715

    The games are parametrized by a single positive integer, namely the number of "level"s
    """

    def get_n_infosets(level):
        if level % 2 == 0:
            p1_n_moves = p2_n_moves = int(level / 2)
        else:
            p1_n_moves = int((level + 1) / 2)
            p2_n_moves = int((level - 1) / 2)
        p1_n_isets = (4**p1_n_moves + 2) / 6
        p2_n_isets = (4**p2_n_moves - 1) / 3
        return int(p1_n_isets), int(p2_n_isets)

    def redu_strats(player, level):
        assert player in [1, 2] and level >= 2
        if level == 2:
            return ["1", "2"]
        elif (level % 2 == 0 and player == 1) or (level % 2 != 0 and player == 2):
            return BinEFGExpRSF.redu_strats(player, level - 1)
        elif player == 2:
            tmp = BinEFGExpRSF.redu_strats(player=1, level=level - 1)
            tmp = [
                t[1:] for t in tmp
            ]  # remove first action (1 from 1st half; 2 from 2nd half)
            # split into two halves
            n_half = int(len(tmp) / 2)
            first_half = tmp[:n_half]
            second_half = tmp[n_half:]
            # create first half suffix
            first_half = product(first_half, first_half)
            first_half = ["".join(t) for t in first_half]
            first_half = ["1" + t for t in first_half]  # add 1 to front
            # create second half suffix
            second_half = product(second_half, second_half)
            second_half = ["".join(t) for t in second_half]
            second_half = ["2" + t for t in second_half]  # add 2 to front
            return first_half + second_half  # glue halves together
        else:
            p1_n_isets, p2_n_isets = BinEFGExpRSF.get_n_infosets(level)
            p1_n_isets_level_minus1, p2_n_isets_level_minus1 = (
                BinEFGExpRSF.get_n_infosets(level - 1)
            )
            stars = "*" * (p1_n_isets - p2_n_isets_level_minus1 - 1)
            return [
                "1" + t + stars
                for t in BinEFGExpRSF.redu_strats(player=2, level=level - 1)
            ] + [
                "2" + stars + t
                for t in BinEFGExpRSF.redu_strats(player=2, level=level - 1)
            ]

    def create_binary_tree(g, node, player1_turn, depth, max_depth):
        if depth == max_depth:
            payoff1 = payoff2 = 0
            g.set_outcome(node, g.add_outcome([payoff1, payoff2]))
        else:
            current_player = "1" if player1_turn else "2"
            g.append_move(node, current_player, ["L", "R"])
            for child in node.children:
                BinEFGExpRSF.create_binary_tree(
                    g, child, not player1_turn, depth + 1, max_depth
                )

    def create_game(L):
        g = gbt.Game.new_tree(players=["1", "2"], title=f"Binary Tree Game (L={L})")
        BinEFGExpRSF.create_binary_tree(g, g.root, True, 0, L)
        for n in g.nodes:
            if not n.is_terminal and not n.children[0].is_terminal:
                g.set_infoset(n.children[1], n.children[0].infoset)
        return g

    def create_rsf(reduced_strategies):
        m = len(reduced_strategies[0])
        n = len(reduced_strategies[1])
        zeros = np.zeros((m, n), dtype=int)
        return [zeros, zeros]

    def test_parametrization(L):
        redu_strats = [BinEFGExpRSF.redu_strats(1, L), BinEFGExpRSF.redu_strats(2, L)]

        return (
            BinEFGExpRSF.create_game(L),
            redu_strats,
            BinEFGExpRSF.create_rsf(redu_strats),
        )


def make_rational(input: str):
    return gbt.Rational(input)


vectorized_make_rational = np.vectorize(make_rational)
