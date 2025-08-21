"""A utility module to create/load games for the test suite."""

import pathlib
from abc import ABC, abstractmethod
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


class EfgFamilyForReducedStrategicFormTests(ABC):
    """ """

    @abstractmethod
    def __init__(params):
        pass

    @abstractmethod
    def gbt_game(self):
        pass

    @abstractmethod
    def reduced_strategies(self):
        pass

    @abstractmethod
    def reduced_strategic_form(self):
        pass

    def set_size_of_rsf(self, reduced_strategies):
        self.size_of_rsf = [len(r) for r in reduced_strategies]

    @classmethod
    def get_test_data(cls, **params):
        """
        given the provided parameters, return a tuple with:
            - the game as a gbt.Game object
            - the expected list of players reduced strategies in this game
            - the expected reduced strategic form (i.e. payoff tensors) for this game
        the tuple is used directly in test_reduced_strategic_form in test_extensive.py
        """

        game = cls(params)

        return (
            game.gbt_game(),
            game.reduced_strategies(),
            game.reduced_strategic_form(),
        )


class Centipede(EfgFamilyForReducedStrategicFormTests):
    """
    Two-player Centipede game

    Params: number of rounds (N); two payoff parameters (m0, m1)
    """

    def __init__(self, params):
        self.N = params["N"]
        self.m0 = params["m0"]
        self.m1 = params["m1"]

    def gbt_game(self):
        g = gbt.Game.new_tree(
            players=["1", "2"], title=f"Centipede Game with {self.N} rounds"
        )
        current_node = g.root
        current_player = "1"
        for t in range(self.N):
            g.append_move(current_node, current_player, ["Take", "Push"])
            payoffs = [2**t * self.m0, 2**t * self.m1]  # take payoffs
            if current_player == "2":
                payoffs.reverse()
            g.set_outcome(current_node.children[0], g.add_outcome(payoffs))
            if t == self.N - 1:  # for last round, push payoffs
                payoffs = [2 ** (t + 1) * self.m1, 2 ** (t + 1) * self.m0]
                if current_player == "2":
                    payoffs.reverse()
                g.set_outcome(current_node.children[1], g.add_outcome(payoffs))
            current_node = current_node.children[1]
            current_player = "2" if current_player == "1" else "1"
        return g

    def reduced_strategies(self):

        if self.N % 2 == 0:
            n_moves = [int(self.N / 2)] * 2
        else:
            n_moves = [int((self.N + 1) / 2), int((self.N - 1) / 2)]

        def get_rss(n):
            # Given n number of times a player moves, their reduced strategies are such that
            # they Have n positions; have all *s after any 1; have prefixes 1, 21, 221, 2221, etc.
            # and finally the last strategyt is all 2s
            ret = ["2" * (i) + "1" * 1 + "*" * (n - i - 1) for i in range(n)]
            ret.append("2" * n)
            return ret

        rs = [get_rss(n) for n in n_moves]
        self.set_size_of_rsf(rs)
        return rs

    def reduced_strategic_form(self):
        m, n = self.size_of_rsf
        p1_payoffs = np.zeros((m, n), dtype=int)
        p2_payoffs = np.zeros((m, n), dtype=int)
        row1_1 = [self.m0] * n
        row1_2 = [self.m1] * n
        p1_payoffs[0, :] = row1_1
        p2_payoffs[0, :] = row1_2

        for j in range(n - 1 if self.N % 2 == 0 else n):
            max_in_col_p1 = 2 ** (2 * j + 1) * self.m1
            max_in_col_p2 = 2 ** (2 * j + 1) * self.m0
            base1 = [max_in_col_p1] * (m - 1)
            base2 = [max_in_col_p2] * (m - 1)
            for i in range(1, (j + 1)):
                base1[i - 1] = 2 ** (2 * i) * self.m0
                base2[i - 1] = 2 ** (2 * i) * self.m1
            p1_payoffs[1:, j] = base1
            p2_payoffs[1:, j] = base2
        if self.N % 2 == 0:
            # final col
            p1_payoffs[:, n - 1] = p1_payoffs[:, n - 2]
            p2_payoffs[:, n - 1] = p2_payoffs[:, n - 2]
            p1_extra_pay = 2 ** (2 * (n - 1)) * self.m0
            p2_extra_pay = 2 ** (2 * (n - 1)) * self.m1
        else:
            # final row
            p1_payoffs[m - 1, :] = p1_payoffs[m - 2, :]
            p2_payoffs[m - 1, :] = p2_payoffs[m - 2, :]
            p1_extra_pay = 2 ** (2 * (n) - 1) * self.m1
            p2_extra_pay = 2 ** (2 * (n) - 1) * self.m0
        p1_payoffs[m - 1, n - 1] = p1_extra_pay
        p2_payoffs[m - 1, n - 1] = p2_extra_pay
        return p1_payoffs, p2_payoffs


