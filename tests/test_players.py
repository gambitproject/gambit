import pytest

import pygambit as gbt

from . import games


def _generate_strategic_game() -> gbt.Game:
    game = gbt.Game.new_table([2, 2])
    game.players[0].label = "Alphonse"
    game.players[1].label = "Gaston"
    return game


def _generate_extensive_game() -> gbt.Game:
    return gbt.Game.new_tree()


def test_player_count():
    game = gbt.Game.new_table([2, 2])
    assert len(game.players) == 2


def test_player_label():
    game = gbt.Game.new_table([2, 2])
    game.players[0].label = "Alphonse"
    game.players[1].label = "Gaston"
    assert game.players[0].label == "Alphonse"
    assert game.players[1].label == "Gaston"


def test_player_index_by_string():
    game = gbt.Game.new_table([2, 2])
    game.players[0].label = "Alphonse"
    game.players[1].label = "Gaston"
    assert game.players["Alphonse"].label == "Alphonse"
    assert game.players["Gaston"].label == "Gaston"


def test_player_index_out_of_range():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(IndexError):
        _ = game.players[3]


def test_player_index_invalid():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(TypeError):
        _ = game.players[1.3]


def test_player_label_invalid():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(KeyError):
        _ = game.players["Not a player"]


def test_strategic_game_add_player():
    game = gbt.Game.new_table([2, 2])
    game.add_player()
    assert len(game.players) == 3
    assert len(game.players[2].strategies) == 1


def test_extensive_game_add_player():
    game = gbt.Game.new_tree()
    game.add_player()
    assert len(game.players) == 1
    assert len(game.players[0].infosets) == 0
    assert len(game.players[0].strategies) == 1


def test_strategic_game_add_strategy():
    game = gbt.Game.new_table([2, 2])
    game.add_strategy(game.players[0], "new strategy")
    assert len(game.players[0].strategies) == 3


def test_extensive_game_add_strategy():
    game = gbt.Game.new_tree(["Alice"])
    assert len(game.players["Alice"].strategies) == 1
    with pytest.raises(gbt.UndefinedOperationError):
        game.add_strategy(game.players["Alice"], "new strategy")


def test_strategic_game_delete_strategy():
    game = gbt.Game.new_table([2, 2])
    game.delete_strategy(game.players[0].strategies[0])
    assert len(game.players[0].strategies) == 1


def test_strategic_game_delete_last_strategy():
    game = gbt.Game.new_table([1, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        game.delete_strategy(game.players[0].strategies[0])


def test_extensive_game_delete_strategy():
    game = gbt.Game.new_tree(["Alice"])
    with pytest.raises(gbt.UndefinedOperationError):
        game.delete_strategy(game.players["Alice"].strategies[0])


def test_player_strategy_by_label():
    game = gbt.Game.new_table([2, 2])
    game.players[0].strategies[0].label = "Cooperate"
    assert game.players[0].strategies["Cooperate"].label == "Cooperate"


def test_player_strategy_bad_index():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(IndexError):
        _ = game.players[0].strategies[42]


def test_player_strategy_bad_label():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(KeyError):
        _ = game.players[0].strategies["Cooperate"]


def test_player_strategy_bad_type():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(TypeError):
        _ = game.players[0].strategies[1.3]


def test_player_get_min_payoff():
    game = games.read_from_file("payoff_game.nfg")
    assert game.players[0].min_payoff == 4
    assert game.players["Player 1"].min_payoff == 4
    assert game.players[1].min_payoff == 1
    assert game.players["Player 2"].min_payoff == 1


def test_player_get_max_payoff():
    game = games.read_from_file("payoff_game.nfg")
    assert game.players[0].max_payoff == 10
    assert game.players["Player 1"].max_payoff == 10
    assert game.players[1].max_payoff == 8
    assert game.players["Player 2"].max_payoff == 8
