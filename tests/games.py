"""A utility module to create/load games for the test suite."""
import pathlib

import pygambit as gbt


def read_from_file(fn: str) -> gbt.Game:
    if fn.endswith(".efg"):
        return gbt.read_efg(pathlib.Path("tests/test_games")/fn)
    elif fn.endswith(".nfg"):
        return gbt.read_nfg(pathlib.Path("tests/test_games")/fn)
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
        Myerson 2-card poker: Two-player extensive poker game with a chance move with two moves,
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