class BinaryTreeGames(EfgFamilyForReducedStrategicFormTests):
    """
    Params:
        - single positive integer, namely the number of "level"s
        - number of players (currently the 1, 2, and 3-player versions are used in tests)

    These games:
        - are all binary trees with imperfect information
        - after every L/R choice the subsequent, have the two subsequent nodes (unless terminal)
            together and alone in a single infoset (so all infosets except the root are pairs)
        - the purpose of these games is to test the reduced strategy lists
        - payoff matrices are all zero for simplicity

    These games have rougly 2^root(level) many reduced strategies

    The 2-player versions appear in:

    B. von Stengel, A. van den Elzen, and A. J. J. Talman (2002)
    Computing normal form perfect equilibria for extensive two-person games
    Econometrica 70(2), 693-715

    The 1-player versions have Imperfect Recall
    """

    def __init__(self, n_players, params):
        self.level = params["level"]
        self.players = list(range(1, n_players + 1))
        self.n_players = n_players

    def get_n_infosets(self, level):

        if self.n_players == 1:
            return {1: 2 ** (level - 1)}

        players = list(range(1, self.n_players + 1))
        n_isets = [1] + [0] * (self.n_players - 1)
        whose_turn = 1  # start from player 2 and level 2
        for lev in range(2, level + 1):
            n_isets[whose_turn] += 2 ** (lev - 2)
            whose_turn = (whose_turn + 1) % self.n_players
        return {p: n_isets[p - 1] for p in players}

    def _redu_strategies_level_1(self, player):
        return ["1", "2"] if player == 1 else ["*"]

    def player_with_changes(self, level):
        return ((level - 1) % self.n_players) + 1

    def last_player_with_changes(self, level):
        return ((level - 2) % self.n_players) + 1

    @abstractmethod
    def _redu_strats(self, player, level):
        pass

    def reduced_strategies(self):
        rs = [self._redu_strats(player, self.level) for player in self.players]
        self.set_size_of_rsf(rs)
        return rs

    def create_binary_tree(self, g, node, whose_turn, depth, max_depth):
        # whose_turn cycles through 0,1,n_players-1; current player is str(whose_turn + 1)
        if depth == max_depth:
            g.set_outcome(node, g.add_outcome([0] * self.n_players))
        else:
            current_player = str(whose_turn + 1)
            g.append_move(node, current_player, ["L", "R"])

            whose_turn = (whose_turn + 1) % self.n_players
            for child in node.children:
                self.create_binary_tree(g, child, whose_turn, depth + 1, max_depth)

    def gbt_game(self):
        g = gbt.Game.new_tree(
            players=[str(p) for p in self.players],
            title=f"Binary Tree Game (L={self.level})",
        )
        self.create_binary_tree(g, g.root, 0, 0, self.level)
        for n in g.nodes:
            if not n.is_terminal and not n.children[0].is_terminal:
                g.set_infoset(n.children[1], n.children[0].infoset)
        return g

    def reduced_strategic_form(self):
        return None


class BinEfgOnePlayerIR(BinaryTreeGames):

    def __init__(self, params):
        super().__init__(n_players=1, params=params)

    def _redu_strats(self, player, level):
        if level == 1:
            return self._redu_strategies_level_1(player)
        else:
            tmp = self._redu_strats(1, level - 1)
            tmp = [
                t[1:] for t in tmp
            ]  # remove first action (1 from 1st half; 2 from 2nd half)
            n_half = int(len(tmp) / 2)
            first_half = tmp[:n_half]
            second_half = tmp[n_half:]
            n_stars = (
                self.get_n_infosets(level)[1] - self.get_n_infosets(level - 1)[1] - 1
            )
            stars = "*" * n_stars
            return (
                ["11" + t + stars for t in first_half]
                + ["12" + t + stars for t in second_half]
                + ["21" + stars + t for t in first_half]
                + ["22" + stars + t for t in second_half]
            )


class BinEfgTwoOrThreePlayers(BinaryTreeGames):

    def _redu_strats(self, player, level):
        if level == 1:
            return self._redu_strategies_level_1(player)
        elif player == self.player_with_changes(level):
            if player == 1:
                last_player = self.last_player_with_changes(level)
                tmp1 = self.get_n_infosets(level)
                tmp2 = self.get_n_infosets(level - 1)
                n_stars = tmp1[player] - tmp2[last_player] - 1
                stars = "*" * n_stars
                return [
                    "1" + t + stars
                    for t in self._redu_strats(player=last_player, level=level - 1)
                ] + [
                    "2" + stars + t
                    for t in self._redu_strats(player=last_player, level=level - 1)
                ]
            elif player == 2:
                tmp = self._redu_strats(player=1, level=level - 1)
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
            else:  # player == 3:
                tmp = self._redu_strats(player=2, level=level - 1)
                tmp = product(tmp, tmp)
                tmp = ["".join(t) for t in tmp]
                return tmp
        else:
            return self._redu_strats(player, level - 1)


class BinEfgTwoPlayer(BinEfgTwoOrThreePlayers):
    def __init__(self, params):
        super().__init__(n_players=2, params=params)


class BinEfgThreePlayer(BinEfgTwoOrThreePlayers):
    def __init__(self, params):
        super().__init__(n_players=3, params=params)


def make_rational(input: str):
    return gbt.Rational(input)


vectorized_make_rational = np.vectorize(make_rational)
