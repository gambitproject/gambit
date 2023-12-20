"""A utility module to create/load games for the test suite."""
import pathlib

import pygambit as gbt


def read_from_file(fn: str) -> gbt.Game:
    return gbt.Game.read_game(pathlib.Path("tests/test_games")/fn)


def create_mixed_behav_game() -> gbt.Game:
    """
    Returns
    -------
    Game
        Three-player extensive form game: binary tree with 3 infomation sets, one per player,
        with 1, 2, and 4 nodes respectively
    """
    return read_from_file("mixed_behavior_game.efg")


def create_myerson_2_card_poker() -> gbt.Game:
    """
    Returns
    -------
    Game
        Myerson 2-card poker: Two-player extensive poker game with a chance move with two moves,
        then player 1 can raise or fold; after raising player 2 is in an infoset with two nodes
        and can choose to meet or pass
    """
    return read_from_file("myerson_2_card_poker.efg")


def create_strategic_game() -> gbt.Game:
    game = gbt.Game.new_table([2, 2])
    game.players[0].label = "Joe"
    game.players["Joe"].strategies[0].label = "cooperate"
    game.players[1].label = "Dan"
    game.players["Dan"].strategies[1].label = "defect"
    return game


def create_coord_4x4_nfg() -> gbt.Game:
    return read_from_file("coordination_4x4.nfg")


def create_zero_matrix_nfg() -> gbt.Game:
    """4-players with three strategies each"""
    return read_from_file("zero.nfg")


def create_centipede_game_with_chance() -> gbt.Game:
    return read_from_file("cent3.efg")


def create_el_farol_bar_game() -> gbt.Game:
    return read_from_file("el_farol_bar.efg")


def create_selten_horse_game() -> gbt.Game:
    return read_from_file("e01.efg")
