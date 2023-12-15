import pytest
import numpy as np

import pygambit as gbt


def test_from_arrays():
    m = np.array([[8, 2], [10, 5]])
    game = gbt.Game.from_arrays(m, m.transpose())
    assert len(game.players) == 2
    assert len(game.players[0].strategies) == 2
    assert len(game.players[1].strategies) == 2


def test_game_get_outcome_by_index():
    game = gbt.Game.new_table([2, 2])
    assert game[0, 0] == game.outcomes[0]


def test_game_get_outcome_by_label():
    game = gbt.Game.new_table([2, 2])
    game.players[0].strategies[0].label = "defect"
    game.players[1].strategies[0].label = "cooperate"
    assert game["defect", "cooperate"] == game.outcomes[0]


def test_game_get_outcome_invalid_tuple_size():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(KeyError):
        _ = game[0, 0, 0]


def test_game_outcomes_non_tuple():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(TypeError):
        _ = game[42]


def test_game_outcomes_type_exception():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(TypeError):
        _ = game[1.23, 1]


def test_game_get_outcome_index_out_of_range():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(IndexError):
        _ = game[0, 3]


def test_game_get_outcome_unmatched_label():
    game = gbt.Game.new_table([2, 2])
    game.players[0].strategies[0].label = "defect"
    game.players[1].strategies[0].label = "cooperate"
    with pytest.raises(IndexError):
        _ = game["defect", "defect"]


def test_game_get_outcome_with_strategies():
    game = gbt.Game.new_table([2, 2])
    assert (
        game[game.players[0].strategies[0], game.players[1].strategies[0]] ==
        game.outcomes[0]
    )


def test_game_get_outcome_with_bad_strategies():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(IndexError):
        _ = game[game.players[0].strategies[0], game.players[0].strategies[0]]


def test_game_dereference_invalid():
    game = gbt.Game.new_tree()
    game.add_player("One")
    strategy = game.players[0].strategies[0]
    game.append_move([game.root], game.players[0], ["a", "b"])
    with pytest.raises(RuntimeError):
        _ = strategy.label
